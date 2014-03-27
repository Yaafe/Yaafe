file(GLOB_RECURSE yaafe_components_SOURCES *.cpp)

set(yaafe_components_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src_cpp/ ${EIGEN3_INCLUDE_DIR})
set(yaafe_components_LIBS )

if (WITH_FFTW3)
   list(APPEND yaafe_components_INCLUDE_DIRS ${FFTW3_INCLUDE_DIR})
   list(APPEND yaafe_components_LIBS ${FFTW3_LIBRARY})
endif (WITH_FFTW3)

if (WITH_LAPACK)
   list(APPEND yaafe_components_LIBS ${LAPACK_LIBRARY})
else (WITH_LAPACK)
    file(GLOB tmpfile audio/LPC2LSF.cpp)
    list(REMOVE_ITEM yaafe_components_SOURCES ${tmpfile})
endif (WITH_LAPACK)

include_directories(${yaafe_components_INCLUDE_DIRS})
link_directories(${CMAKE_BINARY_DIR}/src_cpp/yaafe-core)
add_library (yaafe-components MODULE ${yaafe_components_SOURCES})
if (NOT APPLE)
   set_target_properties(yaafe-components PROPERTIES VERSION ${YAAFE_LIBVERSION} SOVERSION ${YAAFE_API_VERSION})
endif (NOT APPLE)
target_link_libraries(yaafe-components yaafe-core ${yaafe_components_LIBS})

install(TARGETS yaafe-components DESTINATION ${CMAKE_INSTALL_LIBDIR})

