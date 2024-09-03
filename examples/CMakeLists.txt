include_directories(
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/include
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>/dependencies
)

add_executable(raw2img raw2img.cpp)
target_link_libraries(raw2img PRIVATE astrophoto-toolbox)

