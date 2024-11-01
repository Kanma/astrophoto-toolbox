add_subdirectory(processing)
add_subdirectory(threads)
add_subdirectory(utils)

target_sources(astrophoto-toolbox
    PUBLIC
        stacking.h
        stacking.hpp
)
