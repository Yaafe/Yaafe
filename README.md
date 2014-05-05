## Install

*Yaafe* source code should compile on *linux* and *MacOsX* platforms, and uses CMake as a compilation tool.
Yaafe requires third-party libraries to enable specific features. Some of these libraries may already be available on your system.

### Dependencies

#### Required

- Python 2.5+
- [argtable](http://argtable.sourceforge.net)
  - OSX: `brew install argtable`
- [numpy](http://www.numpy.org/)

#### Optional

Depending on optional features you want to use, other librairies may be used:

* [libsndfile](http://www.mega-nerd.com/libsndfile/): enable reading WAV files format (highly recommended)
* [libmpg123](http://http://www.mpg123.de/api/): enable reading MP3 audio files
* [HDF5](http://www.hdfgroup.org/HDF5/) >= 1.8: enable H5 output format
* [liblapack](http://www.netlib.org/lapack/): enable some audio features (LSF)
* [FFTW3](http://www.fftw.org/): use FFTW instead of Eigen for FFT computations (pay attention to licensing issues when linking with the GPL FFTW3 library).

Once previous libraries are installed (some may have been locally installed in `<lib-path>`),
you can compile with the following steps:

    mkdir build
    cd build
    ccmake -DCMAKE_PREFIX_PATH=<lib-path> -DCMAKE_INSTALL_PREFIX=<install-path> ..
    make
    make install

For more compilation options, see the [online manual](http://yaafe.sourceforge.net/manual/install.html).

Environment
===========

You *must* set your YAAFE_PATH environment var to the "yaafe_extensions" dir so that yaafe can find available features::

    export YAAFE_PATH=$DEST_DIR/yaafe_extensions

To easily use Yaafe, you should set the following environment vars::

    export PATH=$PATH:$DEST_DIR/bin
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEST_DIR/lib
    export PYTHONPATH=$PYTHONPATH:$DEST_DIR/python_packages

On MacOsX replace LD_LIBRARY_PATH by DYLD_LIBRARY_PATH

If you use Matlab, you can set your MATLABPATH var::

    export MATLABPATH=$MATLABPATH:$DEST_DIR/matlab


Documentation
=============

Documentation is also available online: http://yaafe.sourceforge.net/

To build documentation, you need Sphinx.
Before building documentation, you should set your environment correctly so that sphinx builds documentation
with automatic features documentation.

To build documentation, just run `make doc` in the `build` directory. Documentation is built in `doc/html`.

License
=======

YAAFE is released under the version 3 of the GNU Lesser General Public License. Read COPYING and COPYING.LESSER for
more details. The user should also read DISCLAIMER before linking with optional libraries which have different license policy.

Support
=======

To get help with YAAFE, use the mailing-list yaafe-users@lists.sourceforge.net (registration
at https://lists.sourceforge.net/lists/listinfo/yaafe-users ).


Credits
=======

Yaafe was first developed at [Telecom Paristech / AAO Team] (http://www.tsi.telecom-paristech.fr/aao/en/). It uses several great open-source projects like [Eigen] (http://eigen.tuxfamily.org/), [Smarc] (http://audio-smarc.sourceforge.net/), [`libsndfile`] (http://www.mega-nerd.com/libsndfile/), [`mpg123`] (http://www.mpg123.de/), [HDF5] (http://www.hdfgroup.org/HDF5/).

If you want to cite Yaafe in a publication, please see `CITATION <CITATION.rst>`_.
