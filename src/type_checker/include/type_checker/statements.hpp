#pragma once

#include "expressions.hpp"
#include <parser/parser.hpp>
#include <memory>

namespace type_checker {
    class Statement {
    public:
        [[nodiscard]] Statement() = default;
        Statement(Statement const& other) = delete;
        Statement(Statement&& other) noexcept = default;
        Statement& operator=(Statement const& other) = delete;
        Statement& operator=(Statement&& other) noexcept = default;
        virtual ~Statement() = default;
    };

    class Println final : public Statement {
    private:
        std::unique_ptr<Expression> m_argument;

    public:
        [[nodiscard]] explicit Println(parser::Expression const& argument);

        [[nodiscard]] auto argument() const -> std::unique_ptr<Expression> const& {
            return m_argument;
        }
    };

}
