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

#include "Difference.h"
#include <iostream>

using namespace std;

namespace YAAFE
{

Difference::Difference() :
    m_nbCoeffs(0)
{};

Difference::~Difference()
{
}

ParameterDescriptorList Difference::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "DiffNbCoeffs";
    p.m_description = "Maximum number of coeffs to keep. 0 keeps N-1 value (with N the input feature size)";
    p.m_defaultValue = "0";
    pList.push_back(p);

    return pList;
}

bool Difference::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_nbCoeffs = getIntParam("DiffNbCoeffs",params);
	if (m_nbCoeffs==0)
		m_nbCoeffs = in.size - 1;
	if (m_nbCoeffs > in.size-1)
	{
		cerr << "Warning: cannot compute " << m_nbCoeffs << " difference coefficients from input of size " << in.size << endl;
		m_nbCoeffs = in.size - 1;
		cerr << "take only " << m_nbCoeffs << " coefficients" << endl;
	}

	outStreamInfo().add(StreamInfo(in,m_nbCoeffs));
    return true;
}

bool Difference::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	const int N = in->info().size - 1;
	while (!in->empty())
	{
		double* inData = in->readToken();
		double* outData = out->writeToken();
		for (int i=0;i<N;i++)
			outData[i] = inData[i+1] - inData[i];
		in->consumeToken();
	}
    return true;
}

}
