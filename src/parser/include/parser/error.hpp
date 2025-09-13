#pragma once

#include <stdexcept>

namespace parser {
    class ParserError final : public std::runtime_error {
    public:
        [[nodiscard]] explicit ParserError(std::string const& basic_string) : runtime_error{ basic_string } { }
    };
}
