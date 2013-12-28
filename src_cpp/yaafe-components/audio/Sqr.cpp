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

#include "Sqr.h"
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

Sqr::Sqr()
{
}

Sqr::~Sqr()
{
}

bool Sqr::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	outStreamInfo().add(in);
    return true;
}

bool Sqr::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

//	Map<ArrayXd> input(in->readToken(),in->available()*in->info().size);
//	Map<ArrayXd> output(out->reserve(in->available()),in->available()*out->info().size);
//	output = input.square();
//    out->append(in->available());
//    in->forward(in->available());
	const int N = in->info().size;
	while (!in->empty())
	{
		Map<ArrayXd> inData(in->readToken(),N);
		Map<ArrayXd> outData(out->writeToken(),N);
		outData = inData.square();
		in->consumeToken();
	}
    return true;
}

}
