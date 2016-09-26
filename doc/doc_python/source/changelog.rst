Changelog
=========
**v0.70**, 2015-02-25
    Major release.

    * Merge `TPYaafeExtension - Télécom ParisTech Yaafe Extension <http://perso.telecom-paristech.fr/~essid/tp-yaafe-extension/>`_ into Yaafe.

      TPYaafeExtension is an extension of the Yaafe toolbox, developed at `Télécom ParisTech <http://www.tsi.telecom-paristech.fr/aao/en/>`_. It provides several useful mid-level audio features (CQT, Chromas, Chords, ...).
      It is now available under the LGPL licence and was thus merge into Yaafe.

    * Improve Yaafe installation. See :doc:`manual/install`.

      * For Linux-64: Add conda recipes and create conda packages for Yaafe and all its required or optional dependencies. Yaafe can thus now easily be installed in a conda environment.  See https://anaconda.org/Yaafe/yaafe.
	Only available for Linux-64.
	*OSX and Windows developers are very welcome to contribute to build conda recipes for these platform.*  
      * Add docker support to easily deploy Yaafe on every platform. See https://hub.docker.com/r/yaafe/ 
    * Improve documentation:
      * Clean-up and fix thanks to `Pull request #16 <https://github.com/Yaafe/Yaafe/pull/16>`_ and `Pull request #7 <https://github.com/Yaafe/Yaafe/pull/7>`_
      * Fix windows native compilation thanks to `Pull request #13 <https://github.com/Yaafe/Yaafe/pull/13>`_ 
    * Fix and improve `Yaafe` python command line tool:
      * Improve MFCC by limiting the max frequency of Mel filterbank to Nyquist frequency with  `Pull request #12 <https://github.com/Yaafe/Yaafe/pull/12>`_ 
      * Add meaningful error message when -f missing from command with `Pull request #11 <https://github.com/Yaafe/Yaafe/pull/11>`_  


**v0.65**, 2015-01-13
    Bug fix and enhancement release

    * First release after having moved the repository to Github
    * Fix some memory leaks (see `Pull request #2 <https://github.com/Yaafe/Yaafe/pull/2>`_ and `Pull request #4 <https://github.com/Yaafe/Yaafe/pull/4>`_ )
    * Enhancement: support time_start, time_limit for MP3FileReader and AudioFileReader so that we can extract features in a specific time range.
    * Fix installation of Yaafe for windows, (see `Pull request #5 <https://github.com/Yaafe/Yaafe/pull/5>`_)
    * Fix segmentation fault of timer (see `Pull request #6 <https://github.com/Yaafe/Yaafe/pull/6>`_)
    * yaafe command line is now `yaafe`instead of `yaafe.py`

**v0.64**, 2011-11-07
    Minor release.

    * Fix compilation issue: missing cstddef include in ``src_cpp/yaafe-core/Buffer.h``
    * Added MelSpectrum feature

**v0.63**, 2011-08-10
    Minor release.

    * Fix bug when using an invalid stepSize, add error message for empty parameter values
    * Fix compilation error with _mpg123_open_64 symbols
    * Add documentation about creating new audio features

**v0.62**, 2011-05-31
    Minor release.

    * Enable automatic samplerate conversion using `Smarc <http://audio-smarc.sourceforge.net>`_.
    * Improve Loudness feature: add a parameter to enable/disable loudness normalization.
    * Yaafe now supports all formats supported by `libsndfile <http://www.mega-nerd.com/libsndfile/>`_ and not only wav.
    * Fix a bug with the '-n' (normalization) option.
    * Update `Eigen <http://eigen.tuxfamily.org/>`_ to version 3.0.1

**v0.61**, 2011-02-01
	Bug-fix release.

	* Fix shape of feature's numpy array returned by :py:meth:`yaafelib.Engine.readOutput` and :py:meth:`yaafelib.Engine.readAllOutputs` methods.
	* Remove the use of deprecated HDF5 functions.
	* Fix compilation errors under Ubuntu 10.10 when trying to use hdf5 packages. Add compilation flags to force HDF5 using 1.8 API.
	* Add needed ubuntu packages to documentation.

**v0.6**, 2010-12-21
	Deep refactoring of Yaafe internals and adding new features to improve ease of use, building and integration into other projects.

	Most significant changes:

	* *Yaafe* now has several output formats: CSV and H5. Other output format can be added in the future.
		.. warning::
			Default output format is **CSV** because it does not require any thirdparty library. Use ``-o`` and ``-p`` options to produce HDF5 output identical to yaafe v0.53 output (see :ref:`output-format`).
	* *HDF5*, *mpg123*, *lapack* dependencies are optional.
	* Use `Eigen3 <http://eigen.tuxfamily.org>`_ instead of Blitz++: Eigen3 is a template library (do not need to compile Blitz++ any more). Eigen3 project is active, and provide sometime better performances than Blitz++ with easier integration.
	* Python bindings uses *ctypes* instead of *SWIG*. No dependency to Python at compile time is required.
	* C++ API, Python bindings and Matlab bindings to extract features using your favorite language.
	* Internal computation engine has been improved: better computation scheduling and data buffering, reduce memory usage and improve performances.
	* Components can now have several input ports and output ports. This will allow building new features based on several existing features (for example using classifiers).
		.. warning::
			Custom features libraries need to upgrade to the new API. Yaafe v0.6 component API is no more compatible with v0.53 component API.

**v0.53**, 2010-06-28
	* Fix an annoying bug on MacOsX: cannot extract more than 1 feature when the output H5 file do not already exist.

**v0.52**, 2010-06-15
	* Improve compilation, now *Yaafe* supports Linux and MacOsX platforms
	* Improve error messages when failing to load extension

**v0.51**, 2010-05-10
	* Use CMake as compilation tool

**v0.5**, 2010-03-30
	* First public release
