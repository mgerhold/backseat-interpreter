#include <generated.hpp>
#include <print>
#include <token_type.hpp>
#include <utils/types.hpp>
#include <experimental/meta>

auto main() -> int {
    std::println("There are {} patterns.", lexer::patterns.size());
    template for (constexpr auto token_type : std::define_static_array(enumerators_of(^^lexer::TokenType))) {
        std::println("    There are {} states for token type '{}'.",
            lexer::patterns.at(static_cast<usize>([: token_type :])).states.size(),
            display_string_of(token_type)
        );
    }
}
