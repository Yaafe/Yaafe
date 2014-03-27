file(GLOB yaafe_python_SOURCES *.cpp)
file(GLOB yaafe_python_HEADERS *.h)

set(yaafe_python_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src_cpp/)
set(yaafe_python_LIBS "")

if (WITH_HDF5)
    list(APPEND yaafe_python_INCLUDE_DIRS ${HDF5_INCLUDE_DIR})
    list(APPEND yaafe_python_LIBS ${HDF5_LIBRARY} ${HDF5_HL_LIBRARY})
    add_definitions(-DH5Dopen_vers=2 -DH5Tget_array_dims_vers=2)
endif (WITH_HDF5)

include_directories(${yaafe_python_INCLUDE_DIRS})
add_library (yaafe-python MODULE ${yaafe_python_SOURCES} ${yaafe_python_HEADERS})
if (NOT APPLE)
    set_target_properties(yaafe-python PROPERTIES VERSION ${YAAFE_LIBVERSION} SOVERSION ${YAAFE_API_VERSION})
endif (NOT APPLE)
target_link_libraries(yaafe-python yaafe-core ${yaafe_python_LIBS})

install(TARGETS yaafe-python DESTINATION ${CMAKE_INSTALL_LIBDIR} EXPORT yaafe-targets)



