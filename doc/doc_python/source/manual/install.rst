==================
 Installing Yaafe
==================

With Conda
==========

.. |conda| image:: https://anaconda.org/yaafe/yaafe/badges/installer/conda.svg
   :target: https://conda.anaconda.org/yaafe

|conda|
	    
Yaafe can be easily install with `conda <http://conda.pydata.org>`_. To install this package with conda run::
    
  conda install -c https://conda.anaconda.org/yaafe yaafe  


With Docker
===========
Yaafe can also be install through `Docker <https://www.docker.com/>`_.

Get the official *Yaafe* image from `Yaafe repository on Docker Hub <https://hub.docker.com/r/yaafe/yaafe/>`_::
  
  docker pull yaafe/yaafe
  docker tag yaafe yaafe/yaafe

or build it from the sources directory::
    
    docker build --tag=yaafe .

Following both of the method above, you then have a docker image tagged as *yaafe*.
The `yaafe` command is the entrypoint for that docker image so you can run 'yaafe' from the command line through `docker run yaafe`. For example::

    docker run -it yaafe --help
    docker run -it yaafe --volume=$(pwd):/wd  --workdir=/wd -c resources/featureplan -r 16000 --resample resources/yaafe_check.wav

On Linux, it can be usefull to create an `alias` for it::

  alias yaafe='docker run -it --rm=true --volume=$(pwd):/wd  --workdir=/wd yaafe'

You could then simply run::

  yaafe --help
  yaafe -l
    
et voilà !

If you need to set the docker user as the current user on the host, you could try to run docker with the  `-u $(id -u):$(id -g)` option ::
  
    docker run -it --rm=true --volume=$(pwd):/wd  --workdir=/wd -u $(id -u):$(id -g) yaafe -c resources/featureplan -o h5 -r 16000 --resample resources/yaafe_check.wav

Last but not least, the entry-point for the *yaafe* docker image is smart :

- every command that start with a dash `-` will be pass as options to the `yaafe` command inside the docker container
- every command that does not start with a dash will be treated as a regular command. For example::
      
    docker run -it yaafe /bin/bash

will give you access to a bash terminal inside the docker. And ::

    docker run -it yaafe yaafe-engine --help

will launch the  `yaafe-engine` batch processing tool.

    

Installing from sources
=======================

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

To use the *yaafe* script you need Python >= 2.5, and the numpy package.

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

* WITH_FFTW3: enable use of FFTW to compute Fast Fourier transforms (enabled by default if the library is available)
* WITH_HDF5: enable HDF5 output format (enabled by default if the library is available)
* WITH_LAPACK: enable some audio features (LSF) (enabled by default if the library is available)
* WITH_MATLAB_MEX: enable building of matlab mex to extract features within Matlab environment.
* WITH_MPG123: enable read audio from MP3 files (enabled by default if the library is available)
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

