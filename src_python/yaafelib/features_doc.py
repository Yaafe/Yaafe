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

from yaafelib.audiofeature import AudioFeature
from inspect import isclass

def generate_features_parameters(app, what_, name, obj, options, lines):
    if what_=='class' and issubclass(obj, AudioFeature) and not obj is AudioFeature:
        params = obj.get_parameters()
        if params:
            params.sort(cmp=lambda x,y: cmp(x[0],y[0]))
            lines.append('')
            lines.append('**Parameters**:')
            for key,defaultValue,desc in params:
                lines.append(' * ``%s`` (default=%s): %s\n'%(key,defaultValue,desc))
        lines.append('')
        lines.append('**Declaration example**::\n')
        lines.append('')
        params_decl = [' %s=%s'%(key,val) for key,val,desc in params]
        lines.append('   %s'%obj.__name__ + ' '.join(params_decl) + '\n')
        seealso = [step[0] for step in obj.PARAMS if isinstance(step[0],type) and issubclass(step[0], AudioFeature)]
        if seealso:
            seealso = ', '.join([':class:`%s`'%cls.__name__ for cls in seealso])
            lines.append('')
            lines.append('.. seealso:: %s' % seealso)
                
    elif what_=='module':
        allFeatures = [kls for kls in obj.__dict__.values() if isclass(kls) and issubclass(kls, AudioFeature) and not kls==AudioFeature]
        if not allFeatures:
            return
        simpleFeature = [kls.__name__ for kls in allFeatures if not kls.TRANSFORM]
        transform = [kls.__name__ for kls in allFeatures if kls.TRANSFORM]
        if simpleFeature:
            simpleFeature.sort()
            lines.append('Available features')
            lines.append('------------------')
            lines.append('')
            for kls in simpleFeature:
                lines.append(kls)
                lines.append(''.rjust(len(kls),'^'))
                lines.append('.. autoclass:: %s'%kls)
                lines.append('')
        if transform:
            transform.sort()
            lines.append('Available feature transforms')
            lines.append('----------------------------')
            lines.append('')
            for kls in transform:
                lines.append(kls)
                lines.append(''.rjust(len(kls),'^'))
                lines.append('.. autoclass:: %s'%kls)
                lines.append('')
