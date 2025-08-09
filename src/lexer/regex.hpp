#pragma once

#include "../utils/concepts.hpp"
#include "../utils/types.hpp"
#include "../utils/utils.hpp"
#include "regex.hpp"

#include <array>
#include <bitset>
#include <experimental/meta>
#include <ranges>
#include <tuple>
#include <variant>
#include <memory>

namespace lexer {
    struct CharMask final {
        std::bitset<128> mask;

        [[nodiscard]] explicit consteval CharMask() = default;

        [[nodiscard]] explicit consteval CharMask(decltype(mask) const& mask) : mask{ mask } { }

        [[nodiscard]] consteval auto contains(char const c) const -> bool {
            return mask.test(static_cast<usize>(c));
        }

        consteval auto set(char const c) -> void {
            mask.set(static_cast<usize>(c));
        }

        [[nodiscard]] consteval auto operator|(CharMask const other) const -> CharMask {
            auto result = decltype(mask){};
            for (auto i = 0uz; i < mask.size(); ++i) {
                result.set(i, mask.test(i) or other.mask.test(i));
            }
            return CharMask{ result };
        }
    };

    struct Range final {
        char start;
        char end;

        [[nodiscard]] explicit consteval Range(char const start, char const end) : start{ start }, end{ end } { }

        [[nodiscard]] consteval auto get_char_mask() const -> CharMask {
            auto result = CharMask{};
            for (auto c = start; c <= end; ++c) {
                result.set(c);
            }
            return result;
        }
    };

    struct Char final {
        char value{};

        [[nodiscard]] consteval Char() = default;

        [[nodiscard]] explicit consteval Char(char const value) : value{ value } { }

        [[nodiscard]] consteval auto get_char_mask() const -> CharMask {
            auto result = CharMask{};
            result.set(value);
            return result;
        }
    };

    struct CharSet final {
        CharMask mask{};

        [[nodiscard]] consteval CharSet(utils::OneOf<Char, Range> auto const... elements) {
            template for (auto const element : { elements... }) {
                mask = mask | element.get_char_mask();
            }
        }
    };

    struct RegexElement;

    struct ZeroOrMoreOf final {
        std::unique_ptr<RegexElement> element;

        [[nodiscard]] explicit consteval ZeroOrMoreOf(RegexElement element);
    };

    struct Sequence final {
        std::vector<RegexElement> elements;

        [[nodiscard]] explicit consteval Sequence(std::same_as<RegexElement> auto&&... elements);

        [[nodiscard]] explicit consteval Sequence(std::string_view const text);
    };

    struct Maybe final {
        std::unique_ptr<RegexElement> element;

        [[nodiscard]] explicit consteval Maybe(RegexElement element);
    };

    struct RegexElement final {
        std::variant<Char, CharSet, ZeroOrMoreOf, Maybe, Sequence> element;
    };

    [[nodiscard]] consteval ZeroOrMoreOf::ZeroOrMoreOf(RegexElement element)
        : element{ std::make_unique<RegexElement>(std::move(element)) } { }

    [[nodiscard]] consteval Sequence::Sequence(std::same_as<RegexElement> auto&&... elements) {
        template for (auto&& element : { elements... }) {
            this->elements.push_back(std::move(element));
        }
    }

    [[nodiscard]] consteval Sequence::Sequence(std::string_view const text) {
        for (auto const c : text) {
            elements.push_back(RegexElement{ Char{ c } });
        }
    }

    [[nodiscard]] consteval Maybe::Maybe(RegexElement element)
        : element{ std::make_unique<RegexElement>(std::move(element)) } { }
} // namespace lexer
