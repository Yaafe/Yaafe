configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
add_custom_target(doc_cpp doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile COMMENT "Building HTML docs using Doxygen for C++ code...")

