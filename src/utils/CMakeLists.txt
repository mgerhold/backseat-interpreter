add_library(utils INTERFACE
    include/utils/concepts.hpp
    include/utils/types.hpp
    include/utils/utils.hpp
        include/utils/enum_to_string.hpp
        include/utils/files.hpp
        include/utils/colors.hpp
        include/utils/pretty_printer.hpp
)

target_link_libraries(utils INTERFACE backseat_interpreter_options)
target_include_directories(utils INTERFACE include)
