#pragma once

#include <string_view>
#include <utils/types.hpp>
#include <print>
#include <utils/colors.hpp>

namespace lexer {
    struct TextWithColor final {
        std::string_view text;
        utils::TextColor color;
    };

    struct SourcePosition final {
        usize line{};
        usize column{};
    };

    class SourceLocation final {
    private:
        std::string_view m_filename;
        std::string_view m_source;
        usize m_offset;
        usize m_length;

    public:
        [[nodiscard]] SourceLocation(
            std::string_view const filename,
            std::string_view const source,
            usize const offset,
            usize const length
        )
            : m_filename(filename),
              m_source(source),
              m_offset(offset),
              m_length(length) { }

        [[nodiscard]] auto filename() const -> std::string_view {
            return m_filename;
        }

        [[nodiscard]] auto offset() const -> usize {
            return m_offset;
        }

        [[nodiscard]] auto length() const -> usize {
            return m_length;
        }

        [[nodiscard]] auto lexeme() const -> std::string_view {
            return m_source.substr(m_offset, m_length);
        }

        [[nodiscard]] auto position() const -> SourcePosition {
            auto line = 1uz;
            auto column = 1uz;
            for (auto i = 0uz; i < m_offset && i < m_source.size(); ++i) {
                ++column;
                if (m_source.at(i) == '\n') {
                    ++line;
                    column = 1uz;
                }
            }
            return SourcePosition{ .line = line, .column = column };
        }

        [[nodiscard]] auto line() const -> std::string_view {
            auto start = m_source.rfind('\n', m_offset);
            if (start == decltype(m_source)::npos) {
                start = 0uz;
            } else {
                ++start; // Move past the newline character.
            }
            auto end = m_source.find('\n', m_offset);
            if (end == decltype(m_source)::npos) {
                end = m_source.size();
            }
            return m_source.substr(start, end - start);
        }

        auto pretty_print(
            FILE* const file,
            TextWithColor const title_short,
            TextWithColor const title_long,
            TextWithColor const annotation,
            std::optional<TextWithColor> const description_short = std::nullopt,
            std::optional<TextWithColor> const description_long = std::nullopt
        ) const -> void {
            static constexpr auto sidebar_color = utils::TextColor::Blue;

            utils::set_text_color(file, title_short.color);
            std::print(file, "{}", title_short.text);
            utils::reset_colors(file);
            std::print(file, ":");
            utils::set_text_color(file, title_long.color);
            std::println(file, " {}", title_long.text);
            auto const[line_num, column_num] = this->position();
            auto const line_num_digits = std::to_string(line_num).size();
            utils::set_text_color(file, sidebar_color);
            std::print(file, "{:{}}--> ", ' ', line_num_digits);
            utils::reset_colors(file);
            std::println(file, "{}:{}:{}", m_filename, line_num, column_num);

            utils::set_text_color(file, sidebar_color);
            std::println(file, "{:{}} |", ' ', line_num_digits);
            std::print(file, "{} | ", line_num);

            utils::reset_colors(file);
            std::println(file, "{}", this->line());

            utils::set_text_color(file, sidebar_color);
            std::print(file, "{:{}} |", ' ', line_num_digits);

            utils::set_text_color(file, annotation.color);
            std::println(
                file,
                "{:{}}{:^>{}} {}",
                ' ',
                column_num,
                '^',
                m_length,
                annotation.text
            );
            std::println(file, "");

            if (description_short.has_value()) {
                utils::set_text_color(file, description_short.value().color);
                std::print(file, "{}", description_short.value().text);
            }
            utils::reset_colors(file);
            if (description_short.has_value() and description_long.has_value()) {
                std::print(file, ": ");
            }
            if (description_long.has_value()) {
                utils::set_text_color(file, description_long.value().color);
                std::println(file, "{}", description_long.value().text);
            }
            utils::reset_colors(file);
        }

    };
} // namespace lexer
