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

#include "Normalize.h"
#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

#define NORM_SUM 0
#define NORM_EUCLIDEAN 1
#define NORM_MAX 2

namespace YAAFE
{

Normalize::Normalize() : m_mode(NORM_SUM)
{
}

Normalize::~Normalize()
{
}

ParameterDescriptorList Normalize::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "NNorm";
    p.m_description = "Type of normalization to apply within Sum (L1-norm), Euclidean (L2-norm), Max (maximum-norm)";
    p.m_defaultValue = "Sum";
    pList.push_back(p);

    return pList;
}

bool Normalize::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	if (in.size<=1) {
		cerr << "WARNING: trying to normalize vector of size " << in.size << endl;
	}

	string norm = getStringParam("NNorm",params);
	if (norm=="Sum")
		m_mode = NORM_SUM;
	else if (norm=="Euclidean")
		m_mode = NORM_EUCLIDEAN;
	else if (norm=="Max")
		m_mode = NORM_MAX;
	else {
		cerr << "ERROR: Invalid NNorm parameter: " << norm << endl;
		return false;
	}

	outStreamInfo().add(in);
	return true;
}

bool Normalize::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty())
		return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	const int size = in->info().size;
	if (m_mode==NORM_SUM) {
		while (!in->empty()) {
			Map<ArrayXd> inData(in->readToken(),size);
			Map<ArrayXd> outData(out->writeToken(),size);
			outData = inData / inData.sum();
			in->consumeToken();
		}
	} else if (m_mode==NORM_EUCLIDEAN) {
		while (!in->empty()) {
			Map<VectorXd> inData(in->readToken(),size);
			Map<VectorXd> outData(out->writeToken(),size);
			outData.noalias() = inData / inData.norm();
			in->consumeToken();
		}
	} else if (m_mode==NORM_MAX) {
		while (!in->empty()) {
			Map<ArrayXd> inData(in->readToken(),size);
			Map<ArrayXd> outData(out->writeToken(),size);
			outData = inData / inData.abs().maxCoeff();
			in->consumeToken();
		}
	}
	return true;
}

}
