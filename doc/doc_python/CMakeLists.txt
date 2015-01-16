configure_file(${CMAKE_CURRENT_SOURCE_DIR}/source/conf.py.in ${CMAKE_CURRENT_SOURCE_DIR}/source/conf.py)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build_doc.sh.in ${CMAKE_CURRENT_BINARY_DIR}/build_doc.sh)
add_custom_target(doc_python sh ${CMAKE_CURRENT_BINARY_DIR}/build_doc.sh COMMENT "Building HTML docs using Sphinx ...")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doctestenv.py.in ${CMAKE_CURRENT_BINARY_DIR}/doctestenv.py)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_doctest.sh.in ${CMAKE_CURRENT_BINARY_DIR}/run_doctest.sh)
add_custom_target(doctest sh ${CMAKE_CURRENT_BINARY_DIR}/run_doctest.sh COMMENT "Running doctest using Sphinx ...")
