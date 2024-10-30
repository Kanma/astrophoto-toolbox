add_subdirectory(processing)
add_subdirectory(threads)

target_sources(unittests
    PUBLIC
        backgroundcalibration.cpp
        bitmapstacker.cpp
        livestacking.cpp
        registration.cpp
        stacking.cpp
        starmatcher.cpp
)
