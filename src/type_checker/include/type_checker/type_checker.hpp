#pragma once

#include "statements.hpp"
#include <memory>
#include <parser/parser.hpp>
#include <vector>
#include "errors.hpp"

namespace type_checker {
    [[nodiscard]] auto check_types(std::vector<std::unique_ptr<parser::Statement>> statements)
            -> std::vector<std::unique_ptr<Statement>>;
} // namespace type_checker
