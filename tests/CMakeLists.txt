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
)

add_subdirectory(assets)
add_subdirectory(algorithms)
add_subdirectory(data)
add_subdirectory(images)
add_subdirectory(stacking)

if (ASTROPHOTOTOOLBOX_INCLUDE_PLATESOLVING)
    add_subdirectory(platesolving)
endif()


# Run the unit tests
if (ASTROPHOTOTOOLBOX_RUN_TESTS)
    add_custom_target(run-unittests
        ALL
        ${CMAKE_COMMAND} -E rm -rf ${TESTS_TEMP_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_TEMP_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_TEMP_DIR}/empty
        COMMAND unittests
        DEPENDS unittests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Unit testing..." VERBATIM
    )
endif()
