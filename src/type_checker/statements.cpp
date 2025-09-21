#include "checking.hpp"
#include <type_checker/statements.hpp>
#include <type_checker/type_checker.hpp>

namespace type_checker {

    [[nodiscard]] static auto check_print_argument_type(parser::Expression const& argument) -> std::unique_ptr<Expression> {
        auto checked_expression = check_child_types<parser::Expression, Expression>(argument);
        if (not checked_expression->data_type().as_builtin_type().has_value()
            or (checked_expression->data_type().as_builtin_type().value() != BuiltinDataType::String
                and checked_expression->data_type().as_builtin_type().value() != BuiltinDataType::U64)) {
            throw InvalidTypeError{ "Invalid argument type for printing." };
        }
        return checked_expression;
    }

    [[nodiscard]] Print::Print(parser::Expression const& argument)
        : m_argument{ check_print_argument_type(argument) } { }

    [[nodiscard]] Println::Println(parser::Expression const& argument)
        : m_argument{ check_print_argument_type(argument) } { }

} // namespace type_checker
