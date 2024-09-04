set(ASTROMETRY_NET_SRC_DIR "${FETCHCONTENT_BASE_DIR}/astrometry-net-src")

file(WRITE ${CMAKE_BINARY_DIR}/generated/astrometry/os-features-config.h)

set(SRC_FILES
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/anqfits.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_byteswap.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_card.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_convert.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_error.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_float.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_header.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_memory.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_rw.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_table.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_time.c
    ${ASTROMETRY_NET_SRC_DIR}/qfits-an/qfits_tools.c
    ${ASTROMETRY_NET_SRC_DIR}/util/an-endian.c
    ${ASTROMETRY_NET_SRC_DIR}/util/bl.c
    ${ASTROMETRY_NET_SRC_DIR}/util/ctmf.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dallpeaks.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dcen3x3.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dfind.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dmedsmooth.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dobjects.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dpeaks.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dselip.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dsigma.c
    ${ASTROMETRY_NET_SRC_DIR}/util/dsmooth.c
    ${ASTROMETRY_NET_SRC_DIR}/util/errors.c
    ${ASTROMETRY_NET_SRC_DIR}/util/fitsioutils.c
    ${ASTROMETRY_NET_SRC_DIR}/util/image2xy.c
    ${ASTROMETRY_NET_SRC_DIR}/util/ioutils.c
    ${ASTROMETRY_NET_SRC_DIR}/util/log.c
    ${ASTROMETRY_NET_SRC_DIR}/util/mathutil.c
    ${ASTROMETRY_NET_SRC_DIR}/util/permutedsort.c
    ${ASTROMETRY_NET_SRC_DIR}/util/resample.c
    ${ASTROMETRY_NET_SRC_DIR}/util/simplexy.c
    ${ASTROMETRY_NET_SRC_DIR}/util/tic.c
)

add_library(astrometry-net ${SRC_FILES})

target_include_directories(astrometry-net
    PUBLIC
        ${ASTROMETRY_NET_SRC_DIR}/include
    PRIVATE
        ${ASTROMETRY_NET_SRC_DIR}/include/astrometry
        ${CMAKE_BINARY_DIR}/generated
)
