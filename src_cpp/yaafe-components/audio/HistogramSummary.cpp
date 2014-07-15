/*
 * HistogramSummaryIntegrator.cpp
 *
 *  Created on: Sep 1, 2009
 *      Author: bmathieu
 */

#include "HistogramSummary.h"
#include "math.h"

using namespace std;

namespace YAAFE {

HistogramSummary::HistogramSummary() :
	m_hstep(0) {
}

HistogramSummary::~HistogramSummary() {
}

ParameterDescriptorList HistogramSummary::getParameterDescriptorList() const {
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier = "HInf";
	p.m_description = "Minimal value of histogram";
	p.m_defaultValue = "0";
	pList.push_back(p);

	p.m_identifier = "HSup";
	p.m_description = "Maximal value of histogram";
	p.m_defaultValue = "1";
	pList.push_back(p);

	return pList;
}

StreamInfo HistogramSummary::init(const ParameterMap& params,
		const StreamInfo& in) {
	double hinf = getDoubleParam("HInf", params);
	double hsup = getDoubleParam("HSup", params);
	double hstep = (hsup - hinf) / in.size;

	m_hinf = hinf + hstep / 2;
	m_hstep = hstep;

	return StreamInfo(in, 6);
}

void HistogramSummary::processToken(double* inData, const int inSize, double* outData,
		const int outSize)
{
	int i1 = -1, i2 = -1;
	double max1 = 0, max2 = 0;

	// find 2 best peaks
	for (int i = 0; i < inSize; i++) {
		if ((i == inSize - 1 || inData[i] >= inData[i + 1]) && (i == 0
				|| inData[i] > inData[i - 1])) {
			// process peak
			if (inData[i] > max2) {
				if (inData[i] > max1) {
					max2 = max1;
					i2 = i1;
					max1 = inData[i];
					i1 = i;
				} else {
					max2 = inData[i];
					i2 = i;
				}
			}
			i++; // skip next element
		}
	}

	// output amplitude and value of peaks
	if (max1 > 0) {
		outData[0] = max1;
		outData[1] = m_hinf + m_hstep * i1;
	} else {
		outData[0] = nan("");
		outData[1] = nan("");
	}
	if (max2 > 0) {
		outData[2] = max2;
		outData[3] = m_hinf + m_hstep * i2;
	} else {
		outData[2] = nan("");
		outData[3] = nan("");
	}
	// output ratio of second amplitude divided by first amplitude
	outData[4] = max2 / max1;
	// output overall sum
	{
		double s = 0;
		for (int i=0;i<inSize;i++)
			s += inData[i];
		outData[5] = s;
	}
}

}
