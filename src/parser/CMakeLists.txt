add_library(parser STATIC
    include/parser/parser.hpp
    parser.cpp
    include/parser/statements.hpp
    include/parser/expressions.hpp
)

target_include_directories(parser PUBLIC include)
target_link_libraries(parser PUBLIC lexer backseat_interpreter_options)
target_link_system_libraries(parser PUBLIC tl::optional)
