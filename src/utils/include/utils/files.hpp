#pragma once

#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace utils {

    [[nodiscard]] inline auto read_file(std::filesystem::path const& path) -> std::optional<std::string> {
        auto file = std::ifstream{ path };
        if (not file) {
            return std::nullopt;
        }
        auto stream = std::ostringstream{};
        stream << file.rdbuf();
        return std::move(stream).str();
    }

}
