#pragma once

#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>
#include <ranges>
#include <variant>
#include <algorithm>
#include "regex.hpp"
#include "pattern.hpp"
#include "pattern_merging.hpp"
#include <utils/concepts.hpp>
#include <utils/types.hpp>
#include <utils/utils.hpp>

namespace lexer {

    class SequenceParser final {
    // private:
    public:
        Sequence m_sequence;
        std::vector<std::tuple<RegexElement const*, RegexElement const*>> m_successors;
        std::vector<std::tuple<RegexElement const*, usize>> m_state_indices;
        std::vector<State> m_states;

        struct RegexElementTransition final {
            CharMask char_mask{};
            RegexElement const* next_element{};
        };

    public:
        [[nodiscard]] explicit consteval SequenceParser(Sequence sequence)
            : m_sequence{ std::move(sequence) } {
            determine_successors(m_sequence);
            auto _ = determine_state_indices(m_sequence, 1uz);

            m_states.resize(m_state_indices.size() + 1);
        }

        SequenceParser(SequenceParser const& other) = delete;
        SequenceParser(SequenceParser&& other) noexcept = delete;
        SequenceParser& operator=(SequenceParser const& other) = delete;
        SequenceParser& operator=(SequenceParser&& other) noexcept = delete;
        ~SequenceParser() = default;

        [[nodiscard]] consteval auto parse() -> Pattern {
            auto const final_state_indices = parse(RegexElement{ std::move(m_sequence) }, 0uz);
            deduplicate_transitions(m_states);
            auto annotated_states = annotate_states(
                std::move(m_states),
                final_state_indices
            );
            deduplicate_states(annotated_states);
            return Pattern{ std::move(annotated_states) };
        }

        using ValidSubMatches = std::vector<usize>;

        [[nodiscard]] consteval auto parse(
            RegexElement const& element,
            usize const current_state
        ) -> ValidSubMatches {
            return std::visit(
                utils::Overloaded{
                    [&](EitherOf const& either_of) {
                        auto sub_matches = ValidSubMatches{};
                        for (auto const i : std::views::iota(0uz, either_of.elements.size())) {
                            auto const& sub_element = either_of.elements.at(i);
                            std::ranges::copy(
                                parse(sub_element, current_state),
                                std::back_inserter(sub_matches)
                            );
                        }
                        return sub_matches;
                    },
                    [&](Sequence const& sequence) {
                        auto last_sub_matches = ValidSubMatches{ current_state };
                        for (auto const i : std::views::iota(0uz, sequence.elements.size())) {
                            auto const& sub_element = sequence.elements.at(i);
                            auto current_sub_matches = ValidSubMatches{};
                            for (auto const sub_match : last_sub_matches) {
                                std::ranges::copy(
                                    parse(sub_element, sub_match),
                                    std::back_inserter(current_sub_matches)
                                );
                            }
                            last_sub_matches = std::move(current_sub_matches);
                        }
                        return last_sub_matches;
                    },
                    [&](CharSet const& char_set) {
                        auto const state_index = get_index(element);
                        m_states.at(current_state).transitions.emplace_back(
                            char_set.mask,
                            state_index
                        );
                        return ValidSubMatches{ state_index };
                    },
                    [&](Maybe const& maybe) {
                        auto const successor = find_successor(element);
                        auto const successor_transitions = [&]() -> std::optional<std::vector<RegexElementTransition>> {
                            if (successor == nullptr) {
                                return std::nullopt;
                            }
                            return get_possible_transitions(*successor);
                        }();

                        auto const inner_transitions = get_possible_transitions(*maybe.element);
                        auto const sub_matches = parse(*maybe.element, current_state);

                        if (not successor_transitions.has_value()) {
                            return sub_matches;
                        }

                        for (auto const&[char_mask, next_element] : successor_transitions.value()) {
                            m_states.at(current_state).transitions.emplace_back(char_mask, get_index(*next_element));
                        }
                        return sub_matches;
                    },
                    [&](ZeroOrMoreOf const& zero_or_more_of) {
                        auto const successor = find_successor(element);
                        auto const successor_transitions = [&]() -> std::optional<std::vector<RegexElementTransition>> {
                            if (successor == nullptr) {
                                return std::nullopt;
                            }
                            return get_possible_transitions(*successor);
                        }();

                        auto const inner_transitions = get_possible_transitions(*zero_or_more_of.element);
                        auto const sub_matches = parse(*zero_or_more_of.element, current_state);

                        for (auto const sub_match : sub_matches) {
                            auto const _ = parse(*zero_or_more_of.element, sub_match);
                        }

                        if (not successor_transitions.has_value()) {
                            return sub_matches;
                        }

                        for (auto const&[char_mask, next_element] : successor_transitions.value()) {
                            m_states.at(current_state).transitions.emplace_back(char_mask, get_index(*next_element));
                        }
                        return sub_matches;
                    },
                },
                element.element
            );
        }

    // private:
        consteval auto set_successor(RegexElement const& from, RegexElement const& to) -> void {
            m_successors.emplace_back(std::addressof(from), std::addressof(to));
        }

