add_executable(unittests main.cpp)

target_include_directories(unittests
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

target_link_libraries(unittests
    PRIVATE
        astrophoto-toolbox
)

set(TESTS_TEMP_DIR "${CMAKE_BINARY_DIR}/tests/tmp/")

target_compile_definitions(unittests
    PRIVATE
        DATA_DIR="${CMAKE_CURRENT_LIST_DIR}/assets/"
        TEMP_DIR="${TESTS_TEMP_DIR}"
        CATALOGS_FOLDER="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/catalogs"
)

add_subdirectory(assets)
add_subdirectory(algorithms)
add_subdirectory(catalogs)
add_subdirectory(data)
add_subdirectory(images)
add_subdirectory(platesolving)
add_subdirectory(stacking)


# Run the unit tests
if (ASTROPHOTOTOOLBOX_RUN_TESTS)
    add_custom_target(run-unittests
        ALL
        unittests
        DEPENDS unittests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Unit testing..." VERBATIM
    )
endif()
