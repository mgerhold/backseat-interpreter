add_executable(main main.cpp
        lexer/lexer.hpp
        lexer/token.hpp
        utils/utils.hpp
        utils/types.hpp
        lexer/regex.hpp
        utils/concepts.hpp
        lexer/token_definitions.hpp)

target_link_libraries(main backseat_interpreter_options)
