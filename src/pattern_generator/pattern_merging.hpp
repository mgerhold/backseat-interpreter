#pragma once

#include <algorithm>
#include <vector>
#include "pattern.hpp"

namespace lexer {

    consteval auto deduplicate_transitions(std::vector<State>& states) -> void {
        for (auto& [transitions] : states) {
            // Merge duplicated transitions.
            std::ranges::sort(transitions);
            auto const new_end = std::ranges::unique(transitions).begin();
            transitions.erase(new_end, transitions.end());
        }
    }

    [[nodiscard]] consteval auto annotate_states(
        std::vector<State>&& states,
        std::span<usize const> final_state_indices
    ) -> std::vector<AnnotatedState> {
        auto final_states = std::vector<AnnotatedState>{};
        for (auto const i : std::views::iota(0uz, states.size())) {
            auto const state_type = (std::ranges::find(final_state_indices, i) != final_state_indices.end())
                ? StateType::Final
                : StateType::Normal;
            final_states.emplace_back(std::move(states.at(i)), state_type);
        }
        return final_states;
    }

    consteval auto deduplicate_states(std::vector<AnnotatedState>& states) -> void {
        auto did_deduplicate = false;
        do {
            did_deduplicate = false;
            for (auto const i : std::views::iota(0uz, states.size() - 1uz)) {
                auto const& first = states.at(i);
                for (auto const j : std::views::iota(i + 1uz, states.size())) {
                    auto const& second = states.at(j);
                    auto const any_is_final = (first.type == StateType::Final or second.type == StateType::Final);
                    if (first.state != second.state) {
                        continue;
                    }
                    auto const removed_index = i;
                    states.erase(states.begin() + static_cast<std::ptrdiff_t>(removed_index));
                    auto const replacement_index = j - 1uz;
                    if (any_is_final) {
                        states.at(replacement_index).type = StateType::Final;
                    }
                    for (auto& state : states) {
                        for (auto& transition : state.state.transitions) {
                            if (transition.next_state == removed_index) {
                                transition.next_state = replacement_index;
                            } else if (transition.next_state > removed_index) {
                                --transition.next_state; // Adjust for the removed state.
                            }
                        }
                    }
                    did_deduplicate = true;
                    break;
                }
                if (did_deduplicate) {
                    break;
                }
            }
        } while (did_deduplicate);
    }

} // namespace lexer
