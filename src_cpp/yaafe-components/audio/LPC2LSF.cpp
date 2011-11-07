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

#include "LPC2LSF.h"
#include "MathUtils.h"

using namespace std;

namespace YAAFE
{

LPC2LSF::LPC2LSF() :
    m_displacement(-1)
{
}

LPC2LSF::~LPC2LSF()
{
}

ParameterDescriptorList LPC2LSF::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "LSFNbCoeffs";
    p.m_description = "Number of Line Spectral Frequencies to compute";
    p.m_defaultValue = "10";
    pList.push_back(p);

    p.m_identifier = "LSFDisplacement";
    p.m_description = "LSF Displacement parameter: 1 for classical LSF, 0 for Schussler polynomials, >1 is a generalization";
    p.m_defaultValue = "1";
    pList.push_back(p);

    return pList;
}

StreamInfo LPC2LSF::init(const ParameterMap& params, const StreamInfo& in)
{
	m_displacement = getIntParam("LSFDisplacement",params);
	m_nbCoeffs = getIntParam("LSFNbCoeffs",params);

	return StreamInfo(in, m_nbCoeffs);
}

void LPC2LSF::processToken(double* lpc, const int inSize, double* lsf, const int outSize)
{
	int nbLPC = m_nbCoeffs - (m_displacement<2?1:m_displacement);
    assert(nbLPC<=inSize);
	bool hasNaN = false;
	for (int i=0;i<nbLPC;i++)
		if (std::isnan(lpc[i])) {
			hasNaN = true;
			break;
		}
	if (!hasNaN)
		a2lsf(lpc,m_displacement,lsf, m_nbCoeffs);
	else {
		for (int i=0;i<m_nbCoeffs;i++)
			lsf[i] = nan("undefined");
	}
}

}
