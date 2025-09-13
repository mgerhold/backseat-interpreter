#pragma once

#include <stdexcept>

namespace interpreter {
    class InterpreterError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
} // namespace interpreter
