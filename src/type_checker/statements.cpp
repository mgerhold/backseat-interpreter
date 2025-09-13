#include "checking.hpp"
#include <type_checker/type_checker.hpp>
#include <type_checker/statements.hpp>

namespace type_checker {

    [[nodiscard]] Println::Println(parser::Expression const& argument) {
        auto checked_expression = check_child_types<parser::Expression, Expression>(argument);
        if (
            not checked_expression->data_type().as_builtin_type().has_value()
            or (
                checked_expression->data_type().as_builtin_type().value() != BuiltinDataType::String
                and checked_expression->data_type().as_builtin_type().value() != BuiltinDataType::U64
            )
        ) {
            throw InvalidTypeError{ "println statement expects String argument." };
        }
        m_argument = std::move(checked_expression);
    }

} // namespace type_checker
