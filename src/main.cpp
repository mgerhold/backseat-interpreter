#include "lexer/lexer.hpp"
#include "lexer/token_definitions.hpp"
#include <bit>
#include <experimental/meta>
#include <print>
#include <string>
#include <string_view>
#include <vector>

consteval auto to_string(usize number) -> std::string {
    if (number == 0uz) {
        return "0";
    }
    auto str = std::string{};
    while (number > 0uz) {
        str = static_cast<char>('0' + (number % 10)) + str;
        number /= 10;
    }
    return str;
}

consteval auto f() -> auto {
    auto patterns = lexer::get_pattern_descriptions();
    auto sequence_parser = lexer::SequenceParser{ std::move(patterns.at(3)) };
    auto const pattern = sequence_parser.parse();

    auto texts = std::vector<char const*>{};
    for (auto const i : std::views::iota(0uz, pattern.states.size())) {
        auto const& state = pattern.states.at(i);
        texts.push_back(std::define_static_string(std::string_view{ std::string{ "State #" } + to_string(i) }));
        for (auto const& transition : state.transitions) {
            auto mask = std::string{ "  " };
            for (auto c = static_cast<char>(32); c <= 126; ++c) {
                if (transition.char_mask.contains(c)) {
                    mask += c;
                }
            }
            mask += "  Next state: " + to_string(transition.next_state);
            texts.push_back(std::define_static_string(std::string_view{ mask }));
        }
    }
    return texts;
}

consteval auto get_num_states() -> usize {
    auto patterns = lexer::get_pattern_descriptions();
    auto sequence_parser = lexer::SequenceParser{ std::move(patterns.at(4)) };
    return sequence_parser.m_state_indices.size();
}

int main() {
    // Lexer
    // auto lexer = lexer::Lexer{ "int if else >= > while loop for" };
    // lexer.tokenize();
    // Parser
    // Type-Checking
    // Interpreter
    static constexpr auto transitions = std::define_static_array(f());
    for (auto const& mask : transitions) {
        std::println("{}", mask);
    }
    // std::println("There are {} elements in the first sequence.", num_elements);
    // std::println("There are {} successors in the first sequence.", num_successors);
}
