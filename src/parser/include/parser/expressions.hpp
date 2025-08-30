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

        [[nodiscard]] virtual auto as_string_literal() const -> tl::optional<StringLiteral const&> {
            return tl::nullopt;
        }
    };

    class StringLiteral final : public Expression {
    private:
        lexer::Token m_token;

    public:
        [[nodiscard]] StringLiteral(lexer::Token const token) : m_token{ token } { }
        [[nodiscard]] auto as_string_literal() const -> tl::optional<StringLiteral const&> override {
            return *this;
        }

        [[nodiscard]] auto token() const -> lexer::Token const& {
            return m_token;
        }
    };
} // namespace parser
