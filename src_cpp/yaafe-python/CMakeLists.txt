file(GLOB yaafecore_SOURCES *.cpp)
file(GLOB yaafecore_HEADERS *.h)

set(yaafecore_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src_cpp/)
set(yaafecore_LIBS "")

if (WITH_HDF5)
    list(APPEND yaafecore_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
    list(APPEND yaafecore_LIBS ${HDF5_LIBRARY} ${HDF5_HL_LIBRARY})
    add_definitions(-DH5Dopen_vers=2 -DH5Tget_array_dims_vers=2)
endif (WITH_HDF5)

include_directories(${yaafecore_INCLUDE_DIRS})
add_library (yaafecore MODULE ${yaafecore_SOURCES} ${yaafecore_HEADERS})
if (NOT APPLE)
    set_target_properties(yaafecore PROPERTIES VERSION ${YAAFE_LIBVERSION} SOVERSION ${YAAFE_API_VERSION})
endif (NOT APPLE)
target_link_libraries(yaafecore yaafe-core ${yaafecore_LIBS})

install(TARGETS yaafecore DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT yaafe-targets)



