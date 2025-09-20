#pragma once

#include "data_type.hpp"
#include "errors.hpp"
#include <algorithm>
#include <charconv>
#include <lexer/token.hpp>
#include <memory>
#include <utils/enum_to_string.hpp>

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
                std::pair{  'n', '\n' },
                std::pair{  't', '\t' },
                std::pair{  'f', '\f' },
                std::pair{  'r', '\r' },
                std::pair{ '\\', '\\' },
                std::pair{  '"',  '"' },
            };
            auto const inside_quotes =
                    m_token.source_location().lexeme().substr(1, m_token.source_location().length() - 2);
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
                auto const pair =
                        std::ranges::find_if(escape_characters, [second](auto const& p) { return p.first == second; });
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
            auto const without_suffix =
                    m_token.source_location().lexeme().substr(0, m_token.source_location().length() - suffix_length);
            static constexpr auto thousands_separator = '\'';
            auto without_separators = std::string{};
            std::copy_if(
                    without_suffix.begin(),
                    without_suffix.end(),
                    std::back_inserter(without_separators),
                    [](char const c) { return c != thousands_separator; }
            );
            auto value = std::uint64_t{};
            auto const [_, ec] =
                    std::from_chars(without_separators.data(), without_separators.data() + without_separators.length(), value);
            if (ec != std::errc{}) {
                throw std::runtime_error{ "Invalid unsigned integer literal (lexer bug?)." };
            }
            return value;
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
            : Expression{ get_resulting_data_type(*lhs, operator_token, *rhs) },
              m_lhs{ std::move(lhs) },
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

    private:
        [[nodiscard]] static consteval auto get_resulting_data_type(
                BuiltinDataType const lhs_type,
                lexer::TokenType const operator_token_type,
                BuiltinDataType const rhs_type
        ) -> tl::optional<BuiltinDataType> {
            switch (operator_token_type) {
                case lexer::TokenType::Plus:
                case lexer::TokenType::Minus:
                case lexer::TokenType::Asterisk:
                case lexer::TokenType::ForwardSlash:
                case lexer::TokenType::Mod:
                    if (lhs_type == BuiltinDataType::U64 and rhs_type == BuiltinDataType::U64) {
                        return BuiltinDataType::U64;
                    }
                    return tl::nullopt;
                default:
                    return tl::nullopt;
            }
        }

        [[nodiscard]] static consteval auto get_data_type_matrix(lexer::TokenType const operator_token_type) {
            static constexpr auto num_data_types = enumerators_of(dealias(^^BuiltinDataType)).size();
            auto matrix = std::array<std::array<tl::optional<BuiltinDataType>, num_data_types>, num_data_types>{};
            auto lhs_index = 0uz;
            template for (constexpr auto lhs_type :
                          std::define_static_array(enumerators_of(dealias(^^BuiltinDataType)))) {
                auto rhs_index = 0uz;
                template for (constexpr auto rhs_type :
                              std::define_static_array(enumerators_of(dealias(^^BuiltinDataType)))) {
                    if (std::to_underlying([:lhs_type:]) != lhs_index
                        or std::to_underlying([:rhs_type:]) != rhs_index) {
                        throw std::runtime_error{ "Inconsistent data type enumeration." };
                    }
                    matrix[lhs_index][rhs_index] =
                            get_resulting_data_type([:lhs_type:], operator_token_type, [:rhs_type:]);
                    ++rhs_index;
                }
                ++lhs_index;
            }
            return matrix;
        }

        [[nodiscard]] static auto
        get_resulting_data_type(Expression const& lhs, lexer::Token const& operator_token, Expression const& rhs)
                -> std::unique_ptr<DataType> {
            auto const lhs_builtin_type = lhs.data_type().as_builtin_type();
            auto const rhs_builtin_type = rhs.data_type().as_builtin_type();

            if (not lhs_builtin_type.has_value()) {
                throw InvalidTypeError{ "Left-hand side of binary operator has invalid type." };
            }
            if (not rhs_builtin_type.has_value()) {
                throw InvalidTypeError{ "Right-hand side of binary operator has invalid type." };
            }

            static constexpr auto plus_matrix = get_data_type_matrix(lexer::TokenType::Plus);
            static constexpr auto minus_matrix = get_data_type_matrix(lexer::TokenType::Minus);
            static constexpr auto asterisk_matrix = get_data_type_matrix(lexer::TokenType::Asterisk);
            static constexpr auto forward_slash_matrix = get_data_type_matrix(lexer::TokenType::ForwardSlash);
            static constexpr auto mod_matrix = get_data_type_matrix(lexer::TokenType::Mod);

            auto result_type = tl::optional<BuiltinDataType>{};

            auto const lhs_index = static_cast<usize>(lhs_builtin_type.value());
            auto const rhs_index = static_cast<usize>(rhs_builtin_type.value());

            switch (operator_token.type()) {
                case lexer::TokenType::Plus:
                    result_type = plus_matrix[lhs_index][rhs_index];
                    break;
                case lexer::TokenType::Minus:
                    result_type = minus_matrix[lhs_index][rhs_index];
                    break;
                case lexer::TokenType::Asterisk:
                    result_type = asterisk_matrix[lhs_index][rhs_index];
                    break;
                case lexer::TokenType::ForwardSlash:
                    result_type = forward_slash_matrix[lhs_index][rhs_index];
                    break;
                case lexer::TokenType::Mod:
                    result_type = mod_matrix[lhs_index][rhs_index];
                    break;
                default:
                    throw std::runtime_error{ "Unsupported binary operator." };
            }

            if (not result_type.has_value()) {
                throw InvalidTypeError{ std::format(
                        "Invalid operand types '{}' and '{}' for operator '{}'.",
                        utils::enum_to_string(lhs_builtin_type.value()),
                        utils::enum_to_string(rhs_builtin_type.value()),
                        operator_token.source_location().lexeme()
                ) };
            }

            return DataType::from_builtin_type(result_type.value());
        }
    };
} // namespace type_checker
