#pragma once

namespace parser {
    enum class Precedence {
        Unknown,
        Term,
        Factor,
        Unary,
    };
}
