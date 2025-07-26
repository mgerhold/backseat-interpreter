#include "lexer/lexer.hpp"

int main() {
    // Lexer
    auto lexer = lexer::Lexer{ "int if else while loop for" };
    lexer.tokenize();
    // Parser
    // Type-Checking
    // Interpreter
}
