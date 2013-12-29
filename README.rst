Yaafe(Yet Another Audio Feature Extractor)
==========================================

.. image:: https://travis-ci.org/mckelvin/Yaafe.png
   :target: http://travis-ci.org/mckelvin/Yaafe


Install
=======

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

Once previous libraries are installed (some may have been locally installed in <lib-path>),
you can compile with the following steps: ::

 git submodule init  # to prepare Eigen code under the externals directory
 git submodule update
 mkdir build
 cd build
 ccmake -DCMAKE_PREFIX_PATH=<lib-path> -DCMAKE_INSTALL_PREFIX=<install-path> ..
 make
 make install

Several options can control Yaafe compilation. More details can be found at:
	https://yaafe.sourceforge.net/manual/install.html


Environment
===========

To easily use Yaafe, you should set the following environment vars::

 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEST_DIR/lib

On MacOSX replace `LD_LIBRARY_PATH` by `DYLD_FALLBACK_LIBRARY_PATH`

If you use Matlab, you can set your MATLABPATH var::

 export MATLABPATH=$MATLABPATH:$DEST_DIR/matlab


Documentation
=============

Documentation is also available online: http://yaafe.sourceforge.net/

To build documentation, you need Sphinx.
Before building documentation, you should set your environment correctly so that sphinx builds documentation
with automatic features documentation.

To build documentation, just run ``make doc`` in the ``build` directory. Documentation is built in doc/html directory.

License
=======

YAAFE is released under the version 3 of the GNU Lesser General Public License. Read COPYING and COPYING.LESSER for
more details. The user should also read DISCLAIMER before linking with optional libraries which have different license policy.

Support
=======

To get help with YAAFE, use the mailing-list yaafe-users@lists.sourceforge.net (registration
at https://lists.sourceforge.net/lists/listinfo/yaafe-users ).
