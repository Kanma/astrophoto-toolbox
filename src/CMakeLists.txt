add_library(astrophoto-toolbox)

target_include_directories(astrophoto-toolbox
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
    PRIVATE
        ${FETCHCONTENT_BASE_DIR}/eigen3-src/
        ${PROJECT_SOURCE_DIR}/dependencies
)

target_link_libraries(astrophoto-toolbox
    PUBLIC
        libraw::libraw_r
        cfitsio
        astrometry-net-lite
)

if(WIN32)
    target_compile_definitions(astrophoto-toolbox PUBLIC WINDOWS_LEAN_AND_MEAN NOMINMAX)
endif()

add_subdirectory(algorithms)
add_subdirectory(catalogs)
add_subdirectory(data)
add_subdirectory(images)
add_subdirectory(platesolving)
add_subdirectory(stacking)
