Other *Yaafe* tools
===================

Matlab interaction
------------------

If you enabled the WITH_MATLAB_MEX option, you can extract features using the ``Yaafe`` matlab class.

The Matlab Yaafe engine needs to be configured with a :ref:`dataflow file <get-dataflow-file>`.
Once you get the dataflow file corresponding to the features to extract and the analysis sample rate,
you can prepare the Matlab Yaafe engine::


     >> yaafe = Yaafe();
     >> yaafe.load('yaflow')
     1

'yaflow' is the Dataflow file. the 'load' method returns 1 if success or 0 if failed.

Then, the Matlab Yaafe is ready to extract features::

     >> signal = rand(1,100000);
     >> feats1 = yaafe.process(signal);
     >> feats2 = yaafe.processFile('song.wav');
     >> feats1

     feats1 =

           mfcc: [1x1 struct]
        mfcc_d1: [1x1 struct]
        mfcc_d2: [1x1 struct]
             sf: [1x1 struct]
             sr: [1x1 struct]

Once Dataflow file is loaded, you can call the 'process' and 'processFile' methods
as many times as you want. The output is a struct where each fields holds an audio
features with the following metadata:

:name: the feature name has defined in the Dataflow file
:size: size of the feature
:sampleRate: analysis sample rate
:sampleStep: number of sample between consecutive analysis windows
:frameLength: length of analysis window
:data: feature values

.. note::
	If you prefer to manipulate a feature list, you can use the ``struct2cell`` Matlab function::

		>> feats = struct2cell(feats);
		>> feats

		feats =

		    [1x1 struct]
		    [1x1 struct]
		    [1x1 struct]
		    [1x1 struct]
		    [1x1 struct]




Python interaction
------------------

*Yaafe* python bindings allow to easily extract features from Python with a great flexibility.
The first step is always to build the |Dataflow| object corresponding to the audio features
to extract (for example using a |FeaturePlan| object), and configure an |Engine|.

.. testsetup::

	import doctestenv
	audiofile = doctestenv.audiofile
	dataflow_file = doctestenv.dataflow_file

.. doctest::

	>>> from yaafelib import *
	>>>
	>>> # Build a DataFlow object using FeaturePlan
	>>> fp = FeaturePlan(sample_rate=16000)
	>>> fp.addFeature('mfcc: MFCC blockSize=512 stepSize=256')
	True
	>>> fp.addFeature('mfcc_d1: MFCC blockSize=512 stepSize=256 > Derivate DOrder=1')
	True
	>>> fp.addFeature('mfcc_d2: MFCC blockSize=512 stepSize=256 > Derivate DOrder=2')
	True
	>>> df = fp.getDataFlow()
	>>>
	>>> # or load a DataFlow from dataflow file.
	>>> df = DataFlow()
	>>> df.load(dataflow_file)
	True
	>>>
	>>> # configure an Engine
	>>> engine = Engine()
	>>> engine.load(df)
	True
	>>> # extract features from an audio file using AudioFileProcessor
	>>> afp = AudioFileProcessor()
	>>> afp.processFile(engine,audiofile)
	0
	>>> feats = engine.readAllOutputs()
	>>> # and play with your features
	>>>
	>>> # extract features from an audio file and write results to csv files
	>>> afp.setOutputFormat('csv','output',{'Precision':'8'})
	True
	>>> afp.processFile(engine,audiofile)
	0
	>>> # this creates output/myaudio.wav.mfcc.csv,
	>>> #              output/myaudio.wav.mfcc_d1.csv and
	>>> #              output/myaudio.wav.mfcc_d2.csv files.
	>>>
	>>> # extract features from a numpy array
	>>> import numpy
	>>> audio = numpy.random.randn(1,100000)
	>>> feats = engine.processAudio(audio)
	>>> # and play with your features

.. seealso::

	:py:mod:`yaafelib`
		Documentation of yaafelib module.

	|FeaturePlan|
		More on manipulation feature plans

	|Engine|
		Details about metadata and feature extraction

	|AudioFileProcessor|
		Extraction features directly from audio files, writing output to files


``yaafe-engine`` program
------------------------

The ``yaafe-engine`` program is a C++ program to process a :ref:`dataflow file <get-dataflow-file>`
on a list of audio files. It can produce same outputs as the ``yaafe`` script. This program is
usefull if you need to process feature extraction without any dependency to Python.

.. |DataFlow| replace:: :py:class:`DataFlow <yaafelib.DataFlow>`
.. |Engine| replace:: :py:class:`Engine <yaafelib.Engine>`
.. |FeaturePlan| replace:: :py:class:`FeaturePlan <yaafelib.FeaturePlan>`
.. |AudioFileProcessor| replace:: :py:class:`AudioFileProcessor <yaafelib.AudioFileProcessor>`