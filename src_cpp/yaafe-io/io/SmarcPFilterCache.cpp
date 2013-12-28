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

#include "SmarcPFilterCache.h"
#include <iostream>

#define CACHE_SIZE 5
#define BANDWIDTH 0.95
#define RP 0.1
#define RS 120
#define TOL 0.000001

using namespace std;

namespace YAAFE {

std::list<struct PFilter*> SmarcPFilterCache::s_cache;

struct PFilter* SmarcPFilterCache::getPFilter(int fsin, int fsout)
{
	if (fsin==fsout)
		return NULL;
	// check cache
	for (list<struct PFilter*>::iterator it=s_cache.begin();it!=s_cache.end();it++)
	{
		if ((smarc_get_fs_in(*it)==fsin) && (smarc_get_fs_out(*it)==fsout))
		{
			struct PFilter* f = *it;
			s_cache.erase(it);
			s_cache.push_front(f);
			return f;
		}
	}
	// create filter and put in cache
	cerr << "initializing Smarc resampler " << fsin << " => " << fsout << endl;
	s_cache.push_front(smarc_init_pfilter(fsin,fsout,BANDWIDTH,RP,RS,TOL,NULL,1));
	cerr << "Smarc resampler ok !" << endl;
	// release old pfilter if cache exceeds size
	if (s_cache.size()>CACHE_SIZE) {
		smarc_destroy_pfilter(s_cache.back());
		s_cache.pop_back();
	}
	return s_cache.front();
}

void SmarcPFilterCache::release() {
	for (list<struct PFilter*>::iterator it=s_cache.begin();it!=s_cache.end();it++)
	{
		smarc_destroy_pfilter(*it);
	}
}

}
