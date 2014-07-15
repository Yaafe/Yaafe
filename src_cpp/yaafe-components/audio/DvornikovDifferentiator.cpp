/*
 * DvornikovDifferentiator.cpp
 *
 *  Created on: Jan 5, 2010
 *      Author: bmathieu
 */

#include "DvornikovDifferentiator.h"
#include "MathUtils.h"


using namespace std;
using namespace YAAFE;

namespace YAAFE_EXT {

DvornikovDifferentiator::DvornikovDifferentiator() {
}

DvornikovDifferentiator::~DvornikovDifferentiator() {
}

ParameterDescriptorList DvornikovDifferentiator::getParameterDescriptorList() const {
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier = "DDOrder";
	p.m_description = "Dvornikov Differentiator filter order: odd number of frames ('9', '15', etc.) or time duration ('0.1s', '1.5s', etc..)";
	p.m_defaultValue = "9";
	pList.push_back(p);

	return pList;
}

bool DvornikovDifferentiator::initFilter(const ParameterMap& params,
		const StreamInfo& in) {
	// get order
	int order = 0;
	string orderStr = getStringParam("DDOrder",params);
	if (orderStr[orderStr.size()-1]=='s')
	{
		double dur = strtod(orderStr.substr(0,orderStr.size()-1).c_str(),NULL);
		dur /= 2;
		order = (int) floor(dur*in.sampleRate/in.sampleStep);
		order = 2 * order + 1;
	} else {
		order = atoi(orderStr.c_str());
		if (order%2)
		{
			cerr << "ERROR: DDOrder parameter must be odd in " << getIdentifier() << " component !" << endl;
			return false;
		}
	}
	// build filter
	m_length = order;
	m_filter = new double[order];
	for (int i=0;i<order;i++)
		m_filter[i] = 0;
	int halfOrder = (order - 1) / 2;
	m_delay = halfOrder;
	for (int m=1;m<=halfOrder;m++)
	{
		double r1 = 1;
		for (int k=1;k<=halfOrder;k++)
			if (k!=m)
				r1 *= 1 - pow2((double)m/k);
		r1 = 1 / (2*r1*m);
		m_filter[halfOrder-m] = -r1;
		m_filter[halfOrder+m] = r1;
	}
	return true;
}

}
