set(SRC_FILES
    coordinates.cpp

    algorithms/histogram.cpp

    data/fits.cpp
    data/transformation.cpp

    images/bitmap.cpp
    images/helpers.cpp
    images/pnm.cpp
    images/raw.cpp

    platesolving/platesolver.cpp

    stacking/registration.cpp
    stacking/starmatcher.cpp
)

set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)

set(HEADER_FILES
    ${INCLUDE_DIR}/astrophoto-toolbox/coordinates.h
    ${INCLUDE_DIR}/astrophoto-toolbox/algorithms/histogram.h
    ${INCLUDE_DIR}/astrophoto-toolbox/algorithms/math.h
    ${INCLUDE_DIR}/astrophoto-toolbox/data/fits.h
    ${INCLUDE_DIR}/astrophoto-toolbox/data/point.h
    ${INCLUDE_DIR}/astrophoto-toolbox/data/rect.h
    ${INCLUDE_DIR}/astrophoto-toolbox/data/size.h
    ${INCLUDE_DIR}/astrophoto-toolbox/data/star.h
    ${INCLUDE_DIR}/astrophoto-toolbox/data/transformation.h
    ${INCLUDE_DIR}/astrophoto-toolbox/images/bitmap.h
    ${INCLUDE_DIR}/astrophoto-toolbox/images/helpers.h
    ${INCLUDE_DIR}/astrophoto-toolbox/images/pnm.h
    ${INCLUDE_DIR}/astrophoto-toolbox/images/raw.h
    ${INCLUDE_DIR}/astrophoto-toolbox/platesolving/platesolver.h
    ${INCLUDE_DIR}/astrophoto-toolbox/stacking/registration.h
    ${INCLUDE_DIR}/astrophoto-toolbox/stacking/starmatcher.h
    ${INCLUDE_DIR}/astrophoto-toolbox/stacking/starsdistance.h
    ${INCLUDE_DIR}/astrophoto-toolbox/stacking/votingpair.h
)

add_library(astrophoto-toolbox ${SRC_FILES} ${HEADER_FILES})

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
        astrometry-net
)
