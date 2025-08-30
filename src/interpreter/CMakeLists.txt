add_executable(interpreter
        main.cpp
        interpreter.hpp
        values.hpp
)

target_link_libraries(interpreter PUBLIC type_checker)
