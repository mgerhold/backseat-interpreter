message("Gen header is: ${GEN_HEADER}")

add_library(lexer STATIC
    "${GEN_HEADER}"
    include/lexer/lexer.hpp
    include/lexer/token.hpp
    lexer.cpp
    source_location.hpp
)

add_custom_command(
  OUTPUT     "${GEN_HEADER}"
  COMMAND    ${CMAKE_COMMAND} -E make_directory "${GEN_DIRECTORY}"
  COMMAND    $<TARGET_FILE:pattern_generator> "${GEN_HEADER}"
  DEPENDS    pattern_generator                 # build the generator first
  VERBATIM
  COMMENT    "Generating ${GEN_HEADER}"
)

set_source_files_properties("${GEN_HEADER}" PROPERTIES GENERATED TRUE)

target_include_directories(lexer PRIVATE "${GEN_DIRECTORY}" PUBLIC include)
target_link_libraries(lexer PUBLIC utils backseat_interpreter_options token_types)
