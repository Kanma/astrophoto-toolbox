include(FetchContent)

cmake_policy(SET CMP0169 OLD)

# LibRaw configuration
set(BUILD_SHARED_LIBS OFF)
set(ENABLE_JASPER OFF)
set(LIBRAW_PATH "${FETCHCONTENT_BASE_DIR}/libraw-src/")

# Fetch the repositories
FetchContent_Declare(
    LibRaw
    GIT_REPOSITORY https://github.com/LibRaw/LibRaw.git
    GIT_TAG "a3d02d0c5623d1d7f0cedc65229b478c3a083ddf" # aka "0.21-stable"
)

FetchContent_Declare(
    LibRaw-cmake
    GIT_REPOSITORY https://github.com/LibRaw/LibRaw-cmake.git
    GIT_TAG "eb98e4325aef2ce85d2eb031c2ff18640ca616d3"
)

FetchContent_Declare(
    cfitsio
    GIT_REPOSITORY https://github.com/HEASARC/cfitsio.git
    GIT_TAG "6ba7e3319c02c0831c860c37212f37f37a726cce" # aka "cfitsio4_4_1_20240617"
)

FetchContent_Declare(
    astrometry-net
    GIT_REPOSITORY https://github.com/dstndstn/astrometry.net.git
    GIT_TAG "fbca48ebec403d4f954c97bc83d260ea40643577" # aka "0.95"
)

FetchContent_Declare(
    eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG "3147391d946bb4b6c68edd901f2add6ac1f31f8c" # aka 3.4.0"
)

FetchContent_MakeAvailable(LibRaw LibRaw-cmake cfitsio astrometry-net)

FetchContent_GetProperties(eigen3)
if (NOT eigen3_POPULATED)
    FetchContent_Populate(eigen3)
endif()


# Disable warnings in LibRaw
target_compile_options(raw PRIVATE "-Wno-deprecated-declarations")
target_compile_options(raw_r PRIVATE "-Wno-deprecated-declarations")

# Disable warnings in cfitsio
target_compile_options(cfitsio
    PRIVATE
        "-Wno-pointer-bool-conversion"
        "-Wno-incompatible-pointer-types-discards-qualifiers"
        "-Wno-comment"
        "-Wno-format"
        "-Wno-unused-value"
)
