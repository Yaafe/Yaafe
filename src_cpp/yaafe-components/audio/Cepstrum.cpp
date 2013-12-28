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

#include "Cepstrum.h"

#include "MathUtils.h"
#include <iostream>
#include <string.h>


using namespace std;
using namespace Eigen;

namespace YAAFE
{

Cepstrum::Cepstrum()
{
}

Cepstrum::~Cepstrum()
{
}

ParameterDescriptorList Cepstrum::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "CepsNbCoeffs";
    p.m_description = "Number of cepstral coefficient to keep.";
    p.m_defaultValue = "13";
    pList.push_back(p);

    p.m_identifier = "CepsIgnoreFirstCoeff";
    p.m_description = "0 keeps the first cepstral coeffcient, 1 ignore it";
    p.m_defaultValue = "1";
    pList.push_back(p);

    return pList;
}

bool Cepstrum::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_ignoreFirst = getIntParam("CepsIgnoreFirstCoeff",params);
	m_nbCoeffs = getIntParam("CepsNbCoeffs",params);
	if (m_nbCoeffs+m_ignoreFirst>in.size)
	{
		cerr << "Warning: cannot compute " << m_nbCoeffs << " for input of size " << in.size << endl;
		m_nbCoeffs = in.size - m_ignoreFirst;
		cerr << "compute only " << m_nbCoeffs << " coefficients" << endl;
	}

	m_dctPlan.resize(in.size,in.size);
	for (int j=0;j<in.size;j++)
		m_dctPlan(0,j) = 1.0 / sqrt((double)in.size);
	for (int i=1;i<in.size;i++)
		for (int j=0;j<in.size;j++)
			m_dctPlan(i,j) = sqrt(2.0 / in.size) * cos(PI * (j + 0.5) * i / in.size);

	outStreamInfo().add(StreamInfo(in, m_nbCoeffs));
    return true;
}

template<typename Scalar>
struct safeLogOp {
  safeLogOp() : m_logeps(log(EPS)) {};
  const Scalar operator()(const Scalar& x) const { return x>0 ? log10(x) : m_logeps; };
  Scalar m_logeps;
};

bool Cepstrum::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty())
		return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

    safeLogOp<double> slop;
    VectorXd outDct;
    while (!in->empty())
    {
        Map<VectorXd> inData(in->readToken(),in->info().size);
        outDct.noalias() = m_dctPlan * inData.unaryExpr(slop);
        memcpy(out->writeToken(),outDct.data() + m_ignoreFirst, out->info().size*sizeof(double));
        in->consumeToken();
    }
    return true;
}

}
