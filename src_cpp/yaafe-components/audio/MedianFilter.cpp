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

#include "MedianFilter.h"
#include <math.h>

#include <algorithm>

using namespace std;

namespace YAAFE {

MedianFilter::MedianFilter() {
	m_order = 0;
	m_delay = 0;
	m_data = NULL;
}

MedianFilter::~MedianFilter() {
	if (m_data)
		delete [] m_data;
}

ParameterDescriptorList MedianFilter::getParameterDescriptorList() const
{
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier = "MFOrder";
	p.m_description = "Median filtering order, a number of frame ('7' or '9') or a duration ('0.5s' or '0.125s').";
	p.m_defaultValue = "5";
	pList.push_back(p);

	return pList;
}

bool MedianFilter::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	string orderStr = getStringParam("MFOrder",params);
	if (orderStr[orderStr.size()-1]=='s')
	{
		double dur = atof(orderStr.substr(0,orderStr.size()-1).c_str());
		m_order = (int) round(dur*in.sampleRate/in.sampleStep);
	} else {
		m_order = getIntParam("MFOrder",params);
	}
	if (m_order==0)
	{
		cerr << "ERROR: invalid parameter MFOrder " << orderStr << endl;
		return false;
	}

	m_delay = (m_order-1) / 2;
	m_medianIndex = m_delay;

	outStreamInfo().add(in);
	m_dataSize = 2*m_order*in.size;
	m_data = new double[m_dataSize];
	reset();

	return true;
}

void MedianFilter::reset() {
	for (int i=0;i<m_dataSize;i++)
		m_data[i] = 0.0;
	m_pos = 0;
}

bool MedianFilter::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	while (!in->empty())
	{
		double* inPtr = in->readToken();
		double* outPtr = NULL;
		if ((in->tokenno()-out->tokenno()) == m_delay)
			outPtr = out->writeToken();
		for (int i=0;i<in->info().size;i++)
		{
			double* buf = m_data + 2*m_order*i;
			double newValue = isnan(inPtr[i]) ? 0 : inPtr[i];
			double oldValue = buf[m_pos];
			buf[m_pos] = newValue;
			buf += m_order; // buf now points to median filter state
			double* medianPtr = buf + m_medianIndex;
			double* buf_end = buf + m_order;
			if (oldValue<buf[m_medianIndex])
			{
				double* oldPos = find(buf,medianPtr,oldValue);
				if (newValue<*medianPtr)
				{
					// newValue and oldValue are lower than median
					// just swap values
					*oldPos = newValue;
				} else {
					// oldValue < median <= newValue
					*oldPos = *medianPtr;
					*medianPtr = newValue;
					double* minElem = min_element(medianPtr,buf_end);
					if (minElem!=medianPtr)
						iter_swap(minElem,medianPtr);
				}
			} else {
				double* oldPos = find(medianPtr,buf_end, oldValue);
				if (newValue<*medianPtr)
				{
					// newValue < median <= oldValue
					*oldPos = *medianPtr;
					*medianPtr = newValue;
					double* maxElem = max_element(buf,medianPtr+1);
					if (maxElem!=medianPtr)
						iter_swap(maxElem,medianPtr);
				} else {
					// newValue and oldValue are more or equal to median
					*oldPos=newValue;
					if (oldPos==medianPtr) {
						oldPos = min_element(medianPtr,buf_end);
						iter_swap(oldPos,medianPtr);
					}
				}
			}

			if (outPtr) {
				outPtr[i] = *medianPtr;
				if (m_order%2==0)
					outPtr[i] = (outPtr[i] + *min_element(medianPtr+1,buf_end))/2;
			}
		}
		in->consumeToken();
		m_pos = ++m_pos % m_order;
	}

    return true;
}

void MedianFilter::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	inp[0].data->appendZeros(m_delay);
	process(inp,outp);
}

}
