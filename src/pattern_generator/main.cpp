#include "token_definitions.hpp"
#include "sequence_parser.hpp"
#include <bit>
#include <experimental/meta>
#include <print>
#include <string>
#include <string_view>
#include <vector>
#include <cstdio>
#include <memory>

struct FileDeleter final {
    auto operator()(FILE* const file) const -> void {
        std::fclose(file);
    }
};

using File = std::unique_ptr<FILE, FileDeleter>;

[[nodiscard]] static auto open_file(char const* const filename) -> File {
    return File{ std::fopen(filename, "w") };
}

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

template<lexer::TokenType token_type>
[[nodiscard]] consteval auto get_pattern() -> std::tuple<lexer::Pattern, usize> {
    auto descriptions = lexer::get_pattern_descriptions();
    auto const it = std::ranges::find_if(descriptions, [](auto const& description) {
        return description.token_type == token_type;
    });

    auto sequence_parser = lexer::SequenceParser{ std::move(it->pattern) };
    auto pattern = sequence_parser.parse();
    auto const num_states = pattern.states.size();
    return { std::move(pattern), num_states };
}

template<lexer::TokenType token_type>
[[nodiscard]] consteval auto should_emit() -> bool {
    auto descriptions = lexer::get_pattern_descriptions();
    auto const it = std::ranges::find_if(descriptions, [](auto const& description) {
        return description.token_type == token_type;
    });
    return it->should_emit;
}

template<lexer::TokenType token_type>
[[nodiscard]] consteval auto get_num_states_of_pattern() -> usize {
    static constexpr auto num_states = get<1>(get_pattern<token_type>());
    return num_states;
}
template<lexer::TokenType const token_type, usize const state_index>
[[nodiscard]] consteval auto get_transitions() -> std::span<lexer::Transition const> {
    static constexpr auto transitions = std::define_static_array(
        get<0>(get_pattern<token_type>()).states.at(state_index).state.transitions
    );
    return transitions;
}

template<lexer::TokenType const token_type, usize const state_index>
[[nodiscard]] consteval auto get_transitions_array() -> auto {
    static constexpr auto transitions = std::define_static_array(
        get_transitions<token_type, state_index>()
    );
    auto result = std::array<lexer::Transition, transitions.size()>{};
    std::ranges::copy(transitions, result.begin());
    return result;
}

template<lexer::TokenType const token_type, usize const state_index>
[[nodiscard]] consteval auto is_state_final() -> bool {
    return get<0>(get_pattern<token_type>()).states.at(state_index).type == lexer::StateType::Final;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::println(stderr, "Usage: {} <output_file>", argv[0]);
        return EXIT_FAILURE;
    }
    auto const file = open_file(argv[1]);
    if (!file) {
        std::println(stderr, "Error opening file: {}", argv[1]);
        return EXIT_FAILURE;
    }
    std::println(file.get(), R"(#include <array>
#include <cstdlib>
#include <span>

namespace lexer {{

    using usize = std::size_t;

    struct CharMask final {{
        std::array<bool, 128> mask{{}};

        [[nodiscard]] explicit consteval CharMask(std::same_as<char> auto const... elements) {{
            template for (auto const c : {{ elements... }}) {{
                mask.at(static_cast<usize>(c)) = true;
            }}
        }}

        [[nodiscard]] constexpr auto contains(char const c) const -> bool {{
            return mask.at(static_cast<usize>(c));
        }}

        [[nodiscard]] constexpr auto size() const noexcept -> usize {{
            return mask.size();
        }}
    }};

    struct Transition final {{
        CharMask char_mask{{}};
        usize next_state{{}};
    }};

    struct State final {{
        std::span<Transition const> transitions{{}};
        bool is_final{{}};
    }};

    struct Pattern final {{
        std::span<State const> states{{}};
        bool should_emit{{}};
    }};

    namespace detail {{
)");
    template for (constexpr auto token_type : std::define_static_array(enumerators_of(^^lexer::TokenType))) {
        static constexpr auto num_states = get_num_states_of_pattern<([: token_type :])>();
        template for (constexpr auto state_index : std::views::iota(0uz, num_states)) {
            static constexpr auto transitions = get_transitions_array<([: token_type :]), state_index>();
            std::println(file.get(),
                "        inline constexpr auto transitions_{}_state{} = std::array<Transition, {}>{{",
                display_string_of(token_type),
                state_index,
                transitions.size()
            );
            for (auto const& transition : transitions) {
                std::print(file.get(), "            Transition{{ CharMask{{ ");
                auto const& char_mask = transition.char_mask;
                for (auto const i : std::views::iota(0uz, char_mask.size())) {
                    if (char_mask.contains(static_cast<char>(i))) {
                        if (i == '\\') {
                            std::print(file.get(), "'\\\\', ");
                        } else if (i == '\f') {
                            std::print(file.get(), "'\\f', ");
                        } else if (i == '\n') {
                            std::print(file.get(), "'\\n', ");
                        } else if (i == '\r') {
                            std::print(file.get(), "'\\r', ");
                        } else if (i == '\t') {
                            std::print(file.get(), "'\\t', ");
                        } else if (i == '\'') {
                            std::print(file.get(), "'\\\'', ");
                        } else if (i >= 32 && i <= 126) {
                            std::print(file.get(), "'{}', ", static_cast<char>(i));
                        } else {
                            std::print(file.get(), "static_cast<char>({}), ", i);
                        }
                    }
                }
                std::println(file.get(), "}}, {}uz }},", transition.next_state);
            }
            std::println(file.get(), "        }};");
        }
        std::println(file.get(), "");
    }
    template for (constexpr auto token_type : std::define_static_array(enumerators_of(^^lexer::TokenType))) {
        std::println(file.get(), "        inline constexpr auto states_{} = std::array{{", display_string_of(token_type));
        static constexpr auto num_states = get_num_states_of_pattern<([: token_type :])>();
        template for (constexpr auto state_index : std::views::iota(0uz, num_states)) {
            static constexpr auto is_final = is_state_final<([: token_type :]), state_index>();
            std::println(file.get(),
                "            State{{ std::span<Transition const>{{ transitions_{}_state{} }}, {} }},",
                display_string_of(token_type),
                state_index,
                is_final ? "true" : "false"
            );
        }
        std::println(file.get(), "        }};");
        std::println(file.get(), "");
    }

    template for (constexpr auto token_type : std::define_static_array(enumerators_of(^^lexer::TokenType))) {
        std::println(file.get(), "        inline constexpr auto pattern_{} = Pattern{{", display_string_of(token_type));
        std::println(file.get(), "            std::span<State const>{{ states_{} }},", display_string_of(token_type));
        std::println(file.get(), "            {},", should_emit<([: token_type :])>() ? "true" : "false");
        std::println(file.get(), "        }};");
        std::println(file.get(), "");
    }
    std::println(file.get(), "    }} // namespace detail");
    std::println(file.get(), "");
    std::println(file.get(), "    inline constexpr auto patterns = std::array{{");
    template for (constexpr auto token_type : std::define_static_array(enumerators_of(^^lexer::TokenType))) {
        std::println(file.get(), "        detail::pattern_{},", display_string_of(token_type));
    }
    std::println(file.get(), "    }};");
    std::println(file.get(), "}} // namespace lexer");
}
