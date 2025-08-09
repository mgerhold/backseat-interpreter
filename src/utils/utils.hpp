#pragma once

#include <array>
#include <algorithm>
#include <string_view>
#include <experimental/meta>
#include "types.hpp"

namespace utils {
    template<typename... Ts>
    struct Overloaded final : Ts... {
        using Ts::operator()...;
    };

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

        template<char c>
        [[nodiscard]] consteval auto extend() const -> StaticString<n + 1> {
            static constexpr auto next_state = [&]() -> decltype(auto) {
                constexpr auto s = std::string{ view() } + c;
                static constexpr auto array = std::define_static_array(s);
                return array;
            }();
            static constexpr auto next_state_view = std::string_view{
                next_state.begin(),
                next_state.end(),
            };
            return [: std::meta::reflect_constant_string(next_state_view) :];
        }
    };
}
