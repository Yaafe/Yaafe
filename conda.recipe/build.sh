#!/bin/bash

mkdir build_yaafe
cd build_yaafe
cmake -DCMAKE_INSTALL_PREFIX=$PREFIX -DDL_ROOT=/usr/lib/x86_64-linux-gnu/ -DWITH_FFTW3=ON -DWITH_HDF5=ON -DWITH_LAPACK=ON -DWITH_MPG123=ON ..
make
make install

# Add more build steps here, if they are necessary.
#cd $PREFIX
#mv python_packages/yaafelib ./lib/python2.7/
# See
# http://docs.continuum.io/conda/build.html
# for a list of environment variables that are set during the build process.