        [[nodiscard]] consteval auto get_index(RegexElement const& element) const -> usize {
            auto const index = std::ranges::find_if(
                m_state_indices,
                [&](auto const& tuple) {
                    return std::get<0>(tuple) == std::addressof(element);
                }
            );
            if (index == m_state_indices.end()) {
                throw std::runtime_error{ "RegexElement not found in state indices." };
            }
            return std::get<1>(*index);
        }

        [[nodiscard]] consteval auto element_of_state(usize const state_index) const -> RegexElement const& {
            auto const element = std::ranges::find_if(
                m_state_indices,
                [&](auto const& tuple) { return std::get<1>(tuple) == state_index; }
            );
            if (element == m_state_indices.end()) {
                throw std::runtime_error{ "State index not found in state indices." };
            }
            return *std::get<0>(*element);
        }

        [[nodiscard]] consteval auto determine_state_indices(
            RegexElement const& element,
            usize current_index
        ) -> usize {
            std::visit(
                utils::Overloaded{
                    [&](CharSet const&) {
                        m_state_indices.emplace_back(std::addressof(element), current_index++);
                    },
                    [&](ZeroOrMoreOf const& zero_or_more_of) {
                        current_index = determine_state_indices(*zero_or_more_of.element, current_index);
                    },
                    [&](Maybe const& maybe) {
                        current_index = determine_state_indices(*maybe.element, current_index);
                    },
                    [&](EitherOf const& either_of) {
                        for (auto const& sub_element : either_of.elements) {
                            current_index = determine_state_indices(sub_element, current_index);
                        }
                    },
                    [&](Sequence const& sequence) {
                        current_index = determine_state_indices(sequence, current_index);
                    },
                },
                element.element
            );
            return current_index;
        }

        [[nodiscard]] consteval auto determine_state_indices(
            Sequence const& sequence,
            usize current_index
        ) -> usize {
            for (auto const& element : sequence.elements) {
                current_index = determine_state_indices(element, current_index);
            }
            return current_index;
        }

        consteval auto determine_successors(RegexElement const& element) -> void {
            std::visit(
                utils::Overloaded{
                    [&](ZeroOrMoreOf const& zero_or_more_of) {
                        determine_successors(*zero_or_more_of.element);
                    },
                    [&](EitherOf const& either_of) {
                        for (auto const& sub_element : either_of.elements) {
                            determine_successors(sub_element);
                        }
                    },
                    [&](Sequence const& sequence) {
                        determine_successors(sequence);
                    },
                    [&](Maybe const& maybe) {
                        determine_successors(*maybe.element);
                    },
                    [](CharSet const&) {},
                },
                element.element
            );
        }

        consteval auto determine_successors(Sequence const& sequence) -> void {
            // In a sequence, each element's successor is the next element in the sequence.
            for (auto const i : std::views::iota(0uz, sequence.elements.size() - 1)) {
                set_successor(sequence.elements.at(i), sequence.elements.at(i + 1));
            }
            // All elements in the sequence could have internal successor-relations. We have to find them as well.
            for (auto const& element : sequence.elements) {
                determine_successors(element);
            }
        }

        [[nodiscard]] consteval auto find_successor(RegexElement const& element) const -> RegexElement const* {
            auto const successor = std::ranges::find_if(
                m_successors,
                [&](auto const& tuple) { return std::get<0>(tuple) == std::addressof(element); }
            );
            return (successor == m_successors.end()) ? nullptr : std::get<1>(*successor);
        }

        [[nodiscard]] consteval auto get_possible_transitions(RegexElement const& element) -> std::vector<RegexElementTransition> {
            // Helper lambda to avoid code duplication when processing elements that have sub-elements.
            auto const process_sub_element = [this](RegexElement const& element) -> std::vector<RegexElementTransition> {
                auto transitions = get_possible_transitions(element);
                auto const successor = find_successor(element);
                if (successor == nullptr) {
                    return transitions;
                }
                auto successor_transitions = get_possible_transitions(*successor);
                std::ranges::move(std::move(successor_transitions), std::back_inserter(transitions));
                return transitions;
            };

            return std::visit(
                utils::Overloaded{
                    [&](CharSet const& char_set) {
                        return std::vector{
                            RegexElementTransition{
                                .char_mask = char_set.mask,
                                .next_element = std::addressof(element),
                            }
                        };
                    },
                    [&](EitherOf const& either_of) {
                        auto transitions = std::vector<RegexElementTransition>{};
                        for (auto const& sub_element : either_of.elements) {
                            auto sub_transitions = get_possible_transitions(sub_element);
                            std::ranges::move(std::move(sub_transitions), std::back_inserter(transitions));
                        }
                        return transitions;
                    },
                    [&](Sequence const& sequence) {
                        return get_possible_transitions(sequence.elements.at(0));
                    },
                    [&](Maybe const& maybe) {
                        return process_sub_element(*maybe.element);
                    },
                    [&](ZeroOrMoreOf const& zero_or_more_of) {
                        return process_sub_element(*zero_or_more_of.element);
                    },
                },
                element.element
            );
        }
    };
}
