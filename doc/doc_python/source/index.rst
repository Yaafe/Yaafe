.. Yaafe documentation master file, created by
   sphinx-quickstart on Thu Oct 29 14:34:10 2009.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Yaafe - audio features extraction
=================================

*Yaafe* is an audio features extraction toolbox.

**Easy to use**
	The user can easily declare the features to extract and their parameters in a text file.
	Features can be extracted in a batch mode, writing CSV or H5 files. The user can also extract
	features with Python or Matlab.

**Efficient**
	*Yaafe* automatically identifies common intermediate representations (spectrum, envelope, autocorrelation, ...) and computes them only once.
	Extraction is processed block per block so that arbitrarily long files can be processed, and memory occupation is low.

If you wonder about the *Yaafe* acronym, it's just *Yet Another Audio Feature Extractor*.

Project's status
----------------

.. highlights::
	Current version is Yaafe |version|,  released on 2011-11-07.

*Yaafe* may evolve in future versions, but current code is pretty stable and feature computation is already reliable.
*Yaafe* is already used in some Music Information Retrieval systems.

*Yaafe* provides:

* a great collection of classical audio features, with transformations and temporal integration (see :doc:`Available features documentation </features>`).
* reading of WAV, OGG, MP3 (and others) audio file formats.
* CSV and HDF5 output file formats
* Python and Matlab bindings to extract features straight within your favorite environment.
* automatic input samplerate conversion, using `smarc <http://audio-smarc.sourceforge.net>`_.
* an extensible framework that allow you to create your own feature library.
* a C++ API to efficiently integrate *Yaafe* in your project

Download
--------

*Yaafe*'s source code is released under the terms of the GNU LGPLv3 License. *Yaafe* source code compiles on **Linux**, **MacOsX** and **Windows** platforms.

Download current release on GitHub: `yaafe zip <https://github.com/Yaafe/Yaafe/archive/master.zip>`_, see :doc:`changelog <changelog>`.

View all old releases: `Sourceforge project's website <https://sourceforge.net/projects/yaafe/>`_

Documentation
-------------

.. toctree::
   :hidden:

   manual
   features
   changelog

* :doc:`User Manual </manual>`
* :doc:`Available features documentation </features>`

Support
-------

To get help with *Yaafe*, please use `yaafe-users@lists.sourceforge.net <https://lists.sourceforge.net/lists/listinfo/yaafe-users>`_ mailing list.
As project is still in progress, any feedback is welcome !

Cite Yaafe
----------

If you want to cite Yaafe in a publication, please use the following reference:

	*YAAFE, an Easy to Use and Efficient Audio Feature Extraction Software*, B.Mathieu, S.Essid, T.Fillon, J.Prado, G.Richard, proceedings of the 11th ISMIR conference, Utrecht, Netherlands, 2010.

Credits
-------

*Yaafe* is developed at `Telecom Paristech / AAO Team <http://www.tsi.telecom-paristech.fr/aao/>`_. It uses several great open-source projects
like `Eigen <http://eigen.tuxfamily.org/>`_, `Smarc <http://audio-smarc.sourceforge.net>`_, `libsndfile <http://www.mega-nerd.com/libsndfile/>`_, `mpg123 <http://www.mpg123.de/>`_,
`HDF5 <http://www.hdfgroup.org/HDF5/>`_.

