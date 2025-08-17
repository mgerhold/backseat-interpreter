#include <generated.hpp>
#include <print>
#include <token_type.hpp>
#include <utils/types.hpp>
#include <utils/files.hpp>
#include <experimental/meta>
#include <ranges>
#include <cctype>
#include <utils/enum_to_string.hpp>
#include "lexer.hpp"

auto print_char_mask(lexer::CharMask const& mask) -> void {
    for (auto const i : std::views::iota(0uz, mask.size())) {
        if (mask.contains(static_cast<char>(i))) {
            if (std::isspace(static_cast<unsigned char>(i))) {
                std::print("<space>");
            } else if (std::isprint(static_cast<unsigned char>(i))) {
                std::print("{}", static_cast<char>(i));
            } else {
                std::print("<?>");
            }
        }
    }
}

auto main() -> int {
    auto const path = std::filesystem::path{ "source.bs" };
    auto const filename = std::string_view{ path.c_str() };
    auto const source = utils::read_file(path);
    if (not source.has_value()) {
        return EXIT_FAILURE;
    }
    try {
        auto const tokens = lexer::tokenize(filename, std::string_view{ source.value() });
        std::println("There are {} tokens.", tokens.size());
        for (auto const& token : tokens) {
            std::println("{}", utils::enum_to_string(token.type()));
            token.source_location().pretty_print(
                stdout,
                lexer::TextWithColor{ "info", utils::TextColor::Blue },
                lexer::TextWithColor{
                    std::format("this is a `{}` token", utils::enum_to_string(token.type())),
                    utils::TextColor::White,
                },
                lexer::TextWithColor{ "this is the token", utils::TextColor::Blue },
                lexer::TextWithColor{ "info", utils::TextColor::Blue },
                lexer::TextWithColor{ "this is just a test", utils::TextColor::White }
            );
        }
    } catch (lexer::LexerError const& error) {
        error.source_location().pretty_print(
            stderr,
            lexer::TextWithColor{ "error", utils::TextColor::Red },
            lexer::TextWithColor{ error.what(), utils::TextColor::BrightWhite },
            lexer::TextWithColor{ "", utils::TextColor::Red }
        );
    }
}
