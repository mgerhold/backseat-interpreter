#pragma once

#include "regex.hpp"
#include <algorithm>
#include <vector>
#include <print>
#include <limits>
#include <span>
#include <ranges>
#include <unordered_map>

namespace lexer {
    [[nodiscard]] consteval auto get_pattern_descriptions() -> auto {
        // TODO: Add TokenType to each pattern and meta information (e.g. whether the token should be emitted).
        return std::array{
            Sequence{ "function" },
            Sequence{ "return" },
            Sequence{ "Function" },
            // Just a test.
            Sequence{
                RegexElement{
                    Maybe{
                        RegexElement{
                            RegexElement{ Char{ 'a' } },
                        },
                    },
                },
                RegexElement{ Char{ 'b' } },
                RegexElement{
                    Maybe{
                        RegexElement{
                            RegexElement{ Char{ '!' } },
                        },
                    },
                },
                RegexElement{ Char{ 'X' } },
            },
            // Identifiers.
            Sequence{
                RegexElement{
                    CharSet{
                        Range{ 'a', 'z' },
                        Char{ '_' },
                    },
                },
                RegexElement{
                    ZeroOrMoreOf{
                        RegexElement{
                            CharSet{
                                Range{ 'a', 'z' },
                                Range{ 'A', 'Z' },
                                Range{ '0', '9' },
                                Char{ '_' },
                            },
                        },
                    },
                },
            },
            // Unsigned Integers.
            Sequence{
                RegexElement{
                    CharSet{
                        Range{ '0', '9' },
                    },
                },
                RegexElement{
                    ZeroOrMoreOf{
                        RegexElement{
                            Sequence{
                                RegexElement{ Maybe{ RegexElement{ Char{ '\'' } } } },
                                RegexElement{
                                    CharSet{
                                        Range{ '0', '9' },
                                    },
                                },
                            },
                        },
                    },
                },
                RegexElement{ Char{ '_' } },
                RegexElement{ Char{ 'u' } },
                RegexElement{ Char{ '6' } },
                RegexElement{ Char{ '4' } },
            },
        };
    }


    struct Transition final {
        CharMask char_mask{};
        usize next_state{};
    };

    struct State final {
        static constexpr auto final = std::numeric_limits<usize>::max();

        std::vector<Transition> transitions;
    };

