cmake_minimum_required(VERSION 3.28)
project(backseat_interpreter CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${PROJECT_SOURCE_DIR}/project_options.cmake)
include(${PROJECT_SOURCE_DIR}/dependencies.cmake)

backseat_interpreter_setup_dependencies()

#if (${backseat_interpreter_build_tests})
#    enable_testing()
#    add_subdirectory(test)
#endif ()

# Bloomberg's clang fork for P2996 (and other related proposals): https://github.com/bloomberg/clang-p2996
# Documentation: https://github.com/bloomberg/clang-p2996
add_compile_options(
        -ftrapv
        -stdlib=libc++
        -freflection # https://wg21.link/p2996
        -fparameter-reflection # https://wg21.link/p3096
        -fexpansion-statements # https://wg21.link/p1306
#        -ftime-trace
#        -fbackseat_interpreter-latest # ???
#        -fconsteval-blocks # https://wg21.link/p3289
#        -fbackseat_interpreter-new-syntax # https://wg21.link/p3381
)

add_link_options(
        -stdlib=libc++
)

add_subdirectory(src bin)
