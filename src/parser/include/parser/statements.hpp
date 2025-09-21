#pragma once

#include "expressions.hpp"
#include "error.hpp"
#include <memory>

namespace parser {
    class Statement {
    public:
        [[nodiscard]] Statement() = default;
        Statement(Statement const& other) = delete;
        Statement(Statement&& other) noexcept = default;
        Statement& operator=(Statement const& other) = delete;
        Statement& operator=(Statement&& other) noexcept = default;
        virtual ~Statement() = default;
    };

    class Print final : public Statement {
    private:
        std::unique_ptr<Expression> m_argument;

    public:
        [[nodiscard]] explicit Print(std::unique_ptr<Expression> m_argument) : m_argument{ std::move(m_argument) } { }

        [[nodiscard]] auto argument() const -> Expression const& {
            return *m_argument;
        }
    };

    class Println final : public Statement {
    private:
        std::unique_ptr<Expression> m_argument;

    public:
        [[nodiscard]] explicit Println(std::unique_ptr<Expression> m_argument) : m_argument{ std::move(m_argument) } { }

        [[nodiscard]] auto argument() const -> Expression const& {
            return *m_argument;
        }
    };
} // namespace parser
