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

#include "ComplexDomainFlux.h"

#include <Eigen/Dense>
#include <iostream>

using namespace Eigen;
using namespace std;

namespace YAAFE
{

ComplexDomainFlux::ComplexDomainFlux()
{
}

ComplexDomainFlux::~ComplexDomainFlux()
{
}

bool ComplexDomainFlux::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	outStreamInfo().add(StreamInfo(in,1));
    return true;
}

template<typename Scalar>
struct rotatorOp {
  complex<Scalar> operator()(const complex<Scalar>& x) const { return ((x.real()!=0) || (x.imag()!=0)) ? x / abs(x) : 1;}
};

bool ComplexDomainFlux::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	if ((out->tokenno()==0) && (in->tokenno()!=-2))
		in->prependZeros(2);
	if (!in->hasTokens(3)) return false;

	const int N = in->info().size/2;
	ArrayXcd inPredPredRotator(N);
	ArrayXcd inPredRotator(N);
	rotatorOp<double> op;
	{
		Map<ArrayXcd> inPredPredData((complex<double>*) in->token(0),N);
		inPredPredRotator = inPredPredData.unaryExpr(op);
	}
	while (in->hasTokens(3))
	{
		Map<ArrayXcd> inPredData((complex<double>*) in->token(1),N);
		Map<ArrayXcd> inData((complex<double>*) in->token(2), N);
		inPredRotator = inPredData.unaryExpr(op);
		double* output = out->writeToken();
		*output++ = (inData - (inPredData * (inPredRotator * inPredPredRotator.conjugate()))).abs().sum();
		in->consumeToken();
		inPredPredRotator.swap(inPredRotator);
	}

    return true;
}

}
