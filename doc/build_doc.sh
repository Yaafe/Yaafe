#!/bin/sh

export LD_LIBRARY_PATH=@CMAKE_BINARY_DIR@/src_cpp/yaafe-python:@CMAKE_BINARY_DIR@/src_cpp/yaafe-components:$LD_LIBRARY_PATH
export YAAFE_PATH=@CMAKE_SOURCE_DIR@/src_python
export PYTHONPATH=@CMAKE_BINARY_DIR@/src_python:@CMAKE_SOURCE_DIR@/src_python:$PYTHONPATH

@SPHINX_BUILD@ -b html ${CMAKE_CURRENT_SOURCE_DIR}/source ${CMAKE_CURRENT_BINARY_DIR}/html