#pragma once

#include <type_traits>
#include <string_view>
#include "types.hpp"
#include <experimental/meta>

namespace utils {
    [[nodiscard]] auto constexpr enum_to_string(auto const value) -> std::string_view
        requires std::is_enum_v<decltype(value)> {
        using EnumType = std::remove_cvref_t<decltype(value)>;
        auto const _ = EnumType{}; // To silence compiler warning.

        static constexpr auto token_names = [] {
            auto enumerators = enumerators_of(dealias(^^EnumType));
            auto names = std::vector<char const*>{};
            names.reserve(enumerators.size());
            for (auto const token_type : enumerators) {
                names.push_back(std::define_static_string(display_string_of(token_type)));
            }
            return std::define_static_array(names);
        }();

        return std::string_view{ token_names.at(static_cast<usize>(value)) };
    }
}
