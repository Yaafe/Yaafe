#!/bin/sh

mkdir -p $1

# deploy programs
mkdir -p $1/bin
cp src_python/yaafe.py yaafe-engine $1/bin

# deploy libraries
mkdir -p $1/lib
cp libyaafe-core.so $1/lib

# deploy python packages
export PYPATH=$1/python_packages
mkdir -p $PYPATH
mkdir -p $PYPATH/yaafelib
cp yaafecore.py _yaafecore.so $PYPATH
cp src_python/yaafelib/*.py $PYPATH/yaafelib

# deploy root extension
mkdir -p $1/yaafe_extensions
cp libyaafe-components.so $1/yaafe_extensions
cp src_python/yaafefeatures.py $1/yaafe_extensions

# deploy matlab scripts
mkdir -p $1/matlab
cp matlab/*.m $1/matlab

echo ""
echo "export the following vars to use Yaafe from anywhere:"
echo ""
echo "export YAAFE_PATH=$1/yaafe_extensions"
echo "export PATH=$1/bin:\$PATH"
echo "export LD_LIBRARY_PATH=$1/lib:\$LD_LIBRARY_PATH"
echo "export PYTHONPATH=$1/python_packages:\$PYTHONPATH"
echo "export MATLABPATH=$1/matlab:\$MATLABPATH"
echo ""
echo "Enjoy extracting features with Yaafe !"
echo ""
