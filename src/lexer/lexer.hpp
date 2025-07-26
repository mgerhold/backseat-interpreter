#pragma once

#include "token.hpp"
#include <experimental/meta>
#include <print>
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
                std::println("Offset: {}, Current: '{}'", m_offset, current());
                execute_state_machine<"">();
            }
            return {};
        }

        template<utils::StaticString state>
        auto execute_state_machine() -> void {
            static constexpr auto context = std::meta::access_context::current();

            switch (current()) {
                template for (constexpr auto description :
                              define_static_array(members_of(^^token_descriptions, context))) {
                    static constexpr auto dealiased = dealias(description);
                    static constexpr auto lexeme_array = std::define_static_array([: template_arguments_of(dealiased).front() :].view());
                    static constexpr auto lexeme = std::string_view{ lexeme_array.begin(), lexeme_array.end() };

                    if constexpr (lexeme.starts_with(state.view()) and lexeme.length() > state.view().length()) {
                        static constexpr auto c = lexeme[state.view().length()];
                        case c: {
                            static constexpr auto next_state = []() -> decltype(auto) {
                                constexpr auto s = std::string{ state.view() } + c;
                                static constexpr auto array = std::define_static_array(s);
                                return array;
                            }();
                            static constexpr auto next_state_view = std::string_view{
                                next_state.begin(),
                                next_state.end(),
                            };
                            advance();
                            if constexpr (next_state_view.length() == lexeme.length()) {
                                std::println("Found lexeme: {}", lexeme);
                            } else {
                                execute_state_machine<([:std::meta::reflect_constant_string(next_state_view):])>();
                            }
                            return;
                        }
                    }
                }
                default:
                    std::println("Default case");
                    std::exit(1);
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
