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

from ctypes import *
from itertools import count

# load C library
yaafecore = cdll.LoadLibrary('libyaafecore.so')

##################################################
# Convenient functions to parse C array
##################################################

def iterPtrList(plist):
    """ usefull function """
    for i in count(0):
        if not plist[i]:
            raise StopIteration
        yield plist[i]

def iterPtrDict(plist):
    for i in count(0):
        if not plist[i][0]:
            raise StopIteration
        yield plist[i][0], plist[i][1]

##################################################
# Core C functions
##################################################

yaafecore.destroyFactory.restype = None

yaafecore.getYaafeVersion.restype = c_char_p
yaafecore.getYaafeVersion.argtypes = []
yaafecore.getComponentList.restype = POINTER(c_char_p)
yaafecore.getComponentList.argtypes = []
yaafecore.getOutputFormatList.restype = POINTER(c_char_p)
yaafecore.getOutputFormatList.argtypes = []
yaafecore.freeComponentList.restype = None
yaafecore.freeComponentList.argtypes = [POINTER(c_char_p)]
yaafecore.getOutputFormatDescription.restype = c_char_p
yaafecore.getOutputFormatDescription.argtypes = [c_char_p]
yaafecore.freeOutputFormatDescription.restype = None
yaafecore.freeOutputFormatDescription.argtypes = [c_char_p]
yaafecore.setPreferedDataBlockSize.restype = None
yaafecore.setVerbose.restype = None
yaafecore.setVerbose.argtypes = [c_int]

class ComponentParameters(Structure):
    _fields_ = [("identifier",c_char_p),
                ("defaultValue",c_char_p),
                ("description",c_char_p)]

yaafecore.getComponentParameters.restype = POINTER(POINTER(ComponentParameters))
yaafecore.getComponentParameters.argtypes = [c_char_p]
yaafecore.getOutputFormatParameters.restype = POINTER(POINTER(ComponentParameters))
yaafecore.getOutputFormatParameters.argtypes = [c_char_p]

class H5FeatureDescription(Structure):
    _fields_ = [("name",c_char_p),
                ("dim",c_int),
                ("nbframes",c_int),
                ("sampleRate",c_double),
                ("blockSize",c_int),
                ("stepSize",c_int),
                ("attrs",POINTER(c_char_p*2))]

yaafecore.readH5FeatureDescriptions.restype = POINTER(POINTER(H5FeatureDescription))
yaafecore.readH5FeatureDescriptions.argtypes = [c_char_p]
yaafecore.freeH5FeatureDescriptions.restype = None
yaafecore.freeH5FeatureDescriptions.argtypes = [POINTER(POINTER(H5FeatureDescription))]

def destroyFactory():
    yaafecore.destroyFactory()

def getYaafeVersion():
    return yaafecore.getYaafeVersion()

def loadComponentLibrary(name):
    return yaafecore.loadComponentLibrary(name)

def isComponentAvailable(name):
    return True if yaafecore.isComponentAvailable(name) else False

def getComponentList():
    ptr = yaafecore.getComponentList()
    res = [p for p in iterPtrList(ptr)]
    yaafecore.freeComponentList(ptr)
    return res

def getOutputFormatList():
    ptr = yaafecore.getOutputFormatList()
    res = [p for p in iterPtrList(ptr)]
    yaafecore.freeComponentList(ptr)
    return res

def getOutputFormatDescription(name):
    ptr = yaafecore.getOutputFormatDescription(name)
    res = ptr
    yaafecore.freeOutputFormatDescription(ptr)
    return res

def getComponentParameters(name):
    ptr = yaafecore.getComponentParameters(name)
    res = [(p.contents.identifier,p.contents.defaultValue,p.contents.description) for p in iterPtrList(ptr)]
    yaafecore.freeComponentParameters(ptr)
    return res

def getOutputFormatParameters(name):
    ptr = yaafecore.getOutputFormatParameters(name)
    res = [(p.contents.identifier,p.contents.defaultValue,p.contents.description) for p in iterPtrList(ptr)]
    yaafecore.freeComponentParameters(ptr)
    return res

def setPreferedDataBlockSize(size):
    yaafecore.setPreferedDataBlockSize(size)
    
def setVerbose(flag):
    yaafecore.setVerbose(flag and 1 or 0)

def readH5FeatureDescriptions(filename):
    out = yaafecore.readH5FeatureDescriptions(filename)
    res = {}
    if out:
        for featDesc in iterPtrList(out):
            resFeat = {}
            resFeat['dim'] = featDesc.contents.dim
            resFeat['nbframes'] = featDesc.contents.nbframes
            resFeat['sampleRate'] = featDesc.contents.sampleRate
            resFeat['blockSize'] = featDesc.contents.blockSize
            resFeat['stepSize'] = featDesc.contents.stepSize
            resFeat['attrs'] = dict((k,v) for k,v in iterPtrDict(featDesc.contents.attrs))
            res[featDesc.contents.name] = resFeat
    yaafecore.freeH5FeatureDescriptions(out)
    return res

##################################################
# DataFlow functions
##################################################

yaafecore.dataflow_create.restype = c_void_p
yaafecore.dataflow_destroy.restype = None
yaafecore.dataflow_destroy.argtypes = [c_void_p]
yaafecore.dataflow_load.argtypes = [c_void_p,c_char_p]
yaafecore.dataflow_save.restype = None
yaafecore.dataflow_save.argtypes = [c_void_p,c_char_p]
yaafecore.dataflow_dumpdot.restype = None
yaafecore.dataflow_dumpdot.argtypes = [c_void_p, c_char_p]
yaafecore.dataflow_display.restype = None
yaafecore.dataflow_display.argtypes = [c_void_p]

