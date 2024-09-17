include_directories(
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/include
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/dependencies
)

add_executable(raw2img raw2img.cpp)
target_link_libraries(raw2img PRIVATE astrophoto-toolbox)

add_executable(register register.cpp)
target_link_libraries(register PRIVATE astrophoto-toolbox)

add_executable(compute-transformation compute-transformation.cpp)
target_link_libraries(compute-transformation PRIVATE astrophoto-toolbox)

if (ASTROPHOTOTOOLBOX_INCLUDE_PLATESOLVING)
    add_executable(find-coordinates find-coordinates.cpp)
    target_link_libraries(find-coordinates PRIVATE astrophoto-toolbox)

    target_compile_definitions(register
        PRIVATE
            ASTROPHOTOTOOLBOX_INCLUDE_PLATESOLVING
    )
endif()
