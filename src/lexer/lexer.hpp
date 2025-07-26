#pragma once

#include "token.hpp"
#include <algorithm>
#include <experimental/meta>
#include <print>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

template<typename T>
struct RevealType;

namespace lexer {
    class Lexer final {
    private:
        std::string_view m_source;
        usize m_offset = 0uz;

    public:
        explicit Lexer(std::string_view const source) : m_source{ source } { }

        auto tokenize() -> std::vector<Token> {
            while (not is_at_end()) {
                skip_whitespace();
                execute_state_machine<"">();
            }
            return {};
        }

        auto skip_whitespace() -> void {
            while (not is_at_end() and std::isspace(static_cast<unsigned char>(current()))) {
                advance();
            }
        }

        [[nodiscard]] static constexpr auto is_valid_identifier_start(char const c) -> bool {
            return std::isalpha(static_cast<unsigned char>(c)) or c == '_';
        }

        [[nodiscard]] static constexpr auto is_valid_identifier_continuation(char const c) -> bool {
            return is_valid_identifier_start(c) or std::isdigit(static_cast<unsigned char>(c));
        }

        template<utils::StaticString state>
        [[nodiscard]] static consteval auto get_possible_successors() -> decltype(auto) {
            static constexpr auto context = std::meta::access_context::current();

            auto next_chars = std::vector<char>{};
            template for (constexpr auto description : define_static_array(members_of(^^token_descriptions, context))) {
                static constexpr auto dealiased = dealias(description);
                static constexpr auto lexeme = [:template_arguments_of(dealiased).front():].view();
                if constexpr (lexeme.starts_with(state.view()) and lexeme.length() > state.view().length()) {
                    static constexpr auto c = lexeme[state.view().length()];
                    next_chars.push_back(c);
                }
            }
            std::ranges::sort(next_chars);
            auto const [first, last] = std::ranges::unique(next_chars);
            next_chars.erase(first, last);
            return std::define_static_array(next_chars);
        }

        template<utils::StaticString state, char successor>
        [[nodiscard]] static consteval auto get_next_state() -> auto {
            static constexpr auto next_state = []() -> decltype(auto) {
                constexpr auto s = std::string{ state.view() } + successor;
                static constexpr auto array = std::define_static_array(s);
                return array;
            }();
            static constexpr auto next_state_view = std::string_view{
                next_state.begin(),
                next_state.end(),
            };
            return utils::StaticString{[:std::meta::reflect_constant_string(next_state_view):] };
        }

        template<utils::StaticString state>
        auto execute_state_machine() -> void {
            // std::println("State is {}", state.view());
            static constexpr auto successors = get_possible_successors<state>();
            if constexpr (std::size(successors) == 0) {
                if (not is_valid_identifier_continuation(current())) {
                    std::println("Found lexeme: {}", state.view());
                    return;
                } else {
                    // We have an identifier that starts like a keyword, but is not a keyword.
                    // todo: implement
                    throw std::runtime_error{ "Not implemented" };
                }
            } else {
                // std::println("current char: {}", current());
                static constexpr auto successors = get_possible_successors<state>();
                switch (current()) {
                    template for (constexpr char successor : successors) {
                        case static_cast<char>(successor):
                            // std::println("Successor {} found. {}", static_cast<char>(successor), static_cast<int>(successor));
                            // std::println("Next state is: {}", get_next_state<state, static_cast<char>(successor)>().view());
                            advance();
                            execute_state_machine<get_next_state<state, static_cast<char>(successor)>()>();
                            return; // Cannot use `break`, as this would break out of the `template for`.
                    }
                    default:
                        std::println("Default case");
                        std::exit(1);
                }
            }
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
