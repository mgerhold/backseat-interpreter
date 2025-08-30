#include <type_checker/type_checker.hpp>
#include <experimental/meta>
#include <ranges>
#include <algorithm>
#include "checking.hpp"

namespace type_checker {

    [[nodiscard]] auto check_types(std::vector<std::unique_ptr<parser::Statement>> statements)
            -> std::vector<std::unique_ptr<Statement>> {
        auto program = std::vector<std::unique_ptr<Statement>>{};
        program.reserve(statements.size());
        for (auto const& statement : statements) {
            program.push_back(check_child_types<parser::Statement, Statement>(*statement));
        }
        return program;
    }

}
