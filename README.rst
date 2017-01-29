=======
 Yaafe
=======

Yet Another Audio Feature Extractor
+++++++++++++++++++++++++++++++++++


Build status
============
- Branch **master** : |travis_master|
- Branch **dev** : |travis_dev|
- Anaconda : |anaconda_build|
  
.. |travis_master| image:: https://travis-ci.org/Yaafe/Yaafe.svg?branch=master
    :target: https://travis-ci.org/Yaafe/Yaafe

.. |travis_dev| image:: https://travis-ci.org/Yaafe/Yaafe.svg?branch=dev
    :target: https://travis-ci.orgYaafe/Yaafe

.. |anaconda_build| image:: https://anaconda.org/yaafe/yaafe/badges/installer/conda.svg
   :target: https://anaconda.org/yaafe/yaafe/builds

			
Install
=======

Conda
-----

.. |conda| image:: https://anaconda.org/yaafe/yaafe/badges/installer/conda.svg
   :target: https://conda.anaconda.org/yaafe

|conda|
	    
Yaafe can be easily install with `conda <http://conda.pydata.org>`_. To install this package with conda run::
    
  conda install -c https://conda.anaconda.org/yaafe yaafe  


Docker
------

.. |docker| image:: https://badge.imagelayers.io/yaafe/yaafe:latest.svg
   :target: https://imagelayers.io/?images=yaafe/yaafe:latest

|docker|

Yaafe can also be install through `Docker <https://www.docker.com/>`_. 

Get the official *Yaafe* image from `Yaafe repository on Docker Hub <https://hub.docker.com/r/yaafe/yaafe/>`_::
  
  docker pull yaafe/yaafe
  docker tag yaafe yaafe/yaafe

or build it from the sources directory::
    
    docker build --tag=yaafe .

Following both of the method above, you then have a docker image tagged as *yaafe*.
The ``yaafe`` command is the entrypoint for that docker image so you can run 'yaafe' from the command line through `docker run yaafe`. For example::

    docker run -it yaafe --help
    docker run -it yaafe --volume=$(pwd):/wd  --workdir=/wd -c resources/featureplan -r 16000 --resample resources/yaafe_check.wav

On Linux, it can be usefull to create an ``alias`` for it::

  alias yaafe='docker run -it --rm=true --volume=$(pwd):/wd  --workdir=/wd yaafe'

You could then simply run::

  yaafe --help
  yaafe -l
    
et voilà !

If you need to set the docker user as the current user on the host, you could try to run docker with the  ``-u $(id -u):$(id -g)`` option ::
  
    docker run -it --rm=true --volume=$(pwd):/wd  --workdir=/wd -u $(id -u):$(id -g) yaafe -c resources/featureplan -o h5 -r 16000 --resample resources/yaafe_check.wav

Last but not least, the entry-point for the *yaafe* docker image is smart :

- every command that start with a dash ``-`` will be pass as options to the ``yaafe`` command inside the docker container
- every command that does not start with a dash will be treated as a regular command. For example::
      
    docker run -it yaafe /bin/bash

will give you access to a bash terminal inside the docker. And ::

    docker run -it yaafe yaafe-engine --help

will launch the  ``yaafe-engine` batch processing tool.

    

From sources
------------

*Yaafe* source code should compile on *linux* and *MacOsX* platform, and uses CMake as compilation tool.
Yaafe requires third-party libraries to enable specific features. Some of these libraries may already be available on your system.

The `argtable <http://argtable.sourceforge.net/>`_ library is required.

Depending on optional features you want to use, other librairies may be used:

* `libsndfile <http://www.mega-nerd.com/libsndfile/>`_: enable reading WAV files format (highly recommanded)
* `libmpg123 <http://www.mpg123.de/api/>`_: enable reading MP3 audio files
* `HDF5 <http://www.hdfgroup.org/HDF5/>`_ >= 1.8: enable H5 output format
* `liblapack <http://www.netlib.org/lapack/>`_: enable some audio features (LSF)
* `FFTW3 <http://www.fftw.org/>`_: use FFTW instead of Eigen for FFT computations (pay attention to licensing issues when linking with the GPL FFTW3 library).

To use the *yaafe* script you need Python >= 2.5, and the numpy package.

Once previous libraries are installed (some may have been locally installed in <lib-path>),
you can compile with the following steps: ::

 git submodule init  # to prepare Eigen and fmemopen code under the externals directory
 git submodule update # if you used git clone --recursive, you don't need those 2 lines
 mkdir build
 cd build
 ccmake -DCMAKE_PREFIX_PATH=<lib-path> -DCMAKE_INSTALL_PREFIX=<install-path> ..
 make
 make install

Several options can control Yaafe compilation. More details can be found at:
	http://yaafe.sourceforge.net/manual/install.html


Environment
~~~~~~~~~~~

To easily use Yaafe, you should set the following environment vars::

 export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEST_DIR/lib

On MacOSX replace ``LD_LIBRARY_PATH`` by ``DYLD_FALLBACK_LIBRARY_PATH``

The output of ``make install`` should give you the INSTALL_DIR path (defaults to ``/usr/local``). The python files are installed in ``/usr/local/lib/python2.7/site-packages``, which is *not* by default in the python path on MacOSX.
The consequence is the error ``ERROR: cannot load yaafe packages:  No module named yaafelib``.
There are 3 ways to solve this problem :

* The simplest way is to add the line ``sys.path.append("/usr/local/lib/python2.7/site-packages")`` in ``/usr/local/bin/yaafe`` after ``from optparse import OptionParser``, but it won't let you use the yaafelib unless you add this line (and import sys) before each import of yaafelib.

* You can use ``export PYTHONPATH=/usr/local/lib/python2.7/site-packages:$PYTHONPATH`` or add it to your ~/.bash_profile, but this will affect other versions of python.

* You can move the files to a site-package folder that is in your PYTHONPATH::
    
 - if you use Python 2 :

	mv /usr/local/lib/python2.7/site-packages/yaafefeatures.py /usr/local/lib/python2.7/site-packages/yaafelib `python -c 'import sys, re ; print next(i for i in sys.path if re.match(".*site-packages$", i))'`
	
 - if you use Python 3:
    
    mv /usr/local/lib/python2.7/site-packages/yaafefeatures.py /usr/local/lib/python2.7/site-packages/yaafelib `python3 -c 'import sys, re ; print(next(i for i in sys.path if re.match(".*site-packages$", i)))'`
    

If you use Matlab, you can set your MATLABPATH var::

 export MATLABPATH=$MATLABPATH:$DEST_DIR/matlab


Documentation
=============

Documentation is also available online: http://yaafe.github.io/Yaafe/ (mirror:  http://yaafe.sourceforge.net/)

To build documentation, you need Sphinx.
Before building documentation, you should set your environment correctly so that sphinx builds documentation
with automatic features documentation.

To build documentation, just run ``make doc_python`` in the ``build`` directory. Documentation is built in ``doc/doc_python/html``.

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

Yaafe was first developed at `Telecom Paristech / AAO Team <http://www.tsi.telecom-paristech.fr/aao/en/>`_. It uses several great open-source projects like `Eigen <http://eigen.tuxfamily.org/>`_, `Smarc <http://audio-smarc.sourceforge.net/>`_, `libsndfile <http://www.mega-nerd.com/libsndfile/>`_, `mpg123 <http://www.mpg123.de/>`_, `HDF5 <http://www.hdfgroup.org/HDF5/>`_.

If you want to cite Yaafe in a publication, please see `CITATION <CITATION.rst>`_.
