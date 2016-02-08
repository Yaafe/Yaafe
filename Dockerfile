FROM debian:jessie
MAINTAINER Thomas Fillon <fillon@fillon.com>

# Install Debian dependencies
RUN apt-get update
RUN apt-get install -y cmake cmake-curses-gui libargtable2-0 libargtable2-dev libsndfile1 libsndfile1-dev libmpg123-0 libmpg123-dev libfftw3-3 libfftw3-dev liblapack-dev libhdf5-serial-dev bzip2


# Install conda in /opt/miniconda
ENV PATH /opt/miniconda/bin:$PATH
RUN wget http://repo.continuum.io/miniconda/Miniconda-latest-Linux-x86_64.sh -O miniconda.sh && \
    bash miniconda.sh -b -p /opt/miniconda && \
    rm miniconda.sh && \
    hash -r && \
    conda config --set always_yes yes --set changeps1 yes && \
    conda update -q conda && \
    conda install numpy 

RUN mkdir /srv/src
RUN mkdir /srv/src/yaafe
WORKDIR /srv/src/yaafe

COPY . /srv/src/yaafe

RUN mkdir build && \
    cd build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr/local \
          -DCMAKE_INSTALL_PYTHON_PACKAGES=/usr/local/lib/python2.7/dist-packages \
          -DCMAKE_INSTALL_YAAFE_EXTENSIONS=/usr/local/lib/python2.7/dist-packages \
          -DWITH_FFTW3=ON \
          -DWITH_HDF5=ON \
          -DWITH_LAPACK=ON \
          -DWITH_MATLAB_MEX=OFF \
          -DWITH_MPG123=ON \
          -DWITH_SNDFILE=ON \
          .. && \
    make && \
    make install && \
    cd ../.. && \
    rm -rf Yaafe-0.65.1 v0.65.1.tar.gz

ENV LD_LIBRARY_PATH /usr/local/lib
ENV YAAFE_PATH /usr/local/lib/python2.7/dist-packages

CMD ["/usr/local/bin/yaafe", "--help"]