#pragma once

#include <span>
#include "statements.hpp"
#include <lexer/token.hpp>
#include <vector>
#include <memory>
#include "error.hpp"

namespace parser {
    [[nodiscard]] auto parse(std::span<lexer::Token const> tokens) -> std::vector<std::unique_ptr<Statement>>;
}
