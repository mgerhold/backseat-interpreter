#pragma once

#include "data_type.hpp"
#include <lexer/token.hpp>
#include <memory>
#include <algorithm>
#include <charconv>

namespace type_checker {
    class StringLiteral;

    class Expression {
    private:
        std::unique_ptr<DataType> m_data_type;

    public:
        [[nodiscard]] explicit Expression(std::unique_ptr<DataType> data_type) : m_data_type{ std::move(data_type) } { }
        Expression(Expression const& other) = delete;
        Expression(Expression&& other) noexcept = default;
        Expression& operator=(Expression const& other) = delete;
        Expression& operator=(Expression&& other) noexcept = default;
        virtual ~Expression() = default;

        [[nodiscard]] auto data_type() const -> DataType const& {
            return *m_data_type;
        }
    };

    class StringLiteral final : public Expression {
    private:
        lexer::Token m_token;

    public:
        [[nodiscard]] explicit StringLiteral(lexer::Token const& token)
            : Expression{ std::make_unique<String>() },
              m_token{ token } { }

        [[nodiscard]] auto to_escaped_string() const -> std::string {
            static constexpr auto escape_characters = std::array{
                std::pair{ 'n', '\n' },
                std::pair{ 't', '\t' },
                std::pair{ 'f', '\f' },
                std::pair{ 'r', '\r' },
                std::pair{ '\\', '\\' },
                std::pair{ '"', '"' },
            };
            auto const inside_quotes = m_token
                .source_location()
                .lexeme()
                .substr(1, m_token.source_location().length() - 2);
            auto result = std::string{};
            result.reserve(inside_quotes.length());

            auto i = 0u;
            while (i < inside_quotes.length() - 1) {
                auto const first = inside_quotes.at(i);
                auto const second = inside_quotes.at(i + 1);
                if (first != '\\') {
                    result += first;
                    ++i;
                    continue;
                }
                auto const pair = std::ranges::find_if(
                    escape_characters,
                    [second](auto const& p) {
                        return p.first == second;
                    }
                );
                if (pair == escape_characters.end()) {
                    throw std::runtime_error{ "Invalid string literal (lexer bug?)." };
                }
                result += pair->second;
                i += 2;
            }

            // Append last character if not already appended.
            if (i < inside_quotes.length()) {
                result += inside_quotes.at(i);
            }

            return result;
        }
    };

    class UnsignedIntegerLiteral final : public Expression {
    private:
        lexer::Token m_token;

    public:
        [[nodiscard]] explicit UnsignedIntegerLiteral(lexer::Token const& token)
            : Expression{ std::make_unique<U64>() },
              m_token{ token } { }

        [[nodiscard]] auto value() const -> std::uint64_t {
            static constexpr auto suffix_length = std::string_view{ "_u64" }.length();
            auto const without_suffix = m_token.source_location().lexeme().substr(0, m_token.source_location().length() - suffix_length);
            auto value = std::uint64_t{};
            auto const [_, ec] = std::from_chars(without_suffix.data(), without_suffix.data() + without_suffix.length(), value);
            if (ec != std::errc{}) {
                throw std::runtime_error{ "Invalid unsigned integer literal (lexer bug?)." };
            }
            return value;
        }
    };
} // namespace type_checker
