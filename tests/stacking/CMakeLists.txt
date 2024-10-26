add_subdirectory(processing)

target_sources(unittests
    PUBLIC
        backgroundcalibration.cpp
        bitmapstacker.cpp
        livestacking.cpp
        registration.cpp
        stacking.cpp
        starmatcher.cpp
)
