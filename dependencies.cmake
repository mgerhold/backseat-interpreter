include("${PROJECT_SOURCE_DIR}/cmake/CPM.cmake")
include("${PROJECT_SOURCE_DIR}/cmake/system_link.cmake")

function(backseat_interpreter_setup_dependencies)
    CPMAddPackage(
            NAME TL_OPTIONAL
            GITHUB_REPOSITORY TartanLlama/optional
            VERSION 1.1.0
            OPTIONS
            "EXPECTED_BUILD_PACKAGE OFF"
            "EXPECTED_BUILD_TESTS OFF"
            "EXPECTED_BUILD_PACKAGE_DEB OFF"
    )
endfunction()
