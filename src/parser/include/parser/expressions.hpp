#pragma once

#include "error.hpp"
#include <charconv>
#include <lexer/token.hpp>
#include <tl/optional.hpp>

namespace parser {
    class StringLiteral;

    class Expression {
    public:
        [[nodiscard]] Expression() = default;
        Expression(Expression const& other) = delete;
        Expression(Expression&& other) noexcept = default;
        Expression& operator=(Expression const& other) = delete;
        Expression& operator=(Expression&& other) noexcept = default;
        virtual ~Expression() = default;
    };

    class StringLiteral final : public Expression {
    private:
        lexer::Token m_token;

    public:
        [[nodiscard]] explicit StringLiteral(lexer::Token const& token) : m_token{ token } { }

        [[nodiscard]] auto token() const -> lexer::Token const& {
            return m_token;
        }
    };

    class UnsignedIntegerLiteral final : public Expression {
    private:
        lexer::Token m_token;

    public:
        [[nodiscard]] explicit UnsignedIntegerLiteral(lexer::Token const& token) : m_token{ token } {
            static constexpr auto suffix_length = std::string_view{ "_u64" }.length();
            auto const without_suffix =
                    m_token.source_location().lexeme().substr(0, m_token.source_location().length() - suffix_length);
            auto value = std::uint64_t{};
            auto const [_, ec] =
                    std::from_chars(without_suffix.data(), without_suffix.data() + without_suffix.length(), value);
            if (ec == std::errc::result_out_of_range) {
                throw ParserError{ "Unsigned integer literal out of range." };
            }
        }

        [[nodiscard]] auto token() const -> lexer::Token const& {
            return m_token;
        }
    };

    class BinaryOperator final : public Expression {
    private:
        std::unique_ptr<Expression> m_lhs;
        lexer::Token m_operator_token;
        std::unique_ptr<Expression> m_rhs;

    public:
        [[nodiscard]] explicit BinaryOperator(
                std::unique_ptr<Expression> lhs,
                lexer::Token const& operator_token,
                std::unique_ptr<Expression> rhs
        )
            : m_lhs{ std::move(lhs) },
              m_operator_token{ operator_token },
              m_rhs{ std::move(rhs) } { }

        [[nodiscard]] auto lhs() const -> Expression const& {
            return *m_lhs;
        }

        [[nodiscard]] auto operator_token() const -> lexer::Token const& {
            return m_operator_token;
        }

        [[nodiscard]] auto rhs() const -> Expression const& {
            return *m_rhs;
        }
    };
} // namespace parser
