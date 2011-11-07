Quick Start, using ``yaafe.py``
===============================

Once *yaafe* is installed and environment is correctly configured, you can start extracting audio features with ``yaafe.py``.
*Yaafe* uses the YAAFE_PATH  environment variable to find audio features libraries.

First, some help is provided with the ``-h`` option::

	> yaafe.py -h

List available features and output formats
------------------------------------------

With the ``-l`` option you can list available audio features and output formats::

	> yaafe.py -l
	Available features:
	 - AmplitudeModulation
	 - AutoCorrelation
	 - ComplexDomainOnsetDetection
	 - Energy
	 - Envelope
	 - EnvelopeShapeStatistics
	 - Frames
	 - LPC
	 - LSF
	 - Loudness
	 - MFCC
	 - MagnitudeSpectrum
	 - OBSI
	 - OBSIR
	 - PerceptualSharpness
	 - PerceptualSpread
	 - SpectralCrestFactorPerBand
	 - SpectralDecrease
	 - SpectralFlatness
	 - SpectralFlatnessPerBand
	 - SpectralFlux
	 - SpectralRolloff
	 - SpectralShapeStatistics
	 - SpectralSlope
	 - SpectralVariation
	 - TemporalShapeStatistics
	 - ZCR
	Available feature transforms:
	 - AutoCorrelationPeaksIntegrator
	 - Cepstrum
	 - Derivate
	 - HistogramIntegrator
	 - SlopeIntegrator
	 - StatisticalIntegrator
	Available Output formats:
 	 - csv
 	 - h5
	 
Some of these features are not really features (like Frames or Envelope), but they are
intermediate representation used to compute other features.

*Feature transforms* are transformations that can be applied to features.

You can view a description of each feature (or output format) with the ``-d`` option::

	> yaafe.py -d MFCC
	
	Compute the Mel-frequencies cepstrum coefficients.
	
	Parameters are :
	- CepsIgnoreFirstCoeff (default=1): 0 means to keep the first cepstral coeffcient, 1 means to ignore it
	- CepsNbCoeffs (default=13): Number of cepstral coefficient to keep.
	- FFTWindow (default=Hanning): Weighting window to apply before fft. Hanning|Hamming|None
	- MelMaxFreq (default=6854.0): Maximum frequency of the mel filter bank
	- MelMinFreq (default=130.0): Minimum frequency of the mel filter bank
	- MelNbFilters (default=40): Number of mel filters
	- blockSize (default=1024): output frames size
	- stepSize (default=512): step between consecutive frames
	
	> yaafe.py -d csv
	
	[csv] Creates one CSV file per feature and input file.

	Parameters are:
	- Metadata (default=True): If 'True' then write metadata as comments at the beginning of the csv file. If 'False', do not write metadata
	- Precision (default=6): precision of output floating point number.
	

.. _feat-def-format:

Feature definition format
-------------------------

Syntax to define a feature to extract is::

	name: feature [param=value] [param=value] ... [> feature-transform [param=value] ... [> ...] ]

For example::

	mfcc: MFCC blockSize=1024 stepSize=1024 CepsNbCoeffs=11
	mfcc_d1: MFCC blockSize=1024 stepSize=1024 CepsNbCoeffs=11 > Derivate DOrder=1
	mfcc_d2: MFCC blockSize=1024 stepSize=1024 CepsNbCoeffs=11 > Derivate DOrder=2

*name* is used in output file to name the table that holds feature values. Parameters
are optional, they are set to default values if not specified.

Features extraction
-------------------

Extract one feature
^^^^^^^^^^^^^^^^^^^

To extract one feature, you can use::

	> yaafe -r 44100 -f "mfcc: MFCC blockSize=1024 stepSize=512" test.wav

``-f`` option defines the feature to compute, according to :ref:`feat-def-format`. You may provide
``-f`` option multiple times. ``-r`` option defines the expected sample rate. 

Extract several features
^^^^^^^^^^^^^^^^^^^^^^^^

You may define a feature extraction plan, which is a text file with one feature defined per line. For example::

	lx: Loudness 
	psp: PerceptualSpread
	psh: PerceptualSharpness
	ss: SpectralSlope
	sv: SpectralVariation
	sd: SpectralDecrease
	sf: SpectralFlatness
	sss: SpectralShapeStatistics
	mfcc: MFCC blockSize=512 stepSize=256 CepsNbCoeffs=11
	lpc: LPC LPCNbCoeffs=3
	obsi: OBSI
	obsir: OBSIR
	am: AmplitudeModulation blockSize=30720 stepSize=15360

To extract all features define in a feature extraction plan, use the ``-c`` option::

	> yaafe.py -c featureplan -r 32000 file1.wav

