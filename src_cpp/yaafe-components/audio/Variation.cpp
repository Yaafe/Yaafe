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

#include "Variation.h"
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

Variation::Variation()
{
}

Variation::~Variation()
{
}

bool Variation::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;
	outStreamInfo().add(StreamInfo(in,1));
    return true;
}

bool Variation::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (!in->hasTokens(2)) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	if ((out->tokenno()==0) && (in->tokenno()!=-1))
		in->prependZeros(1);

	const int N = in->info().size;
    double lastNorm = 0.0;
    double nextNorm = Map<VectorXd>(in->token(0),N).norm();
    while (in->hasTokens(2))
    {
    	Map<VectorXd> last(in->token(0),N);
        lastNorm = nextNorm;
        Map<VectorXd> next(in->token(1),N);
        nextNorm = next.norm();
        if (lastNorm*nextNorm !=0)
        	lastNorm = 1 - last.dot(next) / (lastNorm * nextNorm);
        else
        	lastNorm = 0.0;
        out->write(&lastNorm,1);
        in->consumeToken();
    }
    return true;
}

}
