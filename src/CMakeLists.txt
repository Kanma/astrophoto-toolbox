set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)


add_library(astrophoto-toolbox)

target_include_directories(astrophoto-toolbox
    PUBLIC
        ${INCLUDE_DIR}
    PRIVATE
        ${FETCHCONTENT_BASE_DIR}/eigen3-src/
)

target_link_libraries(astrophoto-toolbox
    PUBLIC
        libraw::libraw
        cfitsio
)


add_subdirectory(algorithms)
add_subdirectory(catalogs)
add_subdirectory(data)
add_subdirectory(images)
add_subdirectory(stacking)


if (ASTROPHOTOTOOLBOX_INCLUDE_PLATESOLVING)
    add_subdirectory(platesolving)

    target_link_libraries(astrophoto-toolbox
        PUBLIC
            astrometry-net
    )
endif()
