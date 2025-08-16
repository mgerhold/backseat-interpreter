set(GEN_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GEN_HEADER "${GEN_DIRECTORY}/generated.hpp")

add_subdirectory(utils)
add_subdirectory(token_types)
add_subdirectory(pattern_generator)
add_subdirectory(lexer)
