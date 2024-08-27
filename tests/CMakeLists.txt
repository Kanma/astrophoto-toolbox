add_executable(unittests
    main.cpp
    coordinates.cpp
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

add_subdirectory(images)

# Run the unit tests
add_custom_target(run-unittests ALL unittests
                  DEPENDS unittests
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                  COMMENT "Unit testing..." VERBATIM
)
