
add_subdirectory (yaafe-core)
add_subdirectory (yaafe-components)
add_subdirectory (yaafe-io)
add_subdirectory (yaafe-python)
if (WITH_MATLAB_MEX)
    add_subdirectory(yaafe-matlab)
endif (WITH_MATLAB_MEX)

include_directories(${ARGTABLE2_INCLUDE_DIR})
link_directories(${CMAKE_BINARY_DIR}/src_cpp/yaafe-core)

add_executable(yaafe-engine main.cpp)
target_link_libraries(yaafe-engine yaafe-core ${ARGTABLE2_LIBRARY})

install(TARGETS yaafe-engine RUNTIME DESTINATION bin)
