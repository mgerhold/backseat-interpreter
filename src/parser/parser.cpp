#include <format>
#include <parser/parser.hpp>
#include <tl/optional.hpp>
#include <utils/enum_to_string.hpp>


namespace parser {
    class Parser final {
    private:
        std::span<lexer::Token const> m_tokens;
        usize m_index{};

    public:
        [[nodiscard]] Parser(std::span<lexer::Token const> tokens) : m_tokens{ tokens } {
            if (m_tokens.empty() or m_tokens.back().type() != lexer::TokenType::EndOfFile) {
                throw ParserError{ "Token stream does not end with `EndOfFile` token." };
            }
        }

        [[nodiscard]] auto parse() -> std::vector<std::unique_ptr<Statement>> {
            m_index = 0uz;
            auto statements = std::vector<std::unique_ptr<Statement>>{};
            while (not is_at_end()) {
                statements.push_back(statement());
            }
            return statements;
        }

    private:
        [[nodiscard]] auto is_at_end() const -> bool {
            return m_index >= m_tokens.size() or m_tokens.at(m_index).type() == lexer::TokenType::EndOfFile;
        }

        [[nodiscard]] auto current() const -> lexer::Token const& {
            if (is_at_end()) {
                return m_tokens.back();
            }
            return m_tokens.at(m_index);
        }

        [[nodiscard]] auto advance() -> lexer::Token const& {
            if (not is_at_end()) {
                return m_tokens.at(m_index++);
            }
            return m_tokens.back();
        }

        [[nodiscard]] auto match(lexer::TokenType const type) -> tl::optional<lexer::Token const&> {
            if (current().type() != type) {
                return tl::nullopt;
            }
            return advance();
        }

        auto expect(lexer::TokenType const type) -> lexer::Token const& {
            auto const matched = match(type);
            if (not matched.has_value()) {
                throw ParserError{ std::format(
                        "Expected token type '{}', got '{}'.",
                        utils::enum_to_string(type),
                        utils::enum_to_string(current().type())
                ) };
            }
            return matched.value();
        }

        [[nodiscard]] auto statement() -> std::unique_ptr<Statement> {
            using lexer::TokenType;
            if (auto const _ = match(TokenType::Println)) {
                expect(TokenType::LeftParenthesis);
                auto argument = expression();
                expect(TokenType::RightParenthesis);
                expect(TokenType::Semicolon);
                return std::make_unique<Println>(std::move(argument));
            }
            throw ParserError{ std::format("Unexpected token '{}'.", utils::enum_to_string(current().type())) };
        }

        [[nodiscard]] auto expression() -> std::unique_ptr<Expression> {
            using lexer::TokenType;
            if (auto const string_literal_token = match(TokenType::StringLiteral)) {
                return std::make_unique<StringLiteral>(string_literal_token.value());
            }
            if (auto const unsigned_integer_token = match(TokenType::UnsignedIntegerLiteral)) {
                return std::make_unique<UnsignedIntegerLiteral>(unsigned_integer_token.value());
            }
            throw ParserError{ std::format("Expected expression, got '{}'.", utils::enum_to_string(current().type())) };
        }
    };

    [[nodiscard]] auto parse(std::span<lexer::Token const> const tokens) -> std::vector<std::unique_ptr<Statement>> {
        auto parser = Parser{ tokens };
        return parser.parse();
    }
} // namespace parser
