#include <format>
#include <parser/parser.hpp>
#include <tl/optional.hpp>
#include <utils/enum_to_string.hpp>
#include <token_type.hpp>
#include <memory>
#include "precedence.hpp"
#include "parser_table.hpp"
#include <parser/statements.hpp>

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
                auto argument = expression(Precedence::Unknown);
                expect(TokenType::RightParenthesis);
                expect(TokenType::Semicolon);
                return std::make_unique<Println>(std::move(argument));
            }
            throw ParserError{ std::format("Unexpected token '{}'.", utils::enum_to_string(current().type())) };
        }

        [[nodiscard]] auto expression(Precedence const precedence) -> std::unique_ptr<Expression> {
            auto const prefix_parser = current_table_record().prefix_parser;
            if (prefix_parser == nullptr) {
                throw ParserError{
                    std::format(
                        "Unexpected token of type '{}'.",
                        utils::enum_to_string(current().type())
                    )
                };
            }
            auto first_operand = std::invoke(prefix_parser, *this);

            // This could still only be the first operand of a binary operator.
            while (true) {
                auto const [_, infix_parser, infix_precedence] = current_table_record();
                if (infix_precedence <= precedence or infix_parser == nullptr) {
                    return first_operand;
                }
                first_operand = std::invoke(infix_parser, *this, std::move(first_operand));
            }
        }

        [[nodiscard]] auto string_literal() -> std::unique_ptr<Expression> {
            return std::make_unique<StringLiteral>(expect(lexer::TokenType::StringLiteral));
        }

        [[nodiscard]] auto unsigned_integer_literal() -> std::unique_ptr<Expression> {
            return std::make_unique<UnsignedIntegerLiteral>(expect(lexer::TokenType::UnsignedIntegerLiteral));
        }

        // clang-format off
        static inline auto parser_table = create_parser_table(
            ParserTableEntry<lexer::TokenType::Print>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Println>               { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LowercaseFunction>     { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Colon>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Comma>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::TildeArrow>            { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::EndOfFile>             { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Semicolon>             { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Plus>                  { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Minus>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Asterisk>              { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Mod>                   { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::ForwardSlash>          { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LeftParenthesis>       { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::RightParenthesis>      { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LeftCurlyBracket>      { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::RightCurlyBracket>     { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Let>                   { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Equals>                { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::SignedIntegerLiteral>  { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::UnsignedIntegerLiteral>{
                &Parser::unsigned_integer_literal,
                nullptr,
                Precedence::Unknown
            },
            ParserTableEntry<lexer::TokenType::CharLiteral>           { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::StringLiteral>         { &Parser::string_literal,   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::BoolLiteral>           { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::And>                   { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Or>                    { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Not>                   { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::If>                    { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Else>                  { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Loop>                  { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Break>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Continue>              { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::While>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Do>                    { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::For>                   { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Mutable>               { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Const>                 { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::EqualsEquals>          { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::ExclamationEquals>     { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::GreaterThan>           { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LessThan>              { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::GreaterOrEquals>       { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LessOrEquals>          { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Return>                { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::NothingLiteral>        { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::UppercaseFunction>     { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LineComment>           { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::Whitespace>            { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::LowercaseIdentifier>   { nullptr,                   nullptr,         Precedence::Unknown },
            ParserTableEntry<lexer::TokenType::UppercaseIdentifier>   { nullptr,                   nullptr,         Precedence::Unknown }
        );
        // clang-format on

        [[nodiscard]] auto current_table_record() const -> ParserTableRecord const& {
            return parser_table.at(std::to_underlying(current().type()));
        }
    };

    [[nodiscard]] auto parse(std::span<lexer::Token const> const tokens) -> std::vector<std::unique_ptr<Statement>> {
        auto parser = Parser{ tokens };
        return parser.parse();
    }

} // namespace parser
