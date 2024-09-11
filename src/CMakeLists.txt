set(SRC_FILES
    coordinates.cpp

    algorithms/histogram.cpp

    astrometry/astrometry.cpp

    data/fits.cpp
    data/transformation.cpp

    images/bitmap.cpp
    images/helpers.cpp
    images/pnm.cpp
    images/raw.cpp

    stacking/registration.cpp
    stacking/starmatcher.cpp
)

set(HEADER_FILES
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/coordinates.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/algorithms/histogram.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/algorithms/math.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/astrometry/astrometry.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/fits.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/point.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/rect.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/size.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/star.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/data/transformation.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/bitmap.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/helpers.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/pnm.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/images/raw.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/stacking/registration.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/stacking/starmatcher.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/stacking/starsdistance.h
    ${CMAKE_SOURCE_DIR}/include/astrophoto-toolbox/stacking/votingpair.h
)

add_library(astrophoto-toolbox ${SRC_FILES} ${HEADER_FILES})

target_include_directories(astrophoto-toolbox
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/include
    PRIVATE
        ${FETCHCONTENT_BASE_DIR}/eigen3-src/
)

target_link_libraries(astrophoto-toolbox
    PUBLIC
        libraw::libraw
        cfitsio
        astrometry-net
)
