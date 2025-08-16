message("Gen header is: ${GEN_HEADER}")

add_executable(lexer
    main.cpp
    "${GEN_HEADER}"
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

target_include_directories(lexer PRIVATE "${GEN_DIRECTORY}")
target_link_libraries(lexer PUBLIC utils backseat_interpreter_options token_types)
