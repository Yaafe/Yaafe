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

from __future__ import absolute_import, print_function

import sys
from ctypes import cast, c_char_p

from yaafelib._compat import iteritems, to_char, to_str
from yaafelib.core import yaafecore as yc
from yaafelib.core import iterPtrList, iterPtrDict


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
            self._params = dict((k, v) for k, v in iterPtrDict(out))
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

    def __repr__(self):
        return 'DataFlowNode: %s %s' % (
            self.componentId(),
            ' '.join(['%s=%s' % (k, v) for k, v in iteritems(self.params())]))


class DataFlow(object):
    """
        A DataFlow object hold a directed acyclic graph of computational steps
        describing how to compute some audio features.

        DataFlow can be loaded directly from a :ref:`dataflow file
        <dataflow-file>` using the :py:meth:`load` method, or created with a
        :py:class:`FeaturePlan` object.
        The advanced user may also build a dataflow graph from scratch.
    """
    DATAFLOW_KEY = '__dataflow_string'

    def __init__(self):
        self.ptr = yc.dataflow_create()
        self.__dict__[self.DATAFLOW_KEY] = ''

    def __del__(self):
        yc.dataflow_destroy(self.ptr)

    def __getstate__(self):
        state = self.__dict__.copy()
        del state['ptr']
        return state

    def __setstate__(self, state):
        self.__init__()
        self.loads(state.get(self.DATAFLOW_KEY))
        self.update_state()

    def update_state(self):
        self.__dict__[self.DATAFLOW_KEY] = str(self)

    def load(self, filename):
        """
            Build DataFlow from a :ref:`dataflow file <dataflow-file>`.

            :param filename: dataflow file name.
            :type filename: string
            :return: True on success, False on fail.
        """
        if yc.dataflow_load(self.ptr, to_char(filename)):
            self.update_state()
            return True
        return False

    def loads(self, buf):
        """
            Build DataFlow from buf read from a :ref:`dataflow file
            <dataflow-file>`.

            :param buf: buffer read from a dataflow file
            :type buf: string
            :return: True on success, False on fail.
        """
        if yc.dataflow_loads(self.ptr, to_char(buf)):
            self.update_state()
            return True
        return False

    def save(self, filename):
        """
            write DataFlow into a :ref:`dataflow file <dataflow-file>`.

            :param filename: file to write
            :type filename: string
        """
        yc.dataflow_save(self.ptr, to_char(filename))

    def __str__(self):
        """
            Return the Dataflow in string, which can be reload
            via :ref:`DataFlow.loads <Dataflow.loads>`

            :return: Dataflow in string
        """
        ptr = yc.dataflow_stringify(self.ptr)
        buf = cast(ptr, c_char_p).value
        yc.free_dataflow_stringify(ptr)
        return to_str(buf)

    def dumpdot(self, filename):
        """
            write a got graph corresponding to the DataFlow

            :param filename: file to write
            :type filename: string
        """
        yc.dataflow_dumpdot(self.ptr, to_char(filename))

    def display(self):
        """
            Print the DataFlow to the standard output
        """
        yc.dataflow_display(self.ptr)

    def createNode(self, componentId, params):
        tmp = ((c_char_p*2)*(len(params)+1))()
        tmp[:-1] = [(c_char_p*2)(c_char_p(to_char(k)), c_char_p(to_char(v)))
                    for k, v in iteritems(params)]
        return DataFlowNode(yc.dataflow_createNode(self.ptr,
                                                   to_char(componentId), tmp))

    def setNodeName(self, node, name):
        yc.dataflow_setNodeName(self.ptr, node.ptr, to_char(name))

    def getNode(self, name):
        ptr = yc.dataflow_getNode(self.ptr, to_char(name))
        return DataFlowNode(ptr) if ptr else None

    def createInput(self, name, params):
        n = self.createNode('Input', params)
        self.setNodeName(n, name)
        return n

    def createOutput(self, name, params):
        n = self.createNode('Output', params)
        self.setNodeName(n, name)
        return n

    def link(self, sourceNode, sourcePort, targetNode, targetPort):
        yc.dataflow_link(self.ptr, sourceNode.ptr, to_char(sourcePort),
                         targetNode.ptr, to_char(targetPort))

    def getNodes(self):
        out = yc.dataflow_getNodes(self.ptr)
        res = [DataFlowNode(v) for v in iterPtrList(out)]
        yc.dataflow_freeNodeList(out)
        return res

    def rootNodes(self):
        out = yc.dataflow_rootNodes(self.ptr)
        res = [DataFlowNode(v) for v in iterPtrList(out)]
        yc.dataflow_freeNodeList(out)
        return res

    def finalNodes(self):
        out = yc.dataflow_finalNodes(self.ptr)
        res = [DataFlowNode(v) for v in iterPtrList(out)]
        yc.dataflow_freeNodeList(out)
        return res

    def append(self, dataflow):
        yc.dataflow_append(self.ptr, dataflow.ptr)

    def merge(self, dataflow):
        yc.dataflow_merge(self.ptr, dataflow.ptr)

    def useComponentLibrary(self, libname):
        yc.dataflow_useComponentLibrary(self.ptr, to_char(libname))

    def getComponentLibraries(self):
        out = yc.dataflow_getComponentLibraries(self.ptr)
        res = [l for l in iterPtrList(out)]
        yc.dataflow_freeComponentLibraries(out)
        return res
