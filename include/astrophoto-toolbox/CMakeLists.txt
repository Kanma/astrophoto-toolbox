add_subdirectory(algorithms)
add_subdirectory(data)
add_subdirectory(images)
add_subdirectory(stacking)

if (ASTROPHOTOTOOLBOX_INCLUDE_PLATESOLVING)
    add_subdirectory(platesolving)
endif()
