#pragma once

#include <stdexcept>

namespace type_checker {
    class InvalidTypeError : public std::invalid_argument {
    public:
        [[nodiscard]] explicit InvalidTypeError(std::string const& msg) : std::invalid_argument{ msg } { }
    };
}
