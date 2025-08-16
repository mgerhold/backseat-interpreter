#include <generated.hpp>
#include <print>
#include <token_type.hpp>
#include <utils/types.hpp>
#include <experimental/meta>
#include <ranges>
#include <cctype>

auto print_char_mask(lexer::CharMask const& mask) -> void {
    for (auto const i : std::views::iota(0uz, mask.size())) {
        if (mask.contains(static_cast<char>(i))) {
            if (std::isspace(static_cast<unsigned char>(i))) {
                std::print("<space>");
            } else if (std::isprint(static_cast<unsigned char>(i))) {
                std::print("{}", static_cast<char>(i));
            } else {
                std::print("<?>");
            }
        }
    }
}

auto main() -> int {
    std::println("There are {} patterns.", lexer::patterns.size());
    template for (constexpr auto token_type : std::define_static_array(enumerators_of(^^lexer::TokenType))) {
        auto const& pattern = lexer::patterns.at(static_cast<usize>([: token_type :]));
        std::print("\n    {}:", display_string_of(token_type));
        if (not pattern.should_emit) {
            std::print(" (discarded)");
        }
        std::println();

        for (auto const i : std::views::iota(0uz, pattern.states.size())) {
            auto const& state = pattern.states.at(i);
            std::print("      [State {}]", i, state.transitions.size());
            if (state.is_final) {
                std::print(" (final)");
            }
            std::println();

            for (auto const& transition : state.transitions) {
                std::print("        ");
                print_char_mask(transition.char_mask);
                std::println(" -> [State {}]", transition.next_state);
            }
        }
    }
}
