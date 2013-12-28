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

#include "Join.h"
#include <cmath>
#include "string.h"

using namespace std;

namespace YAAFE {

Join::Join() {
}

Join::~Join() {
}

bool Join::init(const ParameterMap& params,const Ports<StreamInfo>& inp)
{
	assert(inp.size()>0);
	const StreamInfo& inref = inp[0].data;
	int outSize = inref.size;

	for (int i=1;i<inp.size();++i)
	{
		const StreamInfo& in = inp[i].data;
		if (std::fabs(inref.sampleRate/inref.sampleStep - in.sampleRate/in.sampleStep)>0.000001)
		{
			cerr << "ERROR: try to join streams with differents sample rates" << endl;
			return false;
		}
		outSize += in.size;
	}

	outStreamInfo().add(StreamInfo(inref,outSize));

	return true;
}

bool Join::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()>0);
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	int insize[inp.size()];
	for (int i=0;i<inp.size();++i)
		insize[i] = inp[i].data->info().size;

	bool doneSomething = false;
	while (true)
	{
		// check if all input buffers have tokens
		for (int i=0;i<inp.size();i++)
			if (inp[i].data->empty())
				return doneSomething;

		// copy all input tokens into output token
		double* outData = out->writeToken();
		for (int i=0;i<inp.size();i++)
		{
			InputBuffer* in = inp[i].data;
			memcpy(outData,in->readToken(),insize[i] * sizeof(double));
			outData += insize[i];
			in->consumeToken();
		}

		doneSomething = true;
	}

	return doneSomething;
}


}
