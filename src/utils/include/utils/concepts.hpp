#pragma once

#include <concepts>

namespace utils {
    template<typename T, typename... Ts>
    concept OneOf = (std::same_as<T, Ts> or ...);
}
