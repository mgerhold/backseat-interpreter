#pragma once

#include "token.hpp"
#include <string_view>
#include <vector>

namespace lexer {
    class LexerError final : public std::runtime_error {
    private:
        SourceLocation m_source_location;
    public:
        [[nodiscard]] explicit LexerError(std::string const& message, SourceLocation const& source_location)
            : std::runtime_error{ message }, m_source_location{ source_location } { }

        [[nodiscard]] auto source_location() const -> SourceLocation const& {
            return m_source_location;
        }
    };

    [[nodiscard]] auto tokenize(
        std::string_view filename,
        std::string_view source
    ) -> std::vector<Token>;
}
