.. Smarc documentation master file, created by
   sphinx-quickstart on Wed Nov  4 16:07:49 2009.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Smarc audio rate converter
==========================

*Smarc* is a **fast** and **high quality** audio rate converter. It allows conversion between any samplerate.
*Smarc* is a command-line program and a C library to be integrated in other applications.

Download
--------

.. highlights::
    Download current release: `smarc 0.3 <https://sourceforge.net/projects/audio-smarc/files/>`_

*Smarc* source code is release under the terms of the **GNU LGPLv3 License**. *Smarc* source code is known to compile on Linux, MacOsX, Windows/mingw platforms.

Features
--------

* signal rate conversion between any sample rates.
* optimized for conversion between standard audio sample rates.
* command-line tool to convert audio files
* C library to integrate *smarc* converter into other applications.
* Initially designed to convert audio, but also any 1-D signals such as EEG, seismic signals, etc.
* Convert sample rate using polyphase decimation and interpolation filters. This method guarantees high-quality for upsampling and downsampling.
* Internal filters can be tuned to emphasize speed or quality.


Documentation
-------------

.. toctree::
   :maxdepth: 2
   
   manualsmarc
   integratesmarc


* `Details on smarc internals <http://sourceforge.net/projects/audio-smarc/files/smarc_internals.pdf/download>`_

Credits
-------

.. image:: _static/logo_tecomptech_85949.gif 
	:align: left
	:alt: Telecom Paristech
	:target: http://www.tsi.telecom-paristech.fr/aao/en/

*Smarc* is developed at Télécom ParisTech, France.

.. raw:: html

	<br style="clear: left"/>

