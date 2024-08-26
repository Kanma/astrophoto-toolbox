set(SRC_FILES
    coordinates.cpp
)

add_library(astrophoto-toolbox ${SRC_FILES})

target_include_directories(astrophoto-toolbox
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/include
)
