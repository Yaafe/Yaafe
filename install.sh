#!/bin/sh
#
# build and install yaafe into a virtualenv
#

set -e
rm -rf ./venv
virtualenv --system-site-package venv
OS=`uname`
if [[ "$OS" == "Darwin" ]]; then
    LD_PARAM_NAME="DYLD_FALLBACK_LIBRARY_PATH"
else
    LD_PARAM_NAME="LD_LIBRARY_PATH"
fi
echo "export $LD_PARAM_NAME=\$VIRTUAL_ENV/lib" >> ./venv/bin/activate
VIRTUAL_ENV=`pwd`/venv
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_INSTALL_PREFIX=$VIRTUAL_ENV \
      -DWITH_MPG123=ON ..
make
make install
cd ..
echo "start virtualenv via: \`source ./venv/bin/activate\` and remember to unset $LD_PARAM_NAME manually"
echo ""
echo "Enjoy extracting features with Yaafe !"
echo ""
