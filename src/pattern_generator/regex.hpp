#pragma once

#include <utils/concepts.hpp>
#include <utils/types.hpp>
#include <utils/utils.hpp>
#include "char_mask.hpp"
#include "regex.hpp"

#include <array>
#include <bitset>
#include <experimental/meta>
#include <memory>
#include <ranges>
#include <tuple>
#include <variant>

namespace lexer {
    struct CharSet;

    struct Range final {
        char start;
        char end;

        [[nodiscard]] explicit consteval Range(char const start, char const end) : start{ start }, end{ end } { }

        [[nodiscard]] consteval auto get_char_mask() const -> CharMask {
            auto result = CharMask{};
            for (auto const c : std::views::iota(start, end + 1)) {
                result.set(c);
            }
            return result;
        }

        [[nodiscard]] consteval auto operator|(Range const& other) const -> CharSet;

        [[nodiscard]] consteval auto operator|(char const other) const -> CharSet;
    };

    struct RegexElement;

    struct CharSet final {
        CharMask mask{};

        [[nodiscard]] consteval CharSet(CharMask const& mask) : mask{ mask } { }

        [[nodiscard]] consteval CharSet(utils::OneOf<char, Range> auto const... elements) {
            template for (auto const element : { elements... }) {
                using Element = std::remove_cvref_t<decltype(element)>;
                if constexpr (std::same_as<Element, char>) {
                    mask.set(element);
                } else if constexpr (std::same_as<Element, Range>) {
                    mask = mask | element.get_char_mask();
                } else {
                    // Should be unreachable.
                    throw std::runtime_error{ "Not implemented." };
                }
            }
        }

        [[nodiscard]] consteval auto operator|(char const c) const -> CharSet {
            auto result = mask;
            result.set(c);
            return CharSet{ result };
        }

        [[nodiscard]] consteval auto operator|(Range const& range) const -> CharSet {
            return CharSet{ mask | range.get_char_mask() };
        }

        [[nodiscard]] consteval auto operator|(CharSet const& other) const -> CharSet {
            return CharSet{ mask | other.mask };
        }

        [[nodiscard]] consteval auto inverse() const -> CharSet {
            auto result = CharMask{};
            for (auto const i : std::views::iota(0uz, mask.size())) {
                auto const c = static_cast<char>(i);
                if (not mask.contains(c)) {
                    result.set(c);
                }
            }
            return CharSet{ result };
        }
    };

    [[nodiscard]] consteval auto Range::operator|(Range const& other) const -> CharSet {
        auto result = get_char_mask();
        for (auto const c : std::views::iota(other.start, other.end + 1)) {
            result.set(c);
        }
        return CharSet{ result };
    }

    [[nodiscard]] consteval auto Range::operator|(char const other) const -> CharSet {
        auto result = get_char_mask();
        result.set(other);
        return CharSet{ result };
    }

    struct ZeroOrMoreOf final {
        std::unique_ptr<RegexElement> element;

        [[nodiscard]] explicit consteval ZeroOrMoreOf(RegexElement element);
    };

    struct Maybe final {
        std::unique_ptr<RegexElement> element;

        [[nodiscard]] explicit consteval Maybe(RegexElement element);
    };

    struct Sequence;

    struct EitherOf final {
        std::vector<RegexElement> elements;

        [[nodiscard]] explicit consteval EitherOf(
                utils::OneOf<RegexElement, CharSet, Maybe, Range, char, ZeroOrMoreOf, Sequence> auto&&... elements
        );
    };

    struct Sequence final {
        std::vector<RegexElement> elements;

        [[nodiscard]] explicit consteval Sequence(
                utils::OneOf<RegexElement, CharSet, Maybe, Range, char, ZeroOrMoreOf, EitherOf> auto&&... elements
        );

        [[nodiscard]] explicit consteval Sequence(std::string_view const text);
    };

    struct RegexElement final {
        std::variant<CharSet, ZeroOrMoreOf, Maybe, Sequence, EitherOf> element;

        // Implicit contructors to ease construction.
        [[nodiscard]] consteval RegexElement(CharSet const& char_set) : element{ char_set } { }
        [[nodiscard]] consteval RegexElement(ZeroOrMoreOf zero_or_more_of) : element{ std::move(zero_or_more_of) } { }
        [[nodiscard]] consteval RegexElement(Maybe maybe) : element{ std::move(maybe) } { }
        [[nodiscard]] consteval RegexElement(EitherOf either_of) : element{ std::move(either_of) } { }
        [[nodiscard]] consteval RegexElement(Sequence sequence) : element{ std::move(sequence) } { }
        [[nodiscard]] consteval RegexElement(char const c) : element{ CharSet{ c } } { }
        [[nodiscard]] consteval RegexElement(Range const& range) : element{ CharSet{ range } } { }
    };

    [[nodiscard]] consteval ZeroOrMoreOf::ZeroOrMoreOf(RegexElement element)
        : element{ std::make_unique<RegexElement>(std::move(element)) } { }

    [[nodiscard]] consteval EitherOf::EitherOf(
            utils::OneOf<RegexElement, CharSet, Maybe, Range, char, ZeroOrMoreOf, Sequence> auto&&... elements
    ) {
        template for (auto&& element : { elements... }) {
            this->elements.push_back(static_cast<RegexElement>(std::move(element)));
        }
    }

    [[nodiscard]] consteval Sequence::Sequence(
            utils::OneOf<RegexElement, CharSet, Maybe, Range, char, ZeroOrMoreOf, EitherOf> auto&&... elements
    ) {
        template for (auto&& element : { elements... }) {
            this->elements.push_back(static_cast<RegexElement>(std::move(element)));
        }
    }

    [[nodiscard]] consteval Sequence::Sequence(std::string_view const text) {
        for (auto const c : text) {
            elements.push_back(RegexElement{ CharSet{ c } });
        }
    }

    [[nodiscard]] consteval Maybe::Maybe(RegexElement element)
        : element{ std::make_unique<RegexElement>(std::move(element)) } { }
} // namespace lexer
