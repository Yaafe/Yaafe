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

#include "ZCR.h"

namespace YAAFE
{

ZCR::ZCR()
{
}

ZCR::~ZCR()
{
}

bool ZCR::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	outStreamInfo() = StreamInfo(in,1);
    return true;
}

bool ZCR::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return true;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

    while (!in->empty())
    {
        double* prev = in->readToken();
        double* current = prev +1;
        double zcr = 0;
        for (int i=in->info().size-1;i>0;--i)
        {
            if (*current != 0.0)
            {
                if (*prev * *current < 0)
                    zcr += 1;
                prev = current;
                current ++;
                continue;
            } else {
                current++;
                continue;
            }
        }
        zcr /= in->info().size;
        out->write(&zcr,1);
        in->consumeToken();
    }
    return true;
}

}
