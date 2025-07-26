include(${PROJECT_SOURCE_DIR}/cmake/warnings.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/sanitizers.cmake)

# the following function was taken from:
# https://github.com/cpp-best-practices/cmake_template/blob/main/ProjectOptions.cmake
macro(check_sanitizer_support)
    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
        set(supports_ubsan ON)
        set(supports_thread_sanitizer ON)
    else ()
        set(supports_ubsan OFF)
        set(supports_thread_sanitizer OFF)
    endif ()

    if ((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
        set(supports_asan OFF)
        set(supports_thread_sanitizer OFF)
    else ()
        set(supports_asan ON)
        set(supports_thread_sanitizer OFF)
    endif ()
endmacro()

check_sanitizer_support()

if (PROJECT_IS_TOP_LEVEL)
    option(backseat_interpreter_warnings_as_errors "Treat warnings as errors" ON)
    option(backseat_interpreter_enable_undefined_behavior_sanitizer "Enable undefined behavior sanitizer" ${supports_ubsan})
    option(backseat_interpreter_enable_address_sanitizer "Enable address sanitizer" ${supports_asan})
    option(backseat_interpreter_build_tests "Build tests using Google Test" ON)
else ()
    option(backseat_interpreter_warnings_as_errors "Treat warnings as errors" OFF)
    option(backseat_interpreter_enable_undefined_behavior_sanitizer "Enable undefined behavior sanitizer" OFF)
    option(backseat_interpreter_enable_address_sanitizer "Enable address sanitizer" OFF)
    option(backseat_interpreter_build_tests "Build tests using Google Test" OFF)
endif ()

add_library(backseat_interpreter_warnings INTERFACE)
set_warnings(backseat_interpreter_warnings ${backseat_interpreter_warnings_as_errors})

add_library(backseat_interpreter_sanitizers INTERFACE)
enable_sanitizers(
        backseat_interpreter_sanitizers
        false # ${backseat_interpreter_enable_address_sanitizer}
        false # ${backseat_interpreter_enable_undefined_behavior_sanitizer}
)

add_library(backseat_interpreter_options INTERFACE)
target_link_libraries(backseat_interpreter_options
        INTERFACE backseat_interpreter_warnings
        INTERFACE backseat_interpreter_sanitizers
)
