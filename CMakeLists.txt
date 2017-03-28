cmake_minimum_required (VERSION 2.6)
project (yaafe CXX C)
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

# The version number.
set (YAAFE_VERSION_MAJOR 0)
set (YAAFE_VERSION_MINOR 70)
set (YAAFE_API_VERSION 0)

set (YAAFE_VERSION ${YAAFE_VERSION_MAJOR}.${YAAFE_VERSION_MINOR})
set (YAAFE_LIBVERSION ${YAAFE_VERSION}.${YAAFE_API_VERSION})

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${YAAFE_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Compiler FLAGS
if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif (NOT CMAKE_BUILD_TYPE)
set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -ftemplate-depth-30 -g2" CACHE STRING "CXX DEBUG FLAGS" FORCE)
set(CMAKE_C_FLAGS_DEBUG "-DDEBUG -g2" CACHE STRING "C DEBUG FLAGS" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math" CACHE STRING "CXX RELEASE FLAGS" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -ffast-math" CACHE STRING "C RELEASE FLAGS" FORCE)
add_definitions(-Wno-write-strings)

find_package(Argtable2 REQUIRED)

if (NOT WIN32)
    find_package(DL REQUIRED)
endif (NOT WIN32)

option(WITH_EIGEN_LIBRARY "Use external Eigen3 library" ON)
find_package(Eigen3)
if (WITH_EIGEN_LIBRARY)
    if (NOT EIGEN3_FOUND)
        #message(SEND_ERROR "Cannot find Eigen3 library")
        set(EIGEN3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/eigen)
    endif (NOT EIGEN3_FOUND)
else (WITH_EIGEN_LIBRARY)
    set(EIGEN3_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/eigen)
endif (WITH_EIGEN_LIBRARY)
mark_as_advanced(FORCE EIGEN3_INCLUDE_DIR)

set(SMARC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/audio-smarc)
mark_as_advanced(FORCE SMARC_INCLUDE_DIR)
if(APPLE)
set(FMEMOPEN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/externals/fmemopen)
mark_as_advanced(FORCE FMEMOPEN_INCLUDE_DIR)
endif(APPLE)

# optional packages
option(WITH_SNDFILE "Use sndfile library" ON)
find_package(Sndfile)
if (WITH_SNDFILE)
    if (NOT SNDFILE_FOUND)
        message(SEND_ERROR "Cannot find libsndfile library")
    endif (NOT SNDFILE_FOUND)
    add_definitions(-DWITH_SNDFILE=1)
endif (WITH_SNDFILE)

find_package(HDF5)
if (HDF5_FOUND)
  option(WITH_HDF5 "Use HDF5 library" ON)
else (HDF5_FOUND)
  option(WITH_HDF5 "Use HDF5 library" OFF)
endif (HDF5_FOUND)
if (WITH_HDF5)
    if (NOT HDF5_FOUND)
        message(SEND_ERROR "Cannot find HDF5 library")
    endif (NOT HDF5_FOUND)
    add_definitions(-DWITH_HDF5=1)
    add_definitions(-DH5_NO_DEPRECATED_SYMBOLS)
endif (WITH_HDF5)

find_package(MPG123)
if (MPG123_FOUND)
  option(WITH_MPG123 "Use MPG123 library" ON)
else (MPG123_FOUND)
  option(WITH_MPG123 "Use MPG123 library" OFF)
endif (MPG123_FOUND)
if (WITH_MPG123)
    if (NOT MPG123_FOUND)
        message(SEND_ERROR "Cannot find MPG123 library")
    endif (NOT MPG123_FOUND)
    add_definitions(-DWITH_MPG123=1)
endif (WITH_MPG123)

find_package(Lapack)
if (LAPACK_FOUND)
  option(WITH_LAPACK "Use Lapack library" ON)
else (LAPACK_FOUND)
  option(WITH_LAPACK "Use Lapack library" OFF)
endif (LAPACK_FOUND)
if (WITH_LAPACK)
    if (NOT LAPACK_FOUND)
        message(SEND_ERROR "Cannot find Lapack library")
    endif (NOT LAPACK_FOUND)
    add_definitions(-DWITH_LAPACK=1)
endif (WITH_LAPACK)

find_package(FFTW3)
if (FFTW3_FOUND)
  option(WITH_FFTW3 "Use Lapack library" ON)
else (FFTW3_FOUND)
  option(WITH_FFTW3 "Use FFTW3 library" OFF)
endif (FFTW3_FOUND)
if (WITH_FFTW3)
    if (NOT FFTW3_FOUND)
        message(SEND_ERROR "Cannot find FFTW3 library")
    endif (NOT FFTW3_FOUND)
    add_definitions(-DWITH_FFTW3=1)
    add_definitions(-DEIGEN_FFTW_DEFAULT=1)
endif (WITH_FFTW3)

option(WITH_MATLAB_MEX "build matlab mex and install matlab package" OFF)
find_package(Matlab)
if (WITH_MATLAB_MEX)
    if (NOT MATLAB_FOUND)
        message(SEND_ERROR "Cannot find matlab programs")
    endif (NOT MATLAB_FOUND)
endif (WITH_MATLAB_MEX)

FIND_LIBRARY(RT_LIBRARY NAMES rt PATHS /usr/lib /usr/local/lib)
option(WITH_TIMERS "Use timers" OFF)
if (WITH_TIMERS)
    if (NOT RT_LIBRARY)
        message(SEND_ERROR "Cannot find rt library, needed to enable timers")
    endif (NOT RT_LIBRARY)
    add_definitions(-DWITH_TIMERS=1)
endif(WITH_TIMERS)

FIND_PROGRAM(SPHINX_BUILD sphinx-build PATHS ENV PATH DOC "sphinx-build script used to build documentation")

execute_process ( COMMAND python -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(prefix=''))" OUTPUT_VARIABLE python_packages OUTPUT_STRIP_TRAILING_WHITESPACE )
# install paths
set(CMAKE_INSTALL_BINDIR bin CACHE PATH "Output directory for programs")
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "Output directory for libraries")
set(CMAKE_INSTALL_INCLUDE include CACHE PATH "Output directory for header files")
set(CMAKE_INSTALL_PYTHON_PACKAGES ${python_packages} CACHE PATH "Output directory for yaafe Python packages")
set(CMAKE_INSTALL_MATLAB_SCRIPTS matlab CACHE PATH "Output directory for matlab scripts")

# add targets
add_definitions(-DVERSION=${YAAFE_VERSION})
add_subdirectory (externals)
add_subdirectory (src_cpp)
add_subdirectory (src_python)
if (WITH_MATLAB_MEX)
add_subdirectory (matlab)
endif (WITH_MATLAB_MEX)
add_subdirectory (doc)

# export targets
install(EXPORT yaafe-targets DESTINATION lib/yaafe)
install(FILES yaafe-config.cmake DESTINATION lib/yaafe)

# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE
     "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.LESSER")
set (CPACK_PACKAGE_VERSION_MAJOR "${YAAFE_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${YAAFE_VERSION_MINOR}")
include (CPack)

# add uninstall target
configure_file(
  "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
