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

"""
Main Yaafe module, providing classes to extract features with Yaafe.

.. testsetup:: *

    from yaafelib import *

"""

import os
import sys
from core import *
from dataflow import DataFlow
from audiofeature import AudioFeatureFactory, AudioFeature, check_dataflow_params, dataflow_safe_append
from featureplan import FeaturePlan
from audiofileprocessor import AudioFileProcessor
from engine import Engine


__all__ = ['loadComponentLibrary',
           'isComponentAvailable',
           'getComponentList',
           'getComponentParameters',
           'getOutputFormatList',
           'getOutputFormatDescription',
           'getOutputFormatParameters',
           'setPreferedDataBlockSize',
           'setVerbose',
           'readH5FeatureDescriptions',
           'DataFlow',
           'AudioFeatureFactory',
           'AudioFeature',
           'check_dataflow_params',
           'dataflow_safe_append',
           'FeaturePlan',
           'AudioFileProcessor',
           'Engine']

def loadPlugins():
    # load available plugins
    if os.getenv('YAAFE_PATH'):
        if not os.getenv('YAAFE_PATH') in sys.path:
            sys.path.append(os.getenv('YAAFE_PATH'))
        for f in os.listdir(os.getenv('YAAFE_PATH')):
            parts = f.split('.')
            if parts[-1]=='py':
                extension = '.'.join(parts[0:-1])
                try:
                    __import__(extension,globals(),locals(),[],-1)
                except ImportError,e:
                    print 'ERROR: cannot load yaafe extension %s !'%extension
                    print e

loadPlugins()