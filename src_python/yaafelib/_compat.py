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


PY2 = sys.version_info[0] == 2
PY3 = sys.version_info[0] == 3


def add_metaclass(metaclass):
    """
    Class decorator for creating a class with a metaclass irrespective of Py2
    or Py3. Taken from python package `six`
    https://bitbucket.org/gutworth/six/src/2c12cd64ff0c7797bb30b0d466e902f7ecd6e562/six.py?fileviewer=file-view-default#six.py-826
    """
    def wrapper(cls):
        orig_vars = cls.__dict__.copy()
        slots = orig_vars.get('__slots__')
        if slots is not None:
            if isinstance(slots, str):
                slots = [slots]
            for slots_var in slots:
                orig_vars.pop(slots_var)
        orig_vars.pop('__dict__', None)
        orig_vars.pop('__weakref__', None)
        return metaclass(cls.__name__, cls.__bases__, orig_vars)
    return wrapper


if PY3:
    def iteritems(d, **kw):
        return iter(d.items(**kw))

    def to_char(s):
        """Convert string to the type used by ``ctypes.c_char_p``."""
        return bytes(s, 'utf-8') if isinstance(s, str) else s

    def to_str(s):
        """Convert from output of ``ctypes.c_char_p`` to str"""
        return s.decode('utf-8') if isinstance(s, bytes) else s
else:
    def iteritems(d, **kw):
        return d.iteritems(**kw)

    def to_char(s):
        """Convert string to the type used by ``ctypes.c_char_p``."""
        return s

    def to_str(s):
        """Convert from output of ``ctypes.c_char_p`` to str"""
        return s