Extract over several audio files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You may pass several audio files as arguments to `yaafe.py` script, but you can also use 
the ``-i`` option to specify a text file that contains one filename per line. Each audio
file must have the same sample rate.

Samplerate conversion
^^^^^^^^^^^^^^^^^^^^^

*Yaafe* provides the possibility to automatically convert input audio samplerate to the desired
analysis samplerate. This is done using the `smarc <http://audio-smarc.sourceforge.net>`_ library, which is included
in *Yaafe* as an external library.

The default behavior is to *not* perform any samplerate conversion (audio files with different samplerate are ignored).
To enable input audio samplerate conversion, use the ``--resample`` option. 

Signal normalization
^^^^^^^^^^^^^^^^^^^^

*Yaafe* can normalize the input signal. The normalized signal has mean equal to 0 and maximum absolute
value equal to the value given by ``--normalize-max`` option.   

``-n``
	enable normalization.

``--normalize-max``
	maximum absolute value, default is 0.98.


Frames iteration and timestamps
-------------------------------

In this paragraph, all indices starts from 0. An array of size N contains elements from 0 to N-1. 

We assume that a feature value computed over a signal frame is associated with the time of the frame's center.
When computing a feature with frame size (*blockSize*) of **b** and step between frame (*stepSize*) of **s**, frame iteration operates as following:  

* first frame is centered on first signal sample (with **b**/2 zeros at left)
* following frames are centered on signal samples **s**, 2 * **s**, 3 * **s**, etc..
* last frame is centered in the last **s** signal samples. It may contains at most **b**/2 zeros at right.  

For example, extraction of MFCC with blockSize=1024 and stepSize=512 over a signal of 10000 samples will result in following frames::

	frame  0 : [  -512    511] -> centered on sample 0, padded with 0 on the left
	frame  1 : [     0   1023] -> centered on sample 512
	frame  2 : [   512   1535] -> centered on sample 1024
	frame  3 : [  1024   2047] -> centered on sample 1536
	...
	frame 18 : [  8704   9727] -> centered on sample 9216
	frame 19 : [  9216  10239] -> centered on sample 9728, padded with 0 on the right

This frame iteration ensures that all features with same *stepSize* parameter will always be aligned, even if they have different *blockSize* parameter.


.. _output-format:

Output format
-------------

You can select an output format using the ``-o`` option. Available output formats can be listed with the ``-l`` option.
CSV is the default format. Other output format depends on compilation options, for example HDF5. Each output format has
his own parameters, that you can list with the ``-d`` option. To set parameters, use the ``-p`` option. For example::

	> yaafe.py -c featureplan -r 44100 audio.wav -o csv -p Precision=8 -p Metadata=False

Metadata
^^^^^^^^

Each feature comes with the following metadata attached:

:sampleRate:        sampleRate of input audio file.
:blockSize:         frame size in number of sample.
:stepSize:          step between consecutive frames, in number of sample.
:version:           yaafe's version used to compute feature.
:inputfile:         input file name.
:normalize:         the normalization parameter (``--normalize-max`` value), or -1 if no normalization has been used.
:yaafedefinition:   yaafe feature definition.

CSV output format
^^^^^^^^^^^^^^^^^

::

	> yaafe.py -d csv

	[csv] Creates one CSV file per feature and input file.

	Parameters are:
	- Metadata (default=True): If 'True' then write metadata as comments at the beginning of the csv file. If 'False', do not write metadata
	- Precision (default=6): precision of output floating point number.
	
*Yaafe* outputs feature values in CSV files, creating one CSV file per features. Metadata are written in comments at the beginning to the files,
but can be ignored with the parameter ``Metadata=False``.
   
HDF5 output format
^^^^^^^^^^^^^^^^^^

::

	> yaafe.py -d h5
	
	[h5] Creates one H5 file per input file, containing one dataset per features.

	Parameters are:
	- Mode (default=update): 'overwrite' force overwrite already existing dataset, 'update' overwrite already existing dataset only if feature parameters have changed, 'create' fails if dataset already exists
	

*Yaafe* outputs feature values in a `HDF5 <http://www.hdfgroup.org/HDF5/>`_ file. 
HDF5 is a binary format designed for efficient storage of large amount of scientific data.
*Yaafe* creates one H5 file per input audio file, and stores each extracted feature in
a different dataset. Metadata are stored in dataset attributes.

If you're working with Matlab, *Yaafe* provides some Matlab scripts to load feature data from h5 files.

If you're working with Python, you can use the `h5py package <http://code.google.com/p/h5py/>`_ to manipulate feature data (other python packages also exists).

Output directory
^^^^^^^^^^^^^^^^

``-b`` option can specify a base directory for output files.

