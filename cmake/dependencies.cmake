include(FetchContent)

if (POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()


# Disable install commands, because it causes problems between zlib and cfitsio
macro (install)
endmacro()


# Disable building shared libraries (several dependencies use this setting)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)


#################################################
# LibRaw
set(ENABLE_JASPER OFF CACHE BOOL "" FORCE)
set(LIBRAW_PATH "${FETCHCONTENT_BASE_DIR}/libraw-src/" CACHE STRING "" FORCE)

set(LIBRRAW_CMAKE_PATCH git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/libraw-cmake.patch)

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
    PATCH_COMMAND ${LIBRRAW_CMAKE_PATCH}
    UPDATE_DISCONNECTED 1
)

FetchContent_MakeAvailable(LibRaw LibRaw-cmake)

# Disable warnings
target_compile_options(raw_r PRIVATE "-w")


#################################################
# astrometry.net lite
set(ASTROMETRY_NET_LITE_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    astrometry-net-lite
    GIT_REPOSITORY https://github.com/Kanma/astrometry.net-lite.git
    GIT_TAG "5a199385393339abb512f5583d5994c48f6d4625"
)

FetchContent_MakeAvailable(astrometry-net-lite)
