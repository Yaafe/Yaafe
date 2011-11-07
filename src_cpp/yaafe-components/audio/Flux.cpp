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

#include "Flux.h"
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

Flux::Flux()
{
}

Flux::~Flux()
{
}

ParameterDescriptorList Flux::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "FluxSupport";
    p.m_description = "support of flux computation. if 'All' then use all bins (default), if 'Increase' then use only bins which are increasing";
    p.m_defaultValue = "All";
    pList.push_back(p);

    return pList;
}

bool Flux::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_onlyIncrease = (getStringParam("FluxSupport",params)=="Increase");
    outStreamInfo().add(StreamInfo(in,1));
    return true;
}

template<typename Scalar>
struct filterNegativeOp {
  filterNegativeOp() {}
  const Scalar operator()(const Scalar& x) const { return x>0 ? x : 0; }
};

bool Flux::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	if ((out->tokenno()==0) && (in->tokenno()!=-1))
		in->prependZeros(1);
	if (!in->hasTokens(2))
		return false;

	const int N = in->info().size;
    double lastNorm = 0.0;
    double nextNorm = Map<VectorXd>(in->token(0),N).norm();
    while (in->hasTokens(2))
    {
    	Map<VectorXd> last(in->token(0),N);
        lastNorm = nextNorm;
        Map<VectorXd> next(in->token(1),N);
        nextNorm = next.norm();
        double* output = out->writeToken();
        if (lastNorm*nextNorm==0)
        	*output = 0.0;
        else if (m_onlyIncrease)
        	*output = (next-last).unaryExpr(filterNegativeOp<double>()).squaredNorm() / (lastNorm*nextNorm);
		else
			*output = (next - last).squaredNorm() / (lastNorm * nextNorm);

        in->consumeToken();
    }
    return true;
}

}
