Extending Yaafe
===============

Before extending *Yaafe*, the developper have to be aware of *Yaafe*'s
general architecture. Please read carefully :doc:`/manual/internals`. 

As a summary, keep in mind that a feature is defined by 2 layers:

#. a python class which builds the directed graph of components, representing the steps to compute the feature
#. the components used to compute the feature. Each component is a C++ class which process the computations like frame tokenization, fft, cepstrum. 


Defining new features
---------------------

To define a new feature, the programmer has to create a subclass of :py:class:`AudioFeature <yaafelib.AudioFeature>`.
see examples in the ``yaafefeatures.py`` file.

.. autoclass:: yaafelib.AudioFeature
    :members:


Create new components
---------------------

.. _components:

A component is a C++ class which represents computational step, such as FrameTokenizer, FFT, Cepstrum, etc.

To create new components, the programmer has to subclass ``YAAFE::Component`` (see file ``src_cpp/yaafe-core/Component.h``).

New components have to be registered to the ``YAAFE::ComponentFactory``. The usual way is to create a
dynamic library which will be loaded at runtime, and to expose the function ``registerYaafeComponents`` (see ``src_cpp/yaafe-components/registration.h``).
The programmer who is integrating *Yaafe*'s C++ Engine into his program can load components directly from his code.


