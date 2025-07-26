#pragma once

#include "../utils/utils.hpp"

#include <string_view>
#include <experimental/meta>
#include <ranges>
#include <variant>

namespace lexer {
    template<utils::StaticString>
    struct ConstantLexeme{};

    namespace token_descriptions {
        using Int = ConstantLexeme<"int">;
        using Input = ConstantLexeme<"input">;
        using For = ConstantLexeme<"for">;
        using Function = ConstantLexeme<"function">;
        using Plus = ConstantLexeme<"+">;
        // struct Int: Token<"int">;
        // struct Ident: Regex<"[a-zA-Z_][a-zA-Z_0-9]*">;
    };

    template<typename T>
    struct TokenAlternative;

    consteval {
        // - Iterate members of `token_descriptions` namespace.
        constexpr auto context = std::meta::access_context::current();
        for (auto member : members_of(^^token_descriptions, context)) {
            auto alternative = substitute(^^TokenAlternative, { member });
            define_aggregate(alternative, {});
        }
    }

    consteval auto create_token_variant() {
        constexpr auto context = std::meta::access_context::current();
        auto const alternatives = members_of(^^token_descriptions, context)
            | std::views::transform([](auto const member) {
                return substitute(^^TokenAlternative, { member });
            });
        return substitute(^^std::variant, alternatives);
    }

    using TokenKind = [: create_token_variant() :];

    struct Token final {
        std::string_view lexeme;
        TokenKind kind;
    };
}