yaafecore.dataflow_createNode.restype = c_void_p
yaafecore.dataflow_createNode.argtypes = [c_void_p, c_char_p, POINTER(c_char_p*2)]
yaafecore.dataflow_setNodeName.restype = None
yaafecore.dataflow_setNodeName.argtypes = [c_void_p, c_void_p, c_char_p]
yaafecore.dataflow_getNode.restype = c_void_p
yaafecore.dataflow_getNode.argtypes = [c_void_p,c_char_p]
yaafecore.dataflow_node_getId.restype = c_char_p
yaafecore.dataflow_node_getId.argtypes = [c_void_p]
yaafecore.dataflow_node_getParams.restype = POINTER(c_char_p*2)
yaafecore.dataflow_node_getParams.argtypes = [c_void_p]
yaafecore.dataflow_node_sources.restype = POINTER(c_void_p)
yaafecore.dataflow_node_sources.argtypes = [c_void_p]
yaafecore.dataflow_node_targets.restype = POINTER(c_void_p)
yaafecore.dataflow_node_targets.argtypes = [c_void_p]
yaafecore.dataflow_getNodes.restype = POINTER(c_void_p)
yaafecore.dataflow_getNodes.argtypes = [c_void_p]
yaafecore.dataflow_rootNodes.restype = POINTER(c_void_p)
yaafecore.dataflow_rootNodes.argtypes = [c_void_p]
yaafecore.dataflow_finalNodes.restype = POINTER(c_void_p)
yaafecore.dataflow_finalNodes.argtypes = [c_void_p]
yaafecore.dataflow_freeNodeList.restype = None
yaafecore.dataflow_freeNodeList.argtypes = [POINTER(c_void_p)]
yaafecore.dataflow_link.restype = None
yaafecore.dataflow_link.argtypes = [c_void_p,c_void_p,c_char_p,c_void_p,c_char_p]
yaafecore.dataflow_append.restype = None
yaafecore.dataflow_append.argtypes = [c_void_p,c_void_p]
yaafecore.dataflow_merge.restype = None
yaafecore.dataflow_merge.argtypes = [c_void_p,c_void_p]
yaafecore.dataflow_useComponentLibrary.restype = None
yaafecore.dataflow_useComponentLibrary.argtypes = [c_void_p,c_char_p]
yaafecore.dataflow_getComponentLibraries.restype = POINTER(c_char_p)
yaafecore.dataflow_getComponentLibraries.argtypes = [c_void_p]
yaafecore.dataflow_freeComponentLibraries.restype = None
yaafecore.dataflow_freeComponentLibraries.argtypes = [POINTER(c_char_p)]

##################################################
# AudioFileProcessor functions
##################################################

yaafecore.audiofileprocessor_create.restype = c_void_p
yaafecore.audiofileprocessor_create.argtypes = []
yaafecore.audiofileprocessor_destroy.restype = None
yaafecore.audiofileprocessor_destroy.argtypes = [c_void_p]
yaafecore.audiofileprocessor_setOutputFormat.argtypes = [c_void_p,c_char_p,c_char_p,POINTER(c_char_p*2)]
yaafecore.audiofileprocessor_processFile.argtypes = [c_void_p,c_void_p,c_char_p]


##################################################
# Engine functions
##################################################

yaafecore.engine_create.restype = c_void_p
yaafecore.engine_create.argtypes = []
yaafecore.engine_destroy.restype = None
yaafecore.engine_destroy.argtypes = [c_void_p]
yaafecore.engine_load.argtypes = [c_void_p,c_void_p]
yaafecore.engine_getInputList.restype = POINTER(c_char_p)
yaafecore.engine_getInputList.argtypes = [c_void_p]
yaafecore.engine_getOutputList.restype = POINTER(c_char_p)
yaafecore.engine_getOutputList.argtypes = [c_void_p]
yaafecore.engine_freeIOList.restype = None
yaafecore.engine_freeIOList.argtypes = [POINTER(c_char_p)]

class IOInfo(Structure):
    _fields_ = [('sampleRate',c_double),
                ('sampleStep',c_int),
                ('frameLength',c_int),
                ('size',c_int),
                ('parameters',POINTER(c_char_p*2))]

yaafecore.engine_getInputInfos.restype = POINTER(IOInfo)
yaafecore.engine_getInputInfos.argtypes = [c_void_p,c_char_p]
yaafecore.engine_getOutputInfos.restype = POINTER(IOInfo)
yaafecore.engine_getOutputInfos.argtypes = [c_void_p,c_char_p]
yaafecore.engine_freeIOInfos.restype = None
yaafecore.engine_freeIOInfos.argtypes = [POINTER(IOInfo)]


yaafecore.engine_output_available.restype = None
yaafecore.engine_output_available.argtypes = [c_void_p,c_char_p,POINTER(c_int),POINTER(c_int)]

try:
    from numpy.ctypeslib import ndpointer
    # need numpy
    yaafecore.engine_input_write.restype = None
    yaafecore.engine_input_write.argtypes = [c_void_p,c_char_p,ndpointer(dtype='f8',ndim=2,flags='CONTIGUOUS'),c_int,c_int]
    yaafecore.engine_output_read.restype = c_int
    yaafecore.engine_output_read.argtypes = [c_void_p,c_char_p,ndpointer(dtype='f8',ndim=2,flags='CONTIGUOUS'),c_int,c_int]
except ImportError:
    # miss some functions
    pass

yaafecore.engine_reset.restype = None
yaafecore.engine_reset.argtypes = [c_void_p]
yaafecore.engine_process.restype = None
yaafecore.engine_process.argtypes = [c_void_p]
yaafecore.engine_flush.restype = None
yaafecore.engine_flush.argtypes = [c_void_p]