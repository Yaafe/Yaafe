# matlab files install
file(GLOB MATLAB_FILES yaafe*.m Yaafe.m)
install(FILES ${MATLAB_FILES} DESTINATION ${CMAKE_INSTALL_MATLAB_SCRIPTS})
