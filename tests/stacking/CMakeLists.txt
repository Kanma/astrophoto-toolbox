add_subdirectory(processing)
add_subdirectory(threads)
add_subdirectory(utils)

target_sources(unittests
    PUBLIC
        livestacking.cpp
        stacking.cpp
)
