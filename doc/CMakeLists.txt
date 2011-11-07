configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_doc.sh ${CMAKE_CURRENT_BINARY_DIR}/build_doc.sh) 
add_custom_target(doc sh ${CMAKE_CURRENT_BINARY_DIR}/build_doc.sh COMMENT "Building HTML docs using Sphinx ...")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doctestenv.py.in ${CMAKE_CURRENT_BINARY_DIR}/doctestenv.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_doctest.sh ${CMAKE_CURRENT_BINARY_DIR}/run_doctest.sh) 
add_custom_target(doctest sh ${CMAKE_CURRENT_BINARY_DIR}/run_doctest.sh COMMENT "Running doctest using Sphinx ...")
