#pragma once

#include <utils/types.hpp>
#include <array>
#include <ranges>

namespace lexer {

    struct CharMask final {
        std::array<bool, 128> mask{};

        [[nodiscard]] explicit consteval CharMask() = default;

        [[nodiscard]] explicit consteval CharMask(decltype(mask) const& mask) : mask{ mask } { }

        [[nodiscard]] constexpr auto contains(char const c) const -> bool {
            return mask.at(static_cast<usize>(c));
        }

        consteval auto set(char const c) -> void {
            mask.at(static_cast<usize>(c)) = true;
        }

        [[nodiscard]] constexpr auto size() const noexcept -> usize {
            return mask.size();
        }

        [[nodiscard]] consteval auto operator|(CharMask const other) const -> CharMask {
            auto result = CharMask{};
            for (auto const i : std::views::iota(0uz, mask.size())) {
                auto const c = static_cast<char>(i);
                if (contains(c) or other.contains(c)) {
                    result.set(c);
                }
            }
            return result;
        }

        constexpr auto operator==(const CharMask&) const noexcept -> bool = default;

        friend constexpr auto operator<=>(const CharMask& lhs, const CharMask& rhs) -> auto = default;

        [[nodiscard]] constexpr auto to_string() const -> std::string {
            auto result = std::string{};
            for (auto c = static_cast<char>(32); c <= 126; ++c) {
                if (contains(c)) {
                    result += c;
                }
            }
            return result;
        }
    };

}
