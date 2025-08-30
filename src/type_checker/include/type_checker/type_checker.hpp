#pragma once

#include "statements.hpp"
#include <memory>
#include <parser/parser.hpp>
#include <vector>

namespace type_checker {
    class InvalidTypeError : public std::invalid_argument {
    public:
        [[nodiscard]] explicit InvalidTypeError(std::string const& msg) : std::invalid_argument{ msg } { }
    };

    [[nodiscard]] auto check_types(std::vector<std::unique_ptr<parser::Statement>> statements)
            -> std::vector<std::unique_ptr<Statement>>;
} // namespace type_checker
