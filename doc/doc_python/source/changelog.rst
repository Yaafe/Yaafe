Changelog
=========
**v0.7**, 2014-07-14

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
