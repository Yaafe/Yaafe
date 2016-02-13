FROM debian:jessie
MAINTAINER Thomas Fillon <thomas@parisson.com>

# Install Debian dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    cmake-curses-gui \ 
    libargtable2-0 \ 
    libargtable2-dev \ 
    libsndfile1 libsndfile1-dev \ 
    libmpg123-0 libmpg123-dev \ 
    libfftw3-3 libfftw3-dev \ 
    liblapack-dev \ 
    libhdf5-dev \ 
    libeigen3-dev \ 
    bzip2 \ 
    wget \ 
    gcc g++

# Install conda in /opt/miniconda
ENV PATH /opt/miniconda/bin:$PATH
RUN wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh && \
    bash miniconda.sh -b -p /opt/miniconda && \
    rm miniconda.sh && \
    hash -r && \
    conda config --set always_yes yes --set changeps1 yes && \
    conda update -q conda
RUN conda install numpy sphinx

RUN mkdir /srv/src
RUN mkdir /srv/src/yaafe
WORKDIR /srv/src/yaafe

COPY . /srv/src/yaafe

RUN mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/opt/miniconda/ \
          -DCMAKE_INSTALL_PYTHON_PACKAGES=/opt/miniconda/lib/python2.7 \
          -DWITH_FFTW3=ON \
	  -DHDF5_ROOT=/usr/lib/x86_64-linux-gnu/hdf5/serial/ \
          -DWITH_HDF5=ON \
          -DWITH_LAPACK=ON \
          -DWITH_MATLAB_MEX=OFF \
          -DWITH_MPG123=ON \
          -DWITH_SNDFILE=ON \
          .. && \
    make && \
    make install && \
    cd ../.. 

#ENV LD_LIBRARY_PATH /opt/miniconda/lib/

COPY ./docker-entrypoint.sh /

ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["--help"]