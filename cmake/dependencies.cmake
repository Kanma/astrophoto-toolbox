include(FetchContent)

# LibRaw configuration
set(BUILD_SHARED_LIBS OFF)
set(LIBRAW_PATH "${FETCHCONTENT_BASE_DIR}/libraw-src/")

# Fetch the repositories
FetchContent_Declare(
  LibRaw
  GIT_REPOSITORY https://github.com/LibRaw/LibRaw.git
  GIT_TAG "0.21.1-57-g70f51187"
)

FetchContent_Declare(
  LibRaw-cmake
  GIT_REPOSITORY https://github.com/LibRaw/LibRaw-cmake.git
  GIT_TAG "eb98e4325aef2ce85d2eb031c2ff18640ca616d3"
)

FetchContent_Declare(
  cfitsio
  GIT_REPOSITORY https://github.com/HEASARC/cfitsio.git
  GIT_TAG "cfitsio4_4_1_20240617"
)

FetchContent_MakeAvailable(LibRaw LibRaw-cmake cfitsio)

# Disable warnings in LibRaw
target_compile_options(raw PRIVATE "-Wno-deprecated-declarations")
target_compile_options(raw_r PRIVATE "-Wno-deprecated-declarations")
