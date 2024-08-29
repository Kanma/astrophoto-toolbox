add_executable(unittests
    main.cpp
    coordinates.cpp
    images/bitmap.cpp
    images/pnm.cpp
    images/raw.cpp
)

target_include_directories(unittests
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/include
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
)

target_link_libraries(unittests
    PRIVATE
        astrophoto-toolbox
)

set(TESTS_TEMP_DIR "${CMAKE_BINARY_DIR}/tests/tmp/")

target_compile_definitions(unittests
    PRIVATE
        DATA_DIR="${CMAKE_CURRENT_LIST_DIR}/data/"
        TEMP_DIR="${TESTS_TEMP_DIR}"
)

add_subdirectory(data)

# Run the unit tests
add_custom_target(run-unittests
    ALL
    ${CMAKE_COMMAND} -E rm -rf ${TESTS_TEMP_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TESTS_TEMP_DIR}
    COMMAND unittests
    DEPENDS unittests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Unit testing..." VERBATIM
)
