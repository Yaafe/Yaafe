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
from core import iterPtrList, iterPtrDict 

from ctypes import *

class DataFlowNode(object):
    def __init__(self, ptr):
        self.ptr = ptr
        self._component_id = None
        self._params = None

    def componentId(self):
        if not self._component_id:
            self._component_id = yc.dataflow_node_getId(self.ptr)
        return self._component_id
    
    def params(self):
        if not self._params:
            out = yc.dataflow_node_getParams(self.ptr)
            self._params = dict((k,v) for k,v in iterPtrDict(out))
            yc.dataflow_node_freeParams(out)
        return self._params
    
    def sourceNodes(self):
        out = yc.dataflow_node_sources(self.ptr)
        res = [DataFlowNode(n) for n in iterPtrList(out)]
        yc.dataflow_freeNodeList(out)
        return res
    
    def targetNodes(self):
        out = yc.dataflow_node_targets(self.ptr)
        res = [DataFlowNode(n) for n in iterPtrList(out)]
        yc.dataflow_freeNodeList(out)
        return res

    def __str__(self):
        return 'DataFlowNode: %s %s'%(self.componentId(),
                        ' '.join(['%s=%s'%(k,v) for k,v in self.params().iteritems()]))
    

class DataFlow(object):
    """
        A DataFlow object hold a directed acyclic graph of computational steps
        describing how to compute some audio features.
        
        DataFlow can be loaded directly from a :ref:`dataflow file <dataflow-file>`
        using the :py:meth:`load` method, or created with a :py:class:`FeaturePlan` object. 
        The advanced user may also build a dataflow graph from scratch.
    """
    def __init__(self):
        self.ptr = yc.dataflow_create();
    def __del__(self):
        yc.dataflow_destroy(self.ptr)
        
    def load(self, filename):
        """
            Build DataFlow from a :ref:`dataflow file <dataflow-file>`.
            
            :param filename: dataflow file name.
            :type filename: string
            :return: True on success, False on fail.
        """
        return True if yc.dataflow_load(self.ptr, filename) else False
    def save(self, filename):
        """
            write DataFlow into a :ref:`dataflow file <dataflow-file>`.
            
            :param filename: file to write
            :type filename: string
        """
        yc.dataflow_save(self.ptr, filename)
        
    def dumpdot(self, filename):
        """
            write a got graph corresponding to the DataFlow
            
            :param filename: file to write
            :type filename: string
        """
        yc.dataflow_dumpdot(self.ptr, filename)
        
    def display(self):
        """
            Print the DataFlow to the standard output
        """
        yc.dataflow_display(self.ptr)
        
    def createNode(self,componentId,params):
        tmp = ((c_char_p*2)*(len(params)+1))() 
        tmp[:-1] = [(c_char_p*2)(c_char_p(k),c_char_p(v)) for k,v in params.iteritems()]
        return DataFlowNode(yc.dataflow_createNode(self.ptr, componentId,tmp))
    def setNodeName(self,node,name):
        yc.dataflow_setNodeName(self.ptr,node.ptr,name)
    def getNode(self,name):
        ptr = yc.dataflow_getNode(self.ptr,name)
        return DataFlowNode(ptr) if ptr else None
    def createInput(self,name,params):
        n = self.createNode('Input',params)
        self.setNodeName(n, name)
        return n
    def createOutput(self,name,params):
        n = self.createNode('Output',params)
        self.setNodeName(n, name)
        return n

    def link(self,sourceNode,sourcePort,targetNode,targetPort):
        yc.dataflow_link(self.ptr,sourceNode.ptr,sourcePort,targetNode.ptr,targetPort)
        
    def getNodes(self):
        out = yc.dataflow_getNodes(self.ptr);
        res = [DataFlowNode(v) for v in iterPtrList(out)]
        yc.dataflow_freeNodeList(out);
        return res
    def rootNodes(self):
        out = yc.dataflow_rootNodes(self.ptr);
        res = [DataFlowNode(v) for v in iterPtrList(out)]
        yc.dataflow_freeNodeList(out);
        return res
    def finalNodes(self):
        out = yc.dataflow_finalNodes(self.ptr);
        res = [DataFlowNode(v) for v in iterPtrList(out)]
        yc.dataflow_freeNodeList(out);
        return res
    
    def append(self,dataflow):
        yc.dataflow_append(self.ptr,dataflow.ptr)
    def merge(self,dataflow):
        yc.dataflow_merge(self.ptr,dataflow.ptr)
        
    def useComponentLibrary(self,libname):
        yc.dataflow_useComponentLibrary(self.ptr,libname)
    def getComponentLibraries(self):
        out = yc.dataflow_getComponentLibraries(self.ptr)
        res = [l for l in iterPtrList(out)]
        yc.dataflow_freeComponentLibraries(out)
        return res

