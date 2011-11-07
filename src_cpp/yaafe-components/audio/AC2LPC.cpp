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

#include "AC2LPC.h"
#include "MathUtils.h"
#include <assert.h>

namespace YAAFE
{

AC2LPC::AC2LPC() : m_nbCoeffs(0)
{
}

AC2LPC::~AC2LPC()
{
}

ParameterDescriptorList AC2LPC::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "LPCNbCoeffs";
    p.m_description = "Number of Linear Predictor Coefficients to compute";
    p.m_defaultValue = "2";
    pList.push_back(p);
    return pList;
}

bool AC2LPC::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_nbCoeffs = getIntParam("LPCNbCoeffs",params);
	outStreamInfo().add(StreamInfo(in,m_nbCoeffs));
    return true;
}

bool AC2LPC::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

    assert(out->info().size==m_nbCoeffs);
    assert(in->info().size>m_nbCoeffs);
    while (!in->empty())
    {
        ac2lpc(in->readToken(),out->writeToken(),m_nbCoeffs);
        in->consumeToken();
    }
    return true;
}

}
