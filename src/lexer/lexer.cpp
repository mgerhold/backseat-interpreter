#include "lexer.hpp"
#include <generated.hpp>
#include <array>
#include <ranges>
#include <utility>
#include <experimental/meta>
#include <optional>
#include <print>
#include <stdexcept>

namespace lexer {
    struct PatternState final {
        bool is_matching;
        usize state_index;
    };

    struct LexerState final {
        std::array<PatternState, patterns.size()> pattern_states{};
    };

    class Lexer final {
    private:
        std::string_view m_filename;
        std::string_view m_source;
        std::vector<Token> m_tokens;
        usize m_offset{ 0 };

    public:
        [[nodiscard]] Lexer(std::string_view const filename, std::string_view const source)
            : m_filename{ filename }, m_source{ source } { }

        [[nodiscard]] auto tokenize() -> std::vector<Token> {
            m_offset = 0uz;
            while (m_tokens.empty() or m_tokens.at(m_tokens.size() - 1).type() != TokenType::EndOfFile) {
                auto const start_offset = m_offset;
                static constexpr auto initial_lexer_state = [] {
                    auto state = LexerState{};
                    state.pattern_states.fill(PatternState{ .is_matching = true, .state_index = 0uz });
                    return state;
                }();
                auto const matched_token_type = match(initial_lexer_state);
                if (not matched_token_type.has_value()) {
                    throw LexerError{
                        "Invalid token.",
                        SourceLocation{
                            m_filename,
                            m_source,
                            start_offset,
                            m_offset - start_offset,
                        },
                    };
                }

                auto const should_emit = patterns.at(std::to_underlying(matched_token_type.value())).should_emit;
                if (not should_emit) {
                    continue;
                }

                auto const source_location = SourceLocation{
                    m_filename,
                    m_source,
                    start_offset,
                    m_offset - start_offset,
                };
                m_tokens.emplace_back(source_location, matched_token_type.value());
            }
            return std::move(m_tokens);
        }

        auto match(LexerState const lexer_state) -> std::optional<TokenType> {
            using std::views::iota;

            auto next_lexer_state = lexer_state;
            auto any_pattern_matched = false;

            template for (constexpr auto pattern_state_index : iota(0uz, patterns.size())) {
                if (lexer_state.pattern_states.at(pattern_state_index).is_matching) {
                    static constexpr auto& states = patterns
                        .at(pattern_state_index)
                        .states;
                    auto does_this_pattern_match = false;
                    for (auto const& transition : states
                        .at(lexer_state.pattern_states.at(pattern_state_index).state_index)
                        .transitions
                    ) {
                        if (transition.char_mask.contains(current())) {
                            next_lexer_state.pattern_states.at(pattern_state_index).state_index = transition.next_state;
                            any_pattern_matched = true;
                            does_this_pattern_match = true;
                            break;
                        }
                    }
                    next_lexer_state.pattern_states.at(pattern_state_index).is_matching = does_this_pattern_match;
                }
            }

            if (any_pattern_matched) {
                advance();
                /*[[clang::musttail]] */return match(next_lexer_state);
            } else {
                for (auto const i : iota(0uz, lexer_state.pattern_states.size())) {
                    auto const& pattern_state = lexer_state.pattern_states.at(i);
                    if (
                        pattern_state.state_index == 0uz
                        or not pattern_state.is_matching
                        or not patterns.at(i).states.at(lexer_state.pattern_states.at(i).state_index).is_final
                    ) {
                        continue;
                    }
                    return TokenType{ i };
                }
                return std::nullopt;
            }
        }

        [[nodiscard]] auto is_at_end() const -> bool {
            return m_offset >= m_source.size();
        }

        [[nodiscard]] auto current() const -> char {
            if (is_at_end()) {
                return '\0';
            }
            return m_source.at(m_offset);
        }

        auto advance() -> void {
            if (is_at_end()) {
                return;
            }
            ++m_offset;
        }
    };

    [[nodiscard]] auto tokenize(
        std::string_view const filename,
        std::string_view const source
    ) -> std::vector<Token> {
        auto lexer = lexer::Lexer{ filename, source };
        return lexer.tokenize();
    }
}
