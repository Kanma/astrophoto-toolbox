cmake_minimum_required(VERSION 3.5.0)
project(astrophoto-toolbox VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Dependencies _________________________________________________________________

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

FetchContent_MakeAvailable(LibRaw LibRaw-cmake)

# Disable warnings in LibRaw
target_compile_options(raw PRIVATE "-Wno-deprecated-declarations")
target_compile_options(raw_r PRIVATE "-Wno-deprecated-declarations")


# astrophoto-tools directories _________________________________________________

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
