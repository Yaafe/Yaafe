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

#include "HalfHannFilter.h"
#include "MathUtils.h"

using namespace std;
using namespace Eigen;

namespace YAAFE {

HalfHannFilter::HalfHannFilter() {
}

HalfHannFilter::~HalfHannFilter() {
}

ParameterDescriptorList HalfHannFilter::getParameterDescriptorList() const {
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier="HHFOrder";
	p.m_description = "Half Hanning Filter order, in number of frames ('10','16'), or in duration ('0.1s','0.25s')";
	p.m_defaultValue = "10";
	pList.push_back(p);

	return pList;
}

bool HalfHannFilter::initFilter(const ParameterMap& params, const StreamInfo& in) {

	// get filter order
	int order = 0;
	string orderStr = getStringParam("HHFOrder",params);
	if (orderStr[orderStr.size()-1]=='s')
	{
		double dur = atof(orderStr.substr(0,orderStr.size()-1).c_str());
		order = (int) floor(dur*in.sampleRate/in.sampleStep);
	} else {
		order = atoi(orderStr.c_str());
	}
	// build filter
	VectorXd hann = ehanning(2*order-1);
	m_delay = 0;
	m_length = order;
	m_filter = new double[m_length];
	memcpy(m_filter,hann.data(),order*sizeof(double));
	return true;
}

}
