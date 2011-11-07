Install
=======

*Smarc* source code should compile on *linux*, *MaxOsX* and *Windows (mingw)* platforms.
The smarc library can be compiled without any dependency to any thirdparty library.
The smarc program needs the following thirdparty LGPL libraries:

* `argtable <http://argtable.sourceforge.net/>`_ 
* `libsndfile <http://www.mega-nerd.com/libsndfile/>`_

To compile smarc, once previous libraries are installed:

 1. Edit Makefile to match your needs (in particular, location of your libraries)
 2. make
 

Documentation
=============

Documentation is available online: http://audio-smarc.sourceforge.net/

To build documentation, you need Sphinx: http://sphinx.pocoo.org/
Once Sphinx is installed, go to the doc/ directory, and type 'make html'.

License
=======

Smarc is released under the version 3 of the GNU Lesser General Public License. Read COPYING and COPYING.LESSER for
more details.

Support
=======

To get help with Smarc, contact the authors:
* benoit.mathieu@telecom-paristech.fr
* jacques.prado@telecom-paristech.fr
