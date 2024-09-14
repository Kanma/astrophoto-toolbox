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
# zlib
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG "51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf" # aka "v1.3.1"
    OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(zlib)

add_library(ZLIB::ZLIB ALIAS zlibstatic)
target_include_directories(zlibstatic INTERFACE ${zlib_BINARY_DIR} ${zlib_SOURCE_DIR})


#################################################
# Eigen3
FetchContent_Declare(
    eigen3
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG "3147391d946bb4b6c68edd901f2add6ac1f31f8c" # aka "3.4.0"
)

# We do it manually to avoid its compilation: we only need the headers
FetchContent_GetProperties(eigen3)
if (NOT eigen3_POPULATED)
    FetchContent_Populate(eigen3)
endif()


#################################################
# Other dependencies that don't require special care
set(ENABLE_JASPER OFF CACHE BOOL "" FORCE)
set(LIBRAW_PATH "${FETCHCONTENT_BASE_DIR}/libraw-src/" CACHE STRING "" FORCE)


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

FetchContent_MakeAvailable(LibRaw LibRaw-cmake cfitsio)


# Tell cfitsio where to find zlib
target_include_directories(cfitsio PRIVATE ${zlib_BINARY_DIR} ${zlib_SOURCE_DIR})
target_link_libraries(cfitsio zlibstatic)


# Disable warnings in our dependencies
target_compile_options(raw PRIVATE "-w")
target_compile_options(raw_r PRIVATE "-w")
target_compile_options(cfitsio PRIVATE "-w")
