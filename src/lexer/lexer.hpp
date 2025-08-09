#pragma once

#include "regex.hpp"
#include "token.hpp"
#include "token_definitions.hpp"
#include <algorithm>
#include <experimental/meta>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace lexer {
    class Lexer final {
    private:
        std::string_view m_source;
        usize m_offset = 0uz;

    public:
        explicit Lexer(std::string_view const source) : m_source{ source } { }

        auto tokenize() -> std::vector<Token> {
            while (not is_at_end()) {
                // skip_whitespace();
            }
            return {};
        }

        auto is_at_end() const -> bool {
            return m_offset >= m_source.length();
        }

        auto current() const -> char {
            return is_at_end() ? '\0' : m_source[m_offset];
        }

        auto advance() -> void {
            if (not is_at_end()) {
                ++m_offset;
            }
        }
    };
} // namespace lexer
