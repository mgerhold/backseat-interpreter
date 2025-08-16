#include <array>
#include <cstdlib>
#include <span>

namespace lexer {

    using usize = std::size_t;

    struct CharMask final {
        std::array<bool, 128> mask{};

        [[nodiscard]] explicit consteval CharMask(std::same_as<char> auto const... elements) {
            template for (auto const c : { elements... }) {
                mask.at(static_cast<usize>(c)) = true;
            }
        }

        [[nodiscard]] constexpr auto contains(char const c) const -> bool {
            return mask.at(static_cast<usize>(c));
        }

        [[nodiscard]] constexpr auto size() const noexcept -> usize {
            return mask.size();
        }
    };

    struct Transition final {
        CharMask char_mask{};
        usize next_state{};
    };

    struct State final {
        std::span<Transition const> transitions{};
        bool is_final{};
    };

    struct Pattern final {
        std::span<State const> states{};
        bool should_emit{};
    };

    namespace detail {

        static constexpr auto transitions_LowercaseIdentifier_state0 = std::array<Transition, 2>{
            Transition{ CharMask{ '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', }, 1uz },
        };
        static constexpr auto transitions_LowercaseIdentifier_state1 = std::array<Transition, 2>{
            Transition{ CharMask{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '_', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', }, 1uz },
        };

        static constexpr auto transitions_LowercaseFunction_state0 = std::array<Transition, 9>{
            Transition{ CharMask{ 'f', }, 1uz },
        };
        static constexpr auto transitions_LowercaseFunction_state1 = std::array<Transition, 9>{
            Transition{ CharMask{ 'u', }, 2uz },
        };
        static constexpr auto transitions_LowercaseFunction_state2 = std::array<Transition, 9>{
            Transition{ CharMask{ 'n', }, 3uz },
        };
        static constexpr auto transitions_LowercaseFunction_state3 = std::array<Transition, 9>{
            Transition{ CharMask{ 'c', }, 4uz },
        };
        static constexpr auto transitions_LowercaseFunction_state4 = std::array<Transition, 9>{
            Transition{ CharMask{ 't', }, 5uz },
        };
        static constexpr auto transitions_LowercaseFunction_state5 = std::array<Transition, 9>{
            Transition{ CharMask{ 'i', }, 6uz },
        };
        static constexpr auto transitions_LowercaseFunction_state6 = std::array<Transition, 9>{
            Transition{ CharMask{ 'o', }, 7uz },
        };
        static constexpr auto transitions_LowercaseFunction_state7 = std::array<Transition, 9>{
            Transition{ CharMask{ 'n', }, 8uz },
        };
        static constexpr auto transitions_LowercaseFunction_state8 = std::array<Transition, 9>{
        };

        static constexpr auto transitions_UnsignedIntegerLiteral_state0 = std::array<Transition, 6>{
            Transition{ CharMask{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', }, 1uz },
        };
        static constexpr auto transitions_UnsignedIntegerLiteral_state1 = std::array<Transition, 6>{
            Transition{ CharMask{ '_', }, 2uz },
            Transition{ CharMask{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', }, 1uz },
            Transition{ CharMask{ '\'', }, 0uz },
        };
        static constexpr auto transitions_UnsignedIntegerLiteral_state2 = std::array<Transition, 6>{
            Transition{ CharMask{ 'u', }, 3uz },
        };
        static constexpr auto transitions_UnsignedIntegerLiteral_state3 = std::array<Transition, 6>{
            Transition{ CharMask{ '6', }, 4uz },
        };
        static constexpr auto transitions_UnsignedIntegerLiteral_state4 = std::array<Transition, 6>{
            Transition{ CharMask{ '4', }, 5uz },
        };
        static constexpr auto transitions_UnsignedIntegerLiteral_state5 = std::array<Transition, 6>{
        };

        static constexpr auto transitions_StringLiteral_state0 = std::array<Transition, 4>{
            Transition{ CharMask{ '"', }, 2uz },
        };
        static constexpr auto transitions_StringLiteral_state1 = std::array<Transition, 4>{
            Transition{ CharMask{ '"', '\\', 'f', 'n', 'r', 't', }, 2uz },
        };
        static constexpr auto transitions_StringLiteral_state2 = std::array<Transition, 4>{
            Transition{ CharMask{ '\\', }, 1uz },
            Transition{ CharMask{ '"', }, 3uz },
            Transition{ CharMask{ static_cast<char>(0), static_cast<char>(1), static_cast<char>(2), static_cast<char>(3), static_cast<char>(4), static_cast<char>(5), static_cast<char>(6), static_cast<char>(7), static_cast<char>(8), static_cast<char>(11), static_cast<char>(14), static_cast<char>(15), static_cast<char>(16), static_cast<char>(17), static_cast<char>(18), static_cast<char>(19), static_cast<char>(20), static_cast<char>(21), static_cast<char>(22), static_cast<char>(23), static_cast<char>(24), static_cast<char>(25), static_cast<char>(26), static_cast<char>(27), static_cast<char>(28), static_cast<char>(29), static_cast<char>(30), static_cast<char>(31), ' ', '!', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', static_cast<char>(127), }, 2uz },
        };
        static constexpr auto transitions_StringLiteral_state3 = std::array<Transition, 4>{
        };

        static constexpr auto states_LowercaseIdentifier = std::array{
            State{ std::span<Transition const>{ transitions_LowercaseIdentifier_state0 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseIdentifier_state1 }, true },
        };

        static constexpr auto states_LowercaseFunction = std::array{
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state0 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state1 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state2 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state3 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state4 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state5 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state6 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state7 }, false },
            State{ std::span<Transition const>{ transitions_LowercaseFunction_state8 }, true },
        };

        static constexpr auto states_UnsignedIntegerLiteral = std::array{
            State{ std::span<Transition const>{ transitions_UnsignedIntegerLiteral_state0 }, false },
            State{ std::span<Transition const>{ transitions_UnsignedIntegerLiteral_state1 }, false },
            State{ std::span<Transition const>{ transitions_UnsignedIntegerLiteral_state2 }, false },
            State{ std::span<Transition const>{ transitions_UnsignedIntegerLiteral_state3 }, false },
            State{ std::span<Transition const>{ transitions_UnsignedIntegerLiteral_state4 }, false },
            State{ std::span<Transition const>{ transitions_UnsignedIntegerLiteral_state5 }, true },
        };

        static constexpr auto states_StringLiteral = std::array{
            State{ std::span<Transition const>{ transitions_StringLiteral_state0 }, false },
            State{ std::span<Transition const>{ transitions_StringLiteral_state1 }, false },
            State{ std::span<Transition const>{ transitions_StringLiteral_state2 }, false },
            State{ std::span<Transition const>{ transitions_StringLiteral_state3 }, true },
        };

        static constexpr auto pattern_LowercaseIdentifier = Pattern{
            std::span<State const>{ states_LowercaseIdentifier },
            true,
        };

        static constexpr auto pattern_LowercaseFunction = Pattern{
            std::span<State const>{ states_LowercaseFunction },
            true,
        };

        static constexpr auto pattern_UnsignedIntegerLiteral = Pattern{
            std::span<State const>{ states_UnsignedIntegerLiteral },
            true,
        };

        static constexpr auto pattern_StringLiteral = Pattern{
            std::span<State const>{ states_StringLiteral },
            true,
        };

    } // namespace detail

    static constexpr auto patterns = std::array{
        detail::pattern_LowercaseIdentifier,
        detail::pattern_LowercaseFunction,
        detail::pattern_UnsignedIntegerLiteral,
        detail::pattern_StringLiteral,
    };
} // namespace lexer
