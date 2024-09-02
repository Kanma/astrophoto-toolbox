add_executable(unittests
    main.cpp
    coordinates.cpp

    data/fits.cpp

    images/bitmap.cpp
    images/bitmap_helpers.h
    images/bitmap_helpers.cpp
    images/bitmap_uint8_color.cpp
    images/bitmap_uint16_color.cpp
    images/bitmap_uint32_color.cpp
    images/bitmap_uint8_gray.cpp
    images/bitmap_uint16_gray.cpp
    images/bitmap_uint32_gray.cpp
    images/bitmap_float_color.cpp
    images/bitmap_float_gray.cpp
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
        DATA_DIR="${CMAKE_CURRENT_LIST_DIR}/assets/"
        TEMP_DIR="${TESTS_TEMP_DIR}"
)

add_subdirectory(assets)

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