    struct Pattern final {
        std::vector<State> states;
    };

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
            auto _ = parse(m_sequence, 0uz);
            return Pattern{
                .states = std::move(m_states),
            };
        }

        using ValidSubMatches = std::vector<usize>;

        [[nodiscard]] consteval auto parse(
            RegexElement const& element,
            usize const current_state
        ) -> ValidSubMatches {
            return std::visit(
                utils::Overloaded{
                    [&](Char const& char_) {
                        auto const transitions = get_possible_transitions(element);
                        if (transitions.size() != 1) {
                            throw std::runtime_error{ "Expected exactly one transition for Char element." };
                        }
                        auto const transition = transitions.at(0);
                        if (transition.next_element != std::addressof(element)) {
                            throw std::runtime_error{ "Transition does not point to the expected element." };
                        }
                        auto const state_index = get_index(element);
                        m_states.at(current_state).transitions.emplace_back(
                            char_.get_char_mask(),
                            state_index
                        );
                        return ValidSubMatches{ state_index };
                    },
                    [&](CharSet const& char_set) {
                        auto const transitions = get_possible_transitions(element);
                        if (transitions.size() != 1) {
                            throw std::runtime_error{ "Expected exactly one transition for Char element." };
                        }
                        auto const transition = transitions.at(0);
                        if (transition.next_element != std::addressof(element)) {
                            throw std::runtime_error{ "Transition does not point to the expected element." };
                        }
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
                        auto sub_matches = ValidSubMatches{};
                        for (auto const&[char_mask, next_element] : inner_transitions) {
                            if (next_element == std::addressof(element)) {
                                throw std::runtime_error{ "Transition for Maybe element points to itself." };
                            }
                            auto const new_sub_matches = parse(*maybe.element, current_state);
                            std::ranges::copy(new_sub_matches, std::back_inserter(sub_matches));
                        }

                        if (not successor_transitions.has_value()) {
                            return sub_matches;
                        }

                        for (auto const&[char_mask, next_element] : successor_transitions.value()) {
                            auto const next_element_index = get_index(*next_element);
                            // for (auto const sub_match : sub_matches) {
                            //     m_states.at(sub_match).transitions.emplace_back(
                            //         char_mask,
                            //         next_element_index
                            //     );
                            // }
                            m_states.at(current_state).transitions.emplace_back(char_mask, next_element_index);
                        }
                        return sub_matches;
                    },
                    [&](Sequence const& sequence) {
                        return parse(sequence, current_state);
                    },
                    [&](auto const&) { return ValidSubMatches{}; },
                },
                element.element
            );
        }

        [[nodiscard]] consteval auto parse(
            Sequence const& sequence,
            usize current_state
        ) -> ValidSubMatches {
            auto sub_matches = ValidSubMatches{ current_state };
            for (auto const& element : sequence.elements) {
                for (auto const sub_match : sub_matches) {
                    sub_matches = parse(element, sub_match);
                }
            }
            return sub_matches;
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
                throw std::runtime_error{
                    "RegexElement not found in state indices."
                };
            }
            return std::get<1>(*index);
        }

        [[nodiscard]] consteval auto element_of_state(usize const state_index) const -> RegexElement const& {
            auto const element = std::ranges::find_if(
                m_state_indices,
                [&](auto const& tuple) {
                    return std::get<1>(tuple) == state_index;
                }
            );
            if (element == m_state_indices.end()) {
                throw std::runtime_error{
                    "State index not found in state indices."
                };
            }
            return *std::get<0>(*element);
        }

        [[nodiscard]] consteval auto determine_state_indices(
            RegexElement const& element,
            usize current_index
        ) -> usize {
            std::visit(
                utils::Overloaded{
                    [&](Char const&) {
                        m_state_indices.emplace_back(std::addressof(element), current_index++);
                    },
                    [&](CharSet const&) {
                        m_state_indices.emplace_back(std::addressof(element), current_index++);
                    },
                    [&](ZeroOrMoreOf const& zero_or_more_of) {
                        current_index = determine_state_indices(*zero_or_more_of.element, current_index);
                    },
                    [&](Maybe const& maybe) {
                        current_index = determine_state_indices(*maybe.element, current_index);
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
                    [&](Sequence const& sequence) {
                        determine_successors(sequence);
                    },
                    [&](Maybe const& maybe) {
                        determine_successors(*maybe.element);
                    },
                    [](Char const&) {},
                    [](CharSet const&) {},
                },
                element.element
            );
        }


        consteval auto determine_successors(Sequence const& sequence) -> void {
            for (auto const i : std::views::iota(0uz, sequence.elements.size() - 1)) {
                set_successor(sequence.elements.at(i), sequence.elements.at(i + 1));
            }
            for (auto const& element : sequence.elements) {
                determine_successors(element);
            }
        }

        [[nodiscard]] consteval auto find_successor(RegexElement const& element) const -> RegexElement const* {
            auto const successor = std::ranges::find_if(
                m_successors,
                [&](auto const& tuple) {
                    return std::get<0>(tuple) == std::addressof(element);
                }
            );
            if (successor == m_successors.end()) {
                return nullptr;
            }
            return std::get<1>(*successor);
        }

        [[nodiscard]] consteval auto get_possible_transitions(RegexElement const& element) -> std::vector<RegexElementTransition> {
            return std::visit(
                utils::Overloaded{
                    [&](Char const& char_) {
                        return std::vector{
                            RegexElementTransition{
                                .char_mask = char_.get_char_mask(),
                                .next_element = std::addressof(element),
                            }
                        };
                    },
                    [&](CharSet const& char_set) {
                        return std::vector{
                            RegexElementTransition{
                                .char_mask = char_set.mask,
                                .next_element = std::addressof(element),
                            }
                        };
                    },
                    [&](Sequence const& sequence) {
                        return get_possible_transitions(sequence.elements.at(0));
                    },
                    [&](Maybe const& maybe) {
                        auto transitions = get_possible_transitions(*maybe.element);
                        auto const successor = find_successor(element);
                        if (successor == nullptr) {
                            return transitions;
                        }
                        auto successor_transitions = get_possible_transitions(*successor);
                        std::ranges::move(std::move(successor_transitions), std::back_inserter(transitions));
                        return transitions;
                    },
                    [&](ZeroOrMoreOf const& zero_or_more_of) {
                        auto transitions = get_possible_transitions(*zero_or_more_of.element);
                        auto const successor = find_successor(element);
                        if (successor == nullptr) {
                            return transitions;
                        }
                        auto successor_transitions = get_possible_transitions(*successor);
                        std::ranges::move(std::move(successor_transitions), std::back_inserter(transitions));
                        return transitions;
                    },
                },
                element.element
            );
        }
    };


} // namespace lexer
