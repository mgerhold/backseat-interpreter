#pragma once

#include <array>
#include <algorithm>
#include <string_view>
#include "types.hpp"

namespace utils {
    template<usize n>
    struct StaticString final {
        std::array<char, n> buffer;

        constexpr StaticString() noexcept = default;

        constexpr StaticString(char const (&string)[n]) noexcept {
            std::ranges::copy_n(string, n, buffer.begin());
        }

        constexpr auto view() const -> std::string_view {
            return std::string_view{ buffer.data(), n - 1 }; // Exclude null terminator
        }
    };
}
