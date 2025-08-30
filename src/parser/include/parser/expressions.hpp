#pragma once

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
        [[nodiscard]] explicit StringLiteral(lexer::Token const token) : m_token{ token } { }

        [[nodiscard]] auto token() const -> lexer::Token const& {
            return m_token;
        }
    };
} // namespace parser
