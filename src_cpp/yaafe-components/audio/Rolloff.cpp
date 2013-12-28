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

#include "Rolloff.h"

namespace YAAFE
{

Rolloff::Rolloff() :
    m_coeff(0.0)
{
}

Rolloff::~Rolloff()
{
}

bool Rolloff::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	double coeff = in.sampleRate / (2 * (in.size-1));
    m_coeff = coeff;
    outStreamInfo().add(StreamInfo(in,1));
    return true;
}

bool Rolloff::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	const int N = in->info().size;
    while (!in->empty())
    {
        const double* inData = in->readToken();
        double ec = 0;
        for (int i=0;i<N;++i)
        	ec += inData[i];
        double thres = 0.99 * ec;
        int kc = N -1;
        while (ec > thres && kc >= 0)
        {
            ec -= inData[kc];
            --kc;
        }
        double output = (kc+1) * m_coeff;
        out->write(&output,1);
        in->consumeToken();
    }
    return true;
}

}
