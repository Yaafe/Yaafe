# -*- coding: utf-8 -*-
#
# Yaafe
#
# Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
# Télécom ParisTech / dept. TSI
#
# Author : Benoit Mathieu
#
# This file is part of Yaafe.
#
# Yaafe is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Yaafe is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from ctypes import c_int, pointer

from core import yaafecore as yc
from core import iterPtrList, iterPtrDict
from dataflow import DataFlow


class Engine(object):
    """
        A Engine object is in charge of processing computations defined in a
        :py:class:`DataFlow` object on given inputs.

        .. doctest::
            :options: +ELLIPSIS, +NORMALIZE_WHITESPACE

            >>> # Initialization
            >>> fp = FeaturePlan(sample_rate=16000)
            >>> fp.addFeature('mfcc: MFCC blockSize=512 stepSize=256')
            True
            >>> fp.addFeature('sr: SpectralRolloff blockSize=512 stepSize=256')
            True
            >>> fp.addFeature('sf: SpectralFlux blockSize=512 stepSize=256')
            True
            >>> df = fp.getDataFlow()
            >>> df2 = DataFlow()
            >>> len(str(df)) > 0
            True
            >>> df2.loads(str(df))
            True
            >>> str(df2) == str(df)
            True
            >>> engine = Engine()
            >>> engine.load(df)
            True
            >>> # get input metadata
            >>> eng_out = engine.getInputs()
            >>> sorted(eng_out.keys())
            ['audio']
            >>> sorted(eng_out['audio'].items())
            [('frameLength', 1), ('parameters', {...}),
             ('sampleRate', 16000.0), ('sampleStep', 1), ('size', 1)]
            >>> sorted(eng_out['audio']['parameters'].items())
            [('Resample', 'no'), ('SampleRate', '16000'),
             ('TimeLimit', '0.0s'), ('TimeStart', '0.0s'),
             ('TrimBeginningSilence', 'no')]
            >>>
            >>>
            >>> # get output metadata
            >>> eng_out = engine.getOutputs()
            >>> sorted(eng_out.items())
            [('mfcc', {...}), ('sf', {...}), ('sr', {...})]
            >>> sorted(eng_out['sr'].items())
            [('frameLength', 512), ('parameters', {...}),
             ('sampleRate', 16000.0), ('sampleStep', 256), ('size', 1)]
            >>> sorted(eng_out['sr']['parameters'].items())
            [('normalize', '-1'), ('resample', 'no'), ('samplerate', '16000'),
             ('version', '0.64'),
             ('yaafedefinition', 'SpectralRolloff blockSize=512 stepSize=256')]
            >>> # extract features from a numpy array
            >>> import numpy # needs numpy
            >>> audio = numpy.random.randn(1,1000000)
            >>> feats = engine.processAudio(audio)
            >>> feats['mfcc']
            array([[...]])
            >>> feats['sf']
            array([[...]])
            >>> feats['sr']
            array([[...]])

        It is also possible to extract features block per block:

        .. testsetup:: blocks

            from yaafelib import *
            import numpy
            fp = FeaturePlan(sample_rate=16000)
            fp.addFeature('mfcc: MFCC blockSize=512 stepSize=256')
            fp.addFeature('sr: SpectralRolloff blockSize=512 stepSize=256')
            fp.addFeature('sf: SpectralFlux blockSize=512 stepSize=256')
            engine = Engine()
            engine.load(fp.getDataFlow())

        .. testcode:: blocks

            # first reset the engine
            engine.reset()
            for i in range(1,10):
                # get your audio data
                audio = numpy.random.rand(1,32000)
                # write audio array on 'audio' input
                engine.writeInput('audio',audio)
                # process available data
                engine.process()
                # read available feature data
                feats = engine.readAllOutputs()
                # do what you want your feature data
            # do not forget to flush
            engine.flush()
            feats = engine.readAllOutputs() # read last data
            # do what you want your feature data

        When extracting features block per block, you should be aware of
        :ref:`Yaafe's engine internals <processingengine>`.


    """
    def __init__(self):
        self.ptr = yc.engine_create()

    def __del__(self):
        yc.engine_destroy(self.ptr)

    def load(self, dataflow):
        """
            Configure engine according to the given dataflow.

            :param dataflow: dataflow object or filename of a dataflow file.
            :type dataflow: :py:class:`DataFlow` or string
            :return: True on success, False on fail.
        """
        if type(dataflow) is str:
            df = DataFlow()
            if df.load(dataflow):
                dataflow = df
        if type(dataflow) is DataFlow:
            return yc.engine_load(self.ptr, dataflow.ptr) and True or False
        raise TypeError('dataflow parameter must be a DataFlow object or '
                        'dataflow filename !')

    def getInputs(self):
        """
            Get input metadata. Result format is the same as for
            :py:meth:`getOutputs` method, but the general case is
            that there is only one input named 'audio' and the sole
            relevant metadata are:

            :sampleRate: expected audio sampleRate
            :parameters: attached parameters

            Others fields should be set to 1.
        """
        res = {}
        iList = yc.engine_getInputList(self.ptr)
        for inputname in iterPtrList(iList):
            ptr = yc.engine_getInputInfos(self.ptr, inputname)
            infos = {}
            if ptr:
                infos['sampleRate'] = ptr.contents.sampleRate
                infos['sampleStep'] = ptr.contents.sampleStep
                infos['frameLength'] = ptr.contents.frameLength
                infos['size'] = ptr.contents.size
                infos['parameters'] = dict(
                    (k, v) for k, v in iterPtrDict(ptr.contents.parameters))
                yc.engine_freeIOInfos(ptr)
            res[inputname] = infos
        yc.engine_freeIOList(iList)
        return res

    def getOutputs(self):
        """
            Get output metadata. For each output feature, you get the following
            metadata:

            :sampleRate: audio analysis samplerate
            :sampleStep: Number of audio samples between consecutive feature
                         values
            :frameLength: Analysis frame size in number of audio samples
            :size: size the feature (or number of coefficients)
            :parameters: attached parameters.
        """
        res = {}
        oList = yc.engine_getOutputList(self.ptr)
        for outputname in iterPtrList(oList):
            ptr = yc.engine_getOutputInfos(self.ptr, outputname)
            infos = {}
            if ptr:
                infos['sampleRate'] = ptr.contents.sampleRate
                infos['sampleStep'] = ptr.contents.sampleStep
                infos['frameLength'] = ptr.contents.frameLength
                infos['size'] = ptr.contents.size
                infos['parameters'] = dict(
                    (k, v) for k, v in iterPtrDict(ptr.contents.parameters))
                yc.engine_freeIOInfos(ptr)
            res[outputname] = infos
        yc.engine_freeIOList(oList)
        return res

    def writeInput(self, name, data):
        """
            Write data on an input.

            :param name: input on which to write
            :type name: string
            :param data: data to write.
            :type data: numpy array

        """
        size = 1
        toks = len(data)
        if (len(data.shape) == 2):
            size = data.shape[0]
            toks = data.shape[1]
        elif (len(data.shape) > 2):
            print 'ERROR: data must be a 1-d or 2-d array !'
            return
        yc.engine_input_write(self.ptr, name, data, size, toks)

    def readOutput(self, name):
        """
            Read a specific output, and returns values as a numpy.array

            :param name: output name to read
            :type name: string
            :rtype: numpy.array
        """
        import numpy as np
        size = c_int(0)
        tokens = c_int(0)
        yc.engine_output_available(self.ptr, name, pointer(size),
                                   pointer(tokens))
        if tokens == 0:
            return None
        data = np.zeros((tokens.value, size.value))
        yc.engine_output_read(self.ptr, name, data, data.shape[1],
                              data.shape[0])
        return data

    def readAllOutputs(self):
        """
            Read all outputs.

            :return: dictionary with output name as key and numpy.array
                     as value.
        """
        res = {}
        oList = yc.engine_getOutputList(self.ptr)
        for o in iterPtrList(oList):
            res[o] = self.readOutput(o)
        return res

    def reset(self):
        """
            Reset engine. All buffers are cleared, and a new analysis
            can start.
        """
        yc.engine_reset(self.ptr)

    def process(self):
        """
            Process available data.
        """
        return yc.engine_process(self.ptr) and True or False

    def flush(self):
        """
            Process available data and flush all buffers so that
            all output data is available. Analysis is ended, the
            :py:meth:`reset` method must be called before any
            further calls to :py:meth:`writeInput`
            and :py:meth:`process`.
        """
        yc.engine_flush(self.ptr)

    def processAudio(self, data):
        """
            Convenient method to extract features from *data*.
            It successively calls :py:meth:`reset`, :py:meth:`writeInput`,
            :py:meth:`process`, :py:meth:`flush`, and returns output
            of :py:meth:`readAllOutputs`
        """
        self.reset()
        self.writeInput('audio', data)
        self.process()
        self.flush()
        return self.readAllOutputs()
