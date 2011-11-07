# -*- coding: ISO-8859-1 -*-
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

import core as yaafecore
from dataflow import DataFlow
from audiofeature import AudioFeatureFactory
import os.path

class FeaturePlan(object):
    """
        FeaturePlan is a collection of features to extract, configured for a
        specific sample rate.
        
        :param sample_rate: analysis samplerate
        :param normalize: signal maximum normalization, in ]0,1], or `None` to skip normalization.
        
        This collection can be load from a file using the :py:meth:`loadFeaturePlan` method,
        or built by adding features with the :py:meth:`addFeature` method.
        
        Then, the :py:meth:`getDataFlow` method retrieve the corresponding :py:class:`DataFlow` object.
        
        .. doctest::
        
            >>> fp = FeaturePlan(sample_rate=16000)
            >>> fp.addFeature('mfcc: MFCC blockSize=512 stepSize=256')
            True
            >>> fp.addFeature('mfcc_d1: MFCC blockSize=512 stepSize=256 > Derivate DOrder=1')
            True
            >>> fp.addFeature('mfcc_d2: MFCC blockSize=512 stepSize=256 > Derivate DOrder=2')
            True
            >>> df = fp.getDataFlow()
            >>> df.display()
            ...        
    """

    def __init__(self,sample_rate=44100,normalize=None,resample=False):
        if type(normalize)==int:
            normalize = '%i'%normalize
        elif type(normalize)==float:
            normalize = '%f'%normalize
        elif normalize and type(normalize)!=str:
            normalize = str(normalize)
        self.features = {}
        self.resample=resample
        self.sample_rate = sample_rate
        self.audio_params = {'SampleRate': str(sample_rate), 'Resample' : 'yes' if resample else 'no'}
        if normalize:
            self.audio_params['RemoveMean'] = 'yes'
            self.audio_params['ScaleMax'] = normalize
        self.out_attrs = {'normalize': normalize or '-1',
                          'version': yaafecore.getYaafeVersion(),
                          'samplerate': str(sample_rate),
                          'resample': 'yes' if resample else 'no'}
        self.dataflow = DataFlow()
    
    def addFeature(self, definition):
        """
            Add a feature defined according the :ref:`feature definition syntax <featplan>`.
            
            :param definition: feature definition
            :type definition: string
            :rtype: True on success, False on fail.
        """
        data = definition.split(':')
        if not len(data)==2:
            print 'Syntax error in "%s"'%definition
            return False
        name,featdef = data
        dataflow = DataFlow()
        inputNode = dataflow.createInput('audio',self.audio_params)
        if featdef.strip():
            for s in featdef.split('>'):
                s = s.strip()
                bb = s.split(' ')
                feat = AudioFeatureFactory.get_feature(bb[0])
                if not feat:
                    return False
                params = {}
                for d in bb[1:]:
                    if len(d)==0:
                        continue
                    if not '=' in d:
                        print 'Invalid feature parameter "%s"'%d
                        return False
                    dd = d.split('=')
                    if not len(dd)==2:
                        print 'Syntax error in feature parameter "%s"'%d
                        return False
                    params[dd[0]] = dd[1]
                dataflow.append(feat.get_dataflow(params, self.sample_rate))
        fNode = dataflow.finalNodes()[0]
        feat_attrs = self.out_attrs.copy()
        feat_attrs['yaafedefinition'] = featdef.strip()
        outNode = dataflow.createOutput(name,feat_attrs)
        dataflow.link(fNode,'',outNode,'')
        self.dataflow.merge(dataflow)
        return True

    def loadFeaturePlan(self,filename):
        """
            Loads feature extraction plan from a file. The file must be a text file,
            where each line defines a feature (see :ref:`feature definition syntax <feat-def-format>`).
            
            :rtype: True on success, False on fail. 
        """
        fin = open(filename,'r')
        for line in fin:
            if line.startswith('#'):
                continue
            line = line.strip()
            if line:
                if not self.addFeature(line):
                    return False
        fin.close()
        return True
        
    def getDataFlow(self):
        """
            Get the :py:class:`DataFlow` object representing how to extract defined features.
            
            :rtype: DataFlow
        """
        return self.dataflow

