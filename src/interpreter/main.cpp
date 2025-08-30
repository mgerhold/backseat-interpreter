#include <type_checker/type_checker.hpp>
#include <utils/files.hpp>
#include <filesystem>
#include <lexer/lexer.hpp>
#include <parser/parser.hpp>
#include "interpreter.hpp"

int main() {
    try {
        static constexpr auto path = std::string_view{ "source.bs" };
        auto const contents = utils::read_file(path);
        auto const tokens = lexer::tokenize(path, contents.value());
        auto parse_tree = parser::parse(tokens);
        auto ast = type_checker::check_types(std::move(parse_tree));
        auto interpreter = interpreter::Interpreter{ std::move(ast) };
        interpreter.run();
    } catch (std::exception const& e) {
        std::println("{}", e.what());
    } catch (...) {
        std::println("Unexpected exception.");
    }
}
