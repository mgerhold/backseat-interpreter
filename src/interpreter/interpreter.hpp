#pragma once

#include "error.hpp"
#include "values.hpp"
#include <algorithm>
#include <experimental/meta>
#include <print>
#include <type_checker/type_checker.hpp>

namespace interpreter {

    class Interpreter final {
    private:
        std::vector<std::unique_ptr<type_checker::Statement>> m_program;

    public:
        [[nodiscard]] explicit Interpreter(std::vector<std::unique_ptr<type_checker::Statement>> program)
            : m_program{ std::move(program) } { }

        auto run() -> void {
            for (auto const& statement : m_program) {
                interpret_statement(*statement);
            }
        }

    private:
        auto print_value(Value const& value, tl::optional<type_checker::BuiltinDataType> const& builtin_type) -> void {
            if (not builtin_type.has_value()) {
                throw InterpreterError{ "Expected builtin data type." };
            }
            switch (builtin_type.value()) {
                using enum type_checker::BuiltinDataType;

                case String:
                    std::print("{}", dynamic_cast<interpreter::String const&>(value).data());
                    break;
                case U64:
                    std::print("{}", dynamic_cast<interpreter::U64 const&>(value).value());
                    break;
                default:
                    throw InterpreterError{ "Unsupported builtin data type." };
            }
        }

        auto interpret(type_checker::Print const& statement) -> void {
            auto const value = evaluate_expression(*statement.argument());
            auto const builtin_type = statement.argument()->data_type().as_builtin_type();
            print_value(*value, builtin_type);
        }

        auto interpret(type_checker::Println const& statement) -> void {
            auto const value = evaluate_expression(*statement.argument());
            auto const builtin_type = statement.argument()->data_type().as_builtin_type();
            print_value(*value, builtin_type);
            std::println();
        }

        auto interpret_statement(type_checker::Statement const& statement) -> void {
            static constexpr auto context = std::meta::access_context::current();
            template for (constexpr auto member : std::define_static_array(members_of(^^type_checker, context))) {
                if constexpr (is_type(member) and is_class_type(member)) {
                    static constexpr auto does_inherit_base =
                            std::ranges::any_of(bases_of(member, context), [](auto const& base) {
                                return is_same_type(type_of(base), ^^type_checker::Statement);
                            });
                    if constexpr (does_inherit_base) {
                        auto const downcasted = dynamic_cast<[:member:] const*>(std::addressof(statement));
                        if (downcasted != nullptr) {
                            return interpret(*downcasted);
                        }
                    }
                }
            }
        }

        auto evaluate(type_checker::StringLiteral const& expression) -> std::unique_ptr<Value> {
            return std::make_unique<String>(expression.to_escaped_string());
        }

        auto evaluate(type_checker::UnsignedIntegerLiteral const& expression) -> std::unique_ptr<Value> {
            return std::make_unique<U64>(expression.value());
        }

        auto evaluate(U64 const& lhs, lexer::TokenType const operator_token_type, U64 const& rhs)
                -> std::unique_ptr<U64> {
            switch (operator_token_type) {
                case lexer::TokenType::Plus:
                    return std::make_unique<U64>(lhs.value() + rhs.value());
                case lexer::TokenType::Minus:
                    return std::make_unique<U64>(lhs.value() - rhs.value());
                case lexer::TokenType::Asterisk:
                    return std::make_unique<U64>(lhs.value() * rhs.value());
                case lexer::TokenType::ForwardSlash:
                    if (rhs.value() == 0) {
                        throw InterpreterError{ "Division by zero." };
                    }
                    return std::make_unique<U64>(lhs.value() / rhs.value());
                case lexer::TokenType::Mod:
                    if (rhs.value() == 0) {
                        throw InterpreterError{ "Division by zero." };
                    }
                    return std::make_unique<U64>(lhs.value() % rhs.value());
                default:
                    throw std::runtime_error{ "Unreachable" };
            }
        }

        auto evaluate(type_checker::BinaryOperator const& expression) -> std::unique_ptr<Value> {
            auto lhs = evaluate_expression(expression.lhs());
            auto rhs = evaluate_expression(expression.rhs());
            auto const operator_token = expression.operator_token();

            auto const lhs_u64 = dynamic_cast<U64 const*>(lhs.get());
            auto const rhs_u64 = dynamic_cast<U64 const*>(rhs.get());
            if (lhs_u64 != nullptr and rhs_u64 != nullptr) {
                return evaluate(*lhs_u64, operator_token.type(), *rhs_u64);
            }

            throw std::runtime_error{ "Unreachable" };
        }

        auto evaluate_expression(type_checker::Expression const& expression) -> std::unique_ptr<Value> {
            static constexpr auto context = std::meta::access_context::current();
            template for (constexpr auto member : std::define_static_array(members_of(^^type_checker, context))) {
                if constexpr (is_type(member) and is_class_type(member)) {
                    static constexpr auto does_inherit_base =
                            std::ranges::any_of(bases_of(member, context), [](auto const& base) {
                                return is_same_type(type_of(base), ^^type_checker::Expression);
                            });
                    if constexpr (does_inherit_base) {
                        auto const downcasted = dynamic_cast<[:member:] const*>(std::addressof(expression));
                        if (downcasted != nullptr) {
                            return evaluate(*downcasted);
                        }
                    }
                }
            }
            throw std::runtime_error{ "Unreachable" };
        }
    };

} // namespace interpreter
