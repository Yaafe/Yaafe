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

def check_dataflow_params(f):
    def _safe_get_dataflow(cls,params,samplerate):
        # filter parameters and set missing parameters with default values
        for k,v in params.iteritems():
            if not k in [key for (key,_,_) in cls._EXPOSED_PARAM_DESCRIPTORS]:
                print 'WARNING: unknown parameter %s for feature %s !'%(k,cls.__name__)
        filt_params = dict([(name,str(params.get(name,default))) for (name,default,desc) in cls._EXPOSED_PARAM_DESCRIPTORS])
        # build dataflow
        df = f(cls, filt_params, samplerate)
        # automatically add library dependency
        for l in cls.COMPONENT_LIBS:
            df.useComponentLibrary(l)
        return df
    return _safe_get_dataflow

def dataflow_safe_append(dataflow,component,params):
    """
    Append component step to dataflow. Dataflow must have at most 1 final node.
    Parameters are filtered according to the component parameters.
    """
    filtered_params = {}
    for name,default,desc in yaafecore.getComponentParameters(component):
        filtered_params[name] = params.get(name,default)
    fNodes = dataflow.finalNodes()
    n = dataflow.createNode(component,filtered_params)
    if (len(fNodes)!=1):
        print 'WARNING: dataflow has %i final nodes when appending component %s !'% (len(fNodes), component)
        return
    dataflow.link(fNodes[0],'',n,'')

class AudioFeatureFactory(type):
    _FEATURES = []
    def __new__(mcs,name,base,kdict):
        # check class attributes
        if not 'PARAMS' in kdict:
            kdict['PARAMS'] = []
        if not 'TRANSFORM' in kdict:
            kdict['TRANSFORM'] = False
        # load component libraries
        if 'COMPONENT_LIBS' in kdict:
            for lib in kdict['COMPONENT_LIBS']:
                if yaafecore.loadComponentLibrary(lib)!=0:
                    return
        # compute exposed parameters and check components
        exposed_params = []
        components_available = True
        for p in kdict['PARAMS']:
            if not type(p)==tuple or not len(p) in [2,3]:
                print 'ERROR: invalid PARAMS attribute for feature %s !'%name
                return None
            if len(p)==3:
                # explicit parameter
                exposed_params.append(p)
            else:
                params = []
                if isinstance(p[0],AudioFeatureFactory):
                    if not AudioFeatureFactory.feature_available(p[0]):
                        components_available = False
                    # import parameters from a feature
                    params = p[0].get_parameters()
                else:
                    # import parameters from a component
                    if yaafecore.isComponentAvailable(p[0]):
                        params = yaafecore.getComponentParameters(p[0])
                    else:
                        components_available = False
                # override defaults
                params = [(key,p[1].get(key,value),desc) for (key,value,desc) in params if p[1].get(key,True)]
                exposed_params.extend(params)
        exposed_params.sort(cmp=lambda x,y: cmp(x[0],y[0]))
        kdict['_EXPOSED_PARAM_DESCRIPTORS'] = exposed_params
        kdict['_COMPONENTS_AVAILABLE'] = components_available
        # list all params
        kdict['_ALL_PARAMS'] = [key for (key,default,desc) in exposed_params]
        # create class
        return type.__new__(mcs,name,base,kdict)
        
    def __init__(cls, name, base, kdict):
        # register feature
        if not cls.__name__=='AudioFeature' and cls._COMPONENTS_AVAILABLE:
            AudioFeatureFactory._FEATURES.append(cls)
    
    @classmethod
    def feature_available(cls,feat):
        return feat in cls._FEATURES
    
    @classmethod
    def get_feature(cls,name):
        for feat in cls._FEATURES:
            if (feat.__name__ == name):
                return feat
        print 'ERROR: unknown feature %s'%name
        return None
    
    @classmethod
    def get_all_features(cls):
        return cls._FEATURES

class AudioFeature(object):
    '''
    classdocs
    '''
    __metaclass__ = AudioFeatureFactory
    
    TRANSFORM = False
    '''
    TRANSFORM is a flag to tell whether the feature is a transformation
    to apply on another feature (Cepstrum, StatisticalIntegrator), or a
    raw feature which is computed directly from the audio signal.
    '''
    
    PARAMS = []
    '''
    PARAMS is a list of tuples defining the parameters of the feature.
    There are 3 differents ways of defining parameters:
    
    Inherits parameters from another feature
        ``(MagnitudeSpectrum, params_dict)`` inherits parameters from the 
        already defined feature MagnitudeSpectrum
    Inherits parameters from a component
        ``('FFT', params_dict)`` inherits parameters from the components
        corresponding to the given id string
    Define new parameter
        ``('Name','default','description')`` defines a new parameter with
        its default value and description.
    
    ``params_dict`` is a dictionary which allow overiding parameters
    default value, or masking a parameter we don't want the user to provide.
    For example, ``{'blockSize':'512', 'FFTWindow':None}`` will overide
    blockSize default value and mask the FFTWindow parameter.
    '''
    
    @classmethod
    def get_parameters(cls):
        '''
        Returns the list of parameters with their description and default
        value. Subclasses do not have to overide this function, the parameter
        list is build automatically from the PARAMS class attribute.
        '''
        return cls._EXPOSED_PARAM_DESCRIPTORS
        
    @classmethod
    def get_dataflow(cls,params, samplerate):
        '''
        Return the dataflow to compute the feature with the given parameters
        at the given sample rate. This is the main method to overide when
        subclassing AudioFeature.
        
        :param params: parameters
        :type params: dict
        :param samplerate: the sample rate
        :return: the feature dataflow
        :rtype: :py:class:`DataFlow <yaafelib.DataFlow>`
        
        '''
        raise NotImplementedError

    @classmethod
    def filter_params(cls,params):
        '''
        Returns a dictionary holding only parameters defined from the PARAMS attribute
        with default value for those not in the given params dict.
        '''
        return dict([(name,params.get(name,default)) for (name,default,desc) in cls._EXPOSED_PARAM_DESCRIPTORS])