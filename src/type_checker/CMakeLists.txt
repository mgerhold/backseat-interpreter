add_library(type_checker STATIC
        include/type_checker/type_checker.hpp
        type_checker.cpp
        include/type_checker/statements.hpp
        include/type_checker/expressions.hpp
        include/type_checker/data_type.hpp
        checking.hpp
        statements.cpp
)

target_include_directories(type_checker PUBLIC include)
target_link_libraries(type_checker PUBLIC parser)
