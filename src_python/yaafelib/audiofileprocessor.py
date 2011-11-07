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

from core import yaafecore as yc
from core import loadComponentLibrary
from core import iterPtrList, iterPtrDict
from ctypes import c_char_p

class AudioFileProcessor(object):
    """
        A AudioFileProcessor object allow to extract features from
        audio files, and possibly write output features into files.
        
        It must be provided with a configured :py:class:`Engine`.
        
        Here is how to extract features from audio files and get it
        as numpy arrays:
        
        .. testsetup:: afp
        
            import doctestenv
            audiofile = doctestenv.audiofile
            dataflow_file = doctestenv.dataflow_file
            
            
        .. doctest:: afp
        
            >>> # configure your engine
            >>> engine = Engine()
            >>> engine.load(dataflow_file) 
            True
            >>> # create your AudioFileProcessor 
            >>> afp = AudioFileProcessor()
            >>> # leave output format to None
            >>> afp.processFile(engine,audiofile)
            0
            >>> # retrieve features from engine
            >>> feats = engine.readAllOutputs()
            >>> # do what you want with your feature data
            >>> feats['mfcc']
            array([[...]])
        
        To write features directly to output files, just set an output
        format with the :py:meth:`setOutputFormat` method.
        
    """
    
    _YAAFE_IO_LOADED = False
    
    def __init__(self):
        self.ptr = yc.audiofileprocessor_create()
    def __del__(self):
        yc.audiofileprocessor_destroy(self.ptr)
    
    def setOutputFormat(self,format, outDir, params):
        """
            Set output format.
            
            :param format: format to set
            :type format: string
            :param outDir: base output directory for output files
            :type outDir: string
            :param params: format parameters
            :type params: dict
            :return: True if ok, False if format does not exists.
        """
        tmp = ((c_char_p*2)*(len(params)+1))() 
        tmp[:-1] = [(c_char_p*2)(c_char_p(k),c_char_p(v)) for k,v in params.iteritems()]
        return yc.audiofileprocessor_setOutputFormat(self.ptr,format, outDir, tmp) and True or False
    
    def processFile(self,engine,filename):
        """
            Extract features from the given file using the given engine.
            
            If an output format has been set, then output files will be written, else
            output feature data can be read using engine's :py:meth:`Engine.readOutput`
            or :py:meth:`Engine.readAllOutputs` methods. 

            :param engine: engine to use for feature extraction. It must already have been configured.
            :type engine: :py:class:`Engine`
            :param filename: audio file to process
            :type filename: string
            :return: 0 on success, negative value on fail
        """
        if not AudioFileProcessor._YAAFE_IO_LOADED:
            AudioFileProcessor._YAAFE_IO_LOADED = (loadComponentLibrary('yaafe-io')==0)
        return yc.audiofileprocessor_processFile(self.ptr,engine.ptr,filename)

