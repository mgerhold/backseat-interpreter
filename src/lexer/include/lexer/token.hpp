#pragma once

#include "source_location.hpp"
#include <token_type.hpp>

namespace lexer {

    class Token final {
    private:
        SourceLocation m_source_location;
        TokenType m_type;

    public:
        [[nodiscard]] Token(SourceLocation const& source_location, TokenType const type)
            : m_source_location{ source_location }, m_type{ type } { }

        [[nodiscard]] auto source_location() const -> SourceLocation const& {
            return m_source_location;
        }

        [[nodiscard]] auto type() const -> TokenType {
            return m_type;
        }
    };

}
