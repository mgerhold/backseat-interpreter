#pragma once

#include <type_checker/type_checker.hpp>
#include <algorithm>
#include <experimental/meta>
#include <print>
#include "values.hpp"

namespace interpreter {

    class Interpreter final {
    private:
        std::vector<std::unique_ptr<type_checker::Statement>> m_program;

    public:
        [[nodiscard]] explicit Interpreter(std::vector<std::unique_ptr<type_checker::Statement>> program)
            : m_program{ std::move(program) } {
        }

        auto run() -> void {
            for (auto const& statement : m_program) {
                interpret_statement(*statement);
            }
        }

    private:
        auto interpret(type_checker::Println const& statement) -> void {
            auto const value = evaluate_expression(*statement.argument());
            auto const& string_value = dynamic_cast<interpreter::String const&>(*value);
            std::println("{}", string_value.data());
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

        auto evaluate_expression(type_checker::Expression const& statement) -> std::unique_ptr<Value> {
            static constexpr auto context = std::meta::access_context::current();
            template for (constexpr auto member : std::define_static_array(members_of(^^type_checker, context))) {
                if constexpr (is_type(member) and is_class_type(member)) {
                    static constexpr auto does_inherit_base =
                            std::ranges::any_of(bases_of(member, context), [](auto const& base) {
                                return is_same_type(type_of(base), ^^type_checker::Expression);
                            });
                    if constexpr (does_inherit_base) {
                        auto const downcasted = dynamic_cast<[:member:] const*>(std::addressof(statement));
                        if (downcasted != nullptr) {
                            return evaluate(*downcasted);
                        }
                    }
                }
            }
            throw std::runtime_error{ "Unreachable" };
        }
    };

}
