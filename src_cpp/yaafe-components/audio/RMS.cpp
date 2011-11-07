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

#include "RMS.h"
#include <Eigen/Dense>

using namespace Eigen;

namespace YAAFE {

RMS::RMS() {
}

RMS::~RMS() {
}

bool RMS::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

    outStreamInfo().add(StreamInfo(in,1));
    return true;
}

bool RMS::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

    const int N = in->info().size;
//    Map<MatrixXd> inData(in->readToken(),N,in->available());
//    Map<VectorXd> outData(out->reserve(in->available()),in->available());
//    outData = std::sqrt(inData.colwise().squaredNorm().array() / N);
//    out->append(in->available());
//    in->forward(in->available());
    while (!in->empty()) {
    	Map<VectorXd> inData(in->readToken(),N);
    	double* output = out->writeToken();
    	*output = std::sqrt(inData.squaredNorm() / N);
    	in->consumeToken();
    }
    return true;
}

}
