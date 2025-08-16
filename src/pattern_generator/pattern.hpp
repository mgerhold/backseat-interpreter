#pragma once

#include <vector>
#include "char_mask.hpp"

namespace lexer {
    struct Transition final {
        CharMask char_mask{};
        usize next_state{};

        friend constexpr auto operator<=>(const Transition&, const Transition&) = default;

        constexpr auto operator==(const Transition&) const noexcept -> bool = default;
    };

    struct State final {
        std::vector<Transition> transitions;

        [[nodiscard]] constexpr auto operator==(State const& other) const noexcept -> bool = default;
    };

    enum class StateType {
        Normal,
        Final,
    };

    struct AnnotatedState final {
        State state;
        StateType type{};
    };

    struct Pattern final {
        std::vector<AnnotatedState> states;
    };

} // namespace lexer
