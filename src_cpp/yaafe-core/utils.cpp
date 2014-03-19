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

#ifndef UTILS_H_
#include "utils.h"
#endif

#include <stdio.h>
#include <iostream>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#ifndef __MINGW32__
#include <pwd.h>
#else
#include <windows.h>
#endif

//#ifdef WITH_FFTW3
//#include "fftw3.h"
//#endif

using namespace std;

namespace YAAFE
{

bool verboseFlag = false;

string getParam(const std::string& key, const ParameterMap& params, const ParameterDescriptorList& pList)
{
    ParameterMap::const_iterator it = params.find(key);
    if (it!=params.end())
    {
        return it->second;
    }
    for (size_t i=0;i<pList.size();i++)
    {
        if (pList[i].m_identifier == key)
            return pList[i].m_defaultValue;
    }
    cerr << "Warning: no parameter " << key << " found in given ParameterMap nor ParameterDescriptorList !" << endl;
    return "";
}

std::string encodeParameterMap(const ParameterMap& params)
{
	ostringstream oss;
	for (ParameterMap::const_iterator it=params.begin();it!=params.end();it++)
	{
		oss << it->first << "%" << it->second << "#";
	}
	string res = oss.str();
	return res.substr(0,res.size()-1);
}

ParameterMap decodeParameterMap(const std::string& thestr)
{
	ParameterMap attrs;
	string str = thestr;
	while (str.size()>0)
	{
		size_t attrEnd = str.find('#');
		if (attrEnd==string::npos)
			attrEnd = str.size();
		size_t keyEnd = str.find('%');
		if (keyEnd==string::npos)
		{
			cerr << "ERROR: cannot parse H5 Attribute : " << str.substr(0,attrEnd) << " !" << endl;
			return ParameterMap();
		}
		string key = str.substr(0,keyEnd);
		string value = str.substr(keyEnd+1,attrEnd-keyEnd-1);
		attrs[key] = value;
		if (attrEnd==str.size())
			break;
		str = str.substr(attrEnd+1);
	}
	return attrs;
}


#ifdef WITH_TIMERS

vector<Timer*> Timer::s_allTimers;

Timer::Timer(const std::string& name) :
    m_name(name), m_totalTime(0.0), m_lastStart(0)
{}

Timer* Timer::get_timer(const string& name)
{
	Timer* t = NULL;
	#pragma omp critical (gettimer)
	{
		for (vector<Timer*>::iterator it=s_allTimers.begin();it!=s_allTimers.end();it++)
		{
			if ((*it)->m_name == name) {
				t = *it;
				break;
			}
		}
		if (t == NULL) {
			t = new Timer(name);
			Timer::s_allTimers.push_back(t);
		}
	}
	return t;
}

void Timer::print_all_timers()
{
    for (vector<Timer*>::const_iterator it=s_allTimers.begin();
         it!=s_allTimers.end();it++)
    {
        cout << (*it)->m_name << " : " << (*it)->m_totalTime << endl;
        delete *it;
    }
}

#endif

}
