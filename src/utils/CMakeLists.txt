add_library(utils INTERFACE
    include/utils/concepts.hpp
    include/utils/types.hpp
    include/utils/utils.hpp
)

target_link_libraries(utils INTERFACE backseat_interpreter_options)
target_include_directories(utils INTERFACE include)
