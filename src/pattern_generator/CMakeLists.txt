add_executable(pattern_generator
    main.cpp
    regex.hpp
    token_definitions.hpp
    sequence_parser.hpp
    pattern_merging.hpp
    pattern.hpp
    char_mask.hpp
)

target_link_libraries(pattern_generator PUBLIC utils backseat_interpreter_options token_types)
