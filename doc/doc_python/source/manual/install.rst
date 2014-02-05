Installing Yaafe
================

Getting Yaafe
-------------

You can download *Yaafe* source code, *yaafe-v?.tgz*, from the project's home page: `http://yaafe.sourceforge.net <http://yaafe.sourceforge.net>`_.

Getting Dependencies
--------------------

*Yaafe* source code should compile on *linux* and *MacOsX* platform, and uses CMake as compilation tool.

Yaafe requires thirdparty libraries to enable specific features. Some of these library may already be available on your system.

The `argtable <http://argtable.sourceforge.net/>`_ library is required.

Depending on optional features you want to use, other librairies may be used:

* `libsndfile <http://www.mega-nerd.com/libsndfile/>`_: enable reading WAV files format (highly recommanded)
* `libmpg123 <http://http://www.mpg123.de/api/>`_: enable reading MP3 audio files
* `HDF5 <http://www.hdfgroup.org/HDF5/>`_ >= 1.8: enable H5 output format
* `liblapack <http://www.netlib.org/lapack/>`_: enable some audio features (LSF)
* `FFTW3 <http://www.fftw.org/>`_: use FFTW instead of Eigen for FFT computations (pay attention to licensing issues when linking with the GPL FFTW3 library).

To use the *yaafe.py* script you need Python >= 2.5, and the numpy package.

Debian/Ubuntu packages
""""""""""""""""""""""

The above dependencies can be installed using the following debian/ubuntu packages:

:cmake:	cmake cmake-curses-gui
:argtable2: libargtable2-0 libargtable2-dev
:libsndfile: libsndfile1 libsndfile1-dev
:libmpg123: libmpg123-0 libmpg123-dev
:lapack: liblapack-dev
:hdf5: libhdf5-serial-dev libhdf5-serial-1.8.4
:fftw3: libfftw3-3 libfftw3-dev

To get all those packages with ``apt-get``::

	> sudo apt-get install cmake cmake-curses-gui libargtable2-0 libargtable2-dev libsndfile1 libsndfile1-dev libmpg123-0 libmpg123-dev libfftw3-3 libfftw3-dev liblapack-dev libhdf5-serial-dev libhdf5-serial-1.8.4


Building on Unix/MacOsX
-----------------------

Once previous libraries are installed (some may have been locally installed in <lib-path>),
you can compile with the following steps:

1. Create build directory and change to it:
"""""""""""""""""""""""""""""""""""""""""""

::
	
	> mkdir build
	> cd build

2. Start CMake and set options:
"""""""""""""""""""""""""""""""

::

	> ccmake -DCMAKE_PREFIX_PATH=<lib-path> -DCMAKE_INSTALL_PREFIX=<install-path> ..

The ``CMAKE_PREFIX_PATH`` option allows to specify a directory where Cmake looks for thirdparty libraries (lib/ and include/ directories).
Additional options can also be used to define the location of a particular library: ``SNDFILE_ROOT``, ``MPG123_ROOT``, ``ARGTABLE2_ROOT``, ``HDF5_ROOT``, ``MATLAB_ROOT``, ``FFTW3_ROOT``.  

``ccmake`` starts an interactive gui which enables you to control various build options.

Several options can be set to control *Yaafe* build:

* WITH_FFTW3: enable use of FFTW to compute Fast Fourier transforms
* WITH_HDF5: enable HDF5 output format
* WITH_LAPACK: enable some audio features (LSF)
* WITH_MATLAB_MEX: enable building of matlab mex to extract features within Matlab environment.
* WITH_MPG123: enable read audio from MP3 files
* WITH_SNDFILE: enable read audio from WAV files (enabled by default)
* WITH_TIMERS: enable timers for debugging purpose (if enabled, the ``yaafe-engine`` program displays cpu time used by each component).

Those options can be set interactively within the ccmake gui, or can be set passing ``-DWITH_XXX=ON -DWITH_XXX=OFF`` arguments to the ``cmake`` or ``ccmake`` programs. 

3. Build and install:
"""""""""""""""""""""

::

	> make
	> make install

4. Setting your environment
"""""""""""""""""""""""""""

You should find the following directories under your installation path::

	INSTALL_DIR/bin/                # scripts and binaries
	INSTALL_DIR/include/            # C++ headers to develop new components or integrate Yaafe in your application 
	INSTALL_DIR/lib/                # dynamic libraries
	INSTALL_DIR/matlab/             # matlab scripts, and mex if WITH_MATLAB_MEX was enabled
	INSTALL_DIR/python_packages/    # Python part of Yaafe
	INSTALL_DIR/yaafe_extensions/   # Yaafe extensions dir


You *must* set your YAAFE_PATH environment var to the "yaafe_extensions" dir so that yaafe can find available features::

	export YAAFE_PATH=$INSTALL_DIR/yaafe_extensions

To easily use Yaafe, you should set the following environment vars::

	export PATH=$PATH:$INSTALL_DIR/bin
	# on MacOsX replace LD_LIBRARY_PATH => DYLD_LIBRARY_PATH
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR/lib
	export PYTHONPATH=$PYTHONPATH:$INSTALL_DIR/python_packages

if you use Matlab, you can set your MATLABPATH var::

	export MATLABPATH=$MATLABPATH:$INSTALL_DIR/matlab

	