set(SRC_FILES
    coordinates.cpp

    astrometry/astrometry.cpp

    data/fits.cpp

    images/bitmap.cpp
    images/pnm.cpp
    images/raw.cpp
)

set(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/coordinates.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/astrometry/astrometry.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/astrometry/starlist.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/fits.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/bitmap.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/pnm.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/raw.h
)

add_library(astrophoto-toolbox ${SRC_FILES} ${HEADER_FILES})

target_include_directories(astrophoto-toolbox
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/include
)

target_link_libraries(astrophoto-toolbox
    PUBLIC
        libraw::libraw
        cfitsio
        astrometry-net
)
