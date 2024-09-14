cmake_minimum_required(VERSION 3.24.0)
project(astrophoto-toolbox VERSION 0.1.0 LANGUAGES C CXX)

option(ASTROPHOTOTOOLBOX_BUILD_EXAMPLES     "Build the examples         (default=ON)"   ON)
option(ASTROPHOTOTOOLBOX_BUILD_TESTS        "Build (and run) the tests  (default=ON)"   ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)

include(cmake/dependencies.cmake)
include(cmake/astrometry.net.cmake)

add_subdirectory(src)

if (ASTROPHOTOTOOLBOX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (ASTROPHOTOTOOLBOX_BUILD_TESTS)
    add_subdirectory(tests)
endif()
