/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Author : Benoit Mathieu
 *
 * This file is part of Yaafe.
 *
 * Yaafe is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Yaafe is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Component.h"

#include <stdlib.h>
#include <assert.h>
#include <iostream>

using namespace std;

namespace YAAFE
{

Component::Component()
{
}

Component::~Component()
{
}

ParameterDescriptorList Component::getParameterDescriptorList() const
{
    return ParameterDescriptorList();
}

const std::string EMPTY_STRING("");

std::string Component::getStringParam(const std::string& id, const ParameterMap& params)
{
    ParameterMap::const_iterator it=params.find(id);
    if (it!=params.end()) {
    	if (it->second.size()==0) {
    		cerr << "ERROR: parameter " << id << " is empty !" << endl;
    		return EMPTY_STRING;
    	}
        return it->second;
    }
    ParameterDescriptorList pList = getParameterDescriptorList();
    for (ParameterDescriptorList::const_iterator descIt=pList.begin();
         descIt!=pList.end(); descIt++)
    {
    	if (descIt->m_identifier==id)
    		return descIt->m_defaultValue;
    }
    cerr << "ERROR: no parameter " << id << " for component " << getIdentifier() << " !" << endl;
    return EMPTY_STRING;
}

int Component::getIntParam(const std::string& id, const ParameterMap& params)
{
    return atoi(getStringParam(id, params).c_str());
}

double Component::getDoubleParam(const std::string& id, const ParameterMap& params)
{
    return atof(getStringParam(id, params).c_str());
}

}
