#pragma once

#include <parser/parser.hpp>
#include <type_checker/expressions.hpp>
#include <type_checker/statements.hpp>
#include <experimental/meta>
#include <algorithm>

namespace type_checker {

    [[nodiscard]] inline auto check_types(parser::StringLiteral const& expression) -> std::unique_ptr<Expression> {
        return std::make_unique<StringLiteral>(expression.token());
    }

    [[nodiscard]] inline auto check_types(parser::Println const& statement) -> std::unique_ptr<Statement> {
        return std::make_unique<Println>(statement.argument());
    }

    template<typename BaseType, typename Result>
    [[nodiscard]] auto check_child_types(BaseType const& value) -> std::unique_ptr<Result> {
        static constexpr auto context = std::meta::access_context::current();
        template for (constexpr auto member : std::define_static_array(members_of(^^parser, context))) {
            if constexpr (is_type(member) and is_class_type(member)) {
                static constexpr auto does_inherit_base =
                        std::ranges::any_of(bases_of(member, context), [](auto const& base) {
                            return is_same_type(type_of(base), ^^BaseType);
                        });
                if constexpr (does_inherit_base) {
                    auto const downcasted = dynamic_cast<[:member:] const*>(std::addressof(value));
                    if (downcasted != nullptr) {
                        return check_types(*downcasted);
                    }
                }
            }
        }
        return {};
    }

} // namespace type_checker
