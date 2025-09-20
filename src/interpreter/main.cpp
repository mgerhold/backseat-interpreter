#include <type_checker/type_checker.hpp>
#include <utils/files.hpp>
#include <filesystem>
#include <lexer/lexer.hpp>
#include <parser/parser.hpp>
#include "interpreter.hpp"
#include <utils/pretty_printer.hpp>

int main() {
    try {
        static constexpr auto path = std::string_view{ "source.bs" };
        auto const contents = utils::read_file(path);
        auto const tokens = lexer::tokenize(path, contents.value());
        auto parse_tree = parser::parse(tokens);
        auto ast = type_checker::check_types(std::move(parse_tree));
        pretty_print(ast);
        auto interpreter = interpreter::Interpreter{ std::move(ast) };
        interpreter.run();
    } catch (std::exception const& e) {
        std::println("{}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        std::println("Unexpected exception.");
        return EXIT_FAILURE;
    }
}
