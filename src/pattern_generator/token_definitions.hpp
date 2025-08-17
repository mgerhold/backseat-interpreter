#pragma once

#include <token_type.hpp>
#include "regex.hpp"
#include <print>

namespace lexer {
    struct PatternDescription final {
        TokenType token_type;
        Sequence pattern;
        bool should_emit;
    };

    [[nodiscard]] consteval auto get_pattern_descriptions() -> auto {
        return std::array{
            PatternDescription{
                TokenType::LowercaseIdentifier,
                Sequence{
                    Range{ 'a', 'z' } | '_',
                    ZeroOrMoreOf{ Range{ 'a', 'z' } | Range{ 'A', 'Z' } | Range{ '0', '9' } | '_' },
                },
                true,
            },
            PatternDescription{
                TokenType::UppercaseIdentifier,
                Sequence{
                    Range{ 'A', 'Z' },
                    ZeroOrMoreOf{ Range{ 'a', 'z' } | Range{ 'A', 'Z' } | Range{ '0', '9' } | '_' },
                },
                true,
            },
            PatternDescription{ TokenType::LowercaseFunction, Sequence{ "function" }, true },
            PatternDescription{ TokenType::Colon, Sequence{ ':' }, true },
            PatternDescription{ TokenType::Comma, Sequence{ ',' }, true },
            PatternDescription{ TokenType::TildeArrow, Sequence{ "~>" }, true },
            PatternDescription{ TokenType::EndOfFile, Sequence{ '\0' }, true },
            PatternDescription{ TokenType::Semicolon, Sequence{ ';' }, true },
            PatternDescription{ TokenType::Plus, Sequence{ '+' }, true },
            PatternDescription{ TokenType::Minus, Sequence{ '-' }, true },
            PatternDescription{ TokenType::Asterisk, Sequence{ '*' }, true },
            PatternDescription{ TokenType::Mod, Sequence{ "mod" }, true },
            PatternDescription{ TokenType::ForwardSlash, Sequence{ '/' }, true },
            PatternDescription{ TokenType::LeftParenthesis, Sequence{ '(' }, true },
            PatternDescription{ TokenType::RightParenthesis, Sequence{ ')' }, true },
            PatternDescription{ TokenType::LeftCurlyBracket, Sequence{ '{' }, true },
            PatternDescription{ TokenType::RightCurlyBracket, Sequence{ '}' }, true },
            PatternDescription{ TokenType::Let, Sequence{ "let" }, true },
            PatternDescription{ TokenType::Equals, Sequence{ '=' }, true },
            PatternDescription{
                TokenType::SignedIntegerLiteral,
                Sequence{
                    Maybe{ '-' },
                    Range{ '0', '9' },
                    ZeroOrMoreOf{
                        Sequence{
                            Maybe{ '\'' },
                            Range{ '0', '9' },
                        },
                    },
                    '_', 'i', '6', '4',
                },
                true,
            },
            PatternDescription{
                TokenType::UnsignedIntegerLiteral,
                Sequence{
                    Range{ '0', '9' },
                    ZeroOrMoreOf{
                        Sequence{
                            Maybe{ '\'' },
                            Range{ '0', '9' },
                        },
                    },
                    '_', 'u', '6', '4',
                },
                true,
            },
            PatternDescription{
                TokenType::CharLiteral,
                Sequence{
                    '\'',
                    EitherOf{
                        // Either an escape sequence...
                        Sequence{ '\\', CharSet{ 'n', 't', 'f', 'r', '\\', '\'' } },
                        // ...or a normal character.
                        CharSet{ '\n', '\t', '\f', '\r', '\\', '\'' }.inverse(),
                    },
                    '\'',
                },
                true,
            },
            PatternDescription{
                TokenType::StringLiteral,
                Sequence{
                    '"',
                    ZeroOrMoreOf{
                        EitherOf{
                            // Either an escape sequence...
                            Sequence{ '\\', CharSet{ 'n', 't', 'f', 'r', '\\', '"' } },
                            // ...or a normal character.
                            CharSet{ '\n', '\t', '\f', '\r', '\\', '"' }.inverse(),
                        },
                    },
                    '"',
                },
                true,
            },
            PatternDescription{
                TokenType::BoolLiteral,
                Sequence{
                    EitherOf{
                        Sequence{ "true" },
                        Sequence{ "false" },
                    },
                },
                true,
            },
            PatternDescription{ TokenType::And, Sequence{ "and" }, true },
            PatternDescription{ TokenType::Or, Sequence{ "or" }, true },
            PatternDescription{ TokenType::Not, Sequence{ "not" }, true },
            PatternDescription{ TokenType::If, Sequence{ "if" }, true },
            PatternDescription{ TokenType::Else, Sequence{ "else" }, true },
            PatternDescription{ TokenType::Loop, Sequence{ "loop" }, true },
            PatternDescription{ TokenType::Break, Sequence{ "break" }, true },
            PatternDescription{ TokenType::Continue, Sequence{ "continue" }, true },
            PatternDescription{ TokenType::While, Sequence{ "while" }, true },
            PatternDescription{ TokenType::Do, Sequence{ "do" }, true },
            PatternDescription{ TokenType::For, Sequence{ "for" }, true },
            PatternDescription{ TokenType::Mutable, Sequence{ "mutable" }, true },
            PatternDescription{ TokenType::Const, Sequence{ "const" }, true },
            PatternDescription{ TokenType::EqualsEquals, Sequence{ "==" }, true },
            PatternDescription{ TokenType::ExclamationEquals, Sequence{ "!=" }, true },
            PatternDescription{ TokenType::GreaterThan, Sequence{ '>' }, true },
            PatternDescription{ TokenType::LessThan, Sequence{ '<' }, true },
            PatternDescription{ TokenType::GreaterOrEquals, Sequence{ ">=" }, true },
            PatternDescription{ TokenType::LessOrEquals, Sequence{ "<=" }, true },
            PatternDescription{ TokenType::Return, Sequence{ "return" }, true },
            PatternDescription{ TokenType::NothingLiteral, Sequence{ "nothing" }, true },
            PatternDescription{ TokenType::UppercaseFunction, Sequence{ "Function" }, true },
            PatternDescription{
                TokenType::LineComment,
                Sequence{ '/', '/', ZeroOrMoreOf{ CharSet{ '\n', '\0' }.inverse() } },
                false,
            },
            PatternDescription{
                TokenType::Whitespace,
                Sequence{
                    CharSet{ ' ', '\f', '\n', '\r', '\t', '\v' },
                },
                false,
            },
        };
    }

} // namespace lexer
