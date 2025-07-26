add_executable(main main.cpp
        lexer/lexer.hpp
        lexer/token.hpp
        utils/utils.hpp
        utils/types.hpp)

target_link_libraries(main backseat_interpreter_options)
