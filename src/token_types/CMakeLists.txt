add_library(token_types INTERFACE token_type.hpp)

target_include_directories(token_types INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(token_types INTERFACE utils backseat_interpreter_options)
