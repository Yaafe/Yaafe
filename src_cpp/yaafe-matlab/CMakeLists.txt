file(GLOB yaafemex_SOURCE yaafemex.cpp)

set(YAAFE_MEXFILE ${CMAKE_BINARY_DIR}/src_cpp/yaafe-matlab/yaafemex.${MATLAB_MEX_EXTENSION})

ADD_CUSTOM_TARGET(yaafemex ALL echo
    DEPENDS ${YAAFE_MEXFILE}
)

ADD_CUSTOM_COMMAND(
  OUTPUT ${YAAFE_MEXFILE}
  COMMAND   ${MATLAB_MEX_COMPILER}
  ARGS      -O -g -I${MATLAB_INCLUDE_DIR} -I${CMAKE_SOURCE_DIR}/src_cpp -L${CMAKE_BINARY_DIR}/src_cpp/yaafe-core -ldl -lyaafe-core ${yaafemex_SOURCE}
  MAIN_DEPENDENCY ${yaafemex_SOURCE}
  DEPENDS yaafe-core
  COMMENT   "building MEX"
)

install(FILES ${YAAFE_MEXFILE} DESTINATION ${CMAKE_INSTALL_MATLAB_SCRIPTS})
