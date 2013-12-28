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

#include "AutoCorrelation.h"
#include "MathUtils.h"

using namespace std;

namespace YAAFE
{

AutoCorrelation::AutoCorrelation()
{
}

AutoCorrelation::~AutoCorrelation()
{
}

ParameterDescriptorList AutoCorrelation::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;
    p.m_identifier = "ACNbCoeffs";
    p.m_description = "Number of autocorrelation coefficients to keep";
    p.m_defaultValue = "49";
    pList.push_back(p);
    return pList;
}

StreamInfo AutoCorrelation::init(const ParameterMap& params, const StreamInfo& in)
{
	return StreamInfo(in, getIntParam("ACNbCoeffs",params));
}

void AutoCorrelation::processToken(double* inPtr, const int inSize, double* outPtr, const int outSize)
{
	// compute several lags at same time improve cache management and increase speed
	const int lMax = outSize;
	int l=0;
	for (double* rout=outPtr;l<lMax-4;l+=4,rout+=4)
	{
	  double *rin1=inPtr;
	  double *rin2=&inPtr[l];
	  double lv[4] = {0,0,0,0};
	  int i = 0;
	  for (;i<inSize-l-4;i+=4,rin1+=4,rin2+=4)
	  {
		lv[0] += rin1[0]*rin2[0] + rin1[1]*rin2[1] + rin1[2]*rin2[2] + rin1[3]*rin2[3];
		lv[1] += rin1[0]*rin2[1] + rin1[1]*rin2[2] + rin1[2]*rin2[3] + rin1[3]*rin2[4];
		lv[2] += rin1[0]*rin2[2] + rin1[1]*rin2[3] + rin1[2]*rin2[4] + rin1[3]*rin2[5];
		lv[3] += rin1[0]*rin2[3] + rin1[1]*rin2[4] + rin1[2]*rin2[5] + rin1[3]*rin2[6];
	  }
	  for (int l1=0,l2=l;l1<4;l1++,l2++)
		for (int i1=i;i1<inSize-l2;i1++)
		  lv[l1] += inPtr[i1]*inPtr[i1+l2];
	  rout[0] = lv[0];
	  rout[1] = lv[1];
	  rout[2] = lv[2];
	  rout[3] = lv[3];
	}
	// compute last lags
	for (;l<lMax;l++)
	{
	  double l0 = 0;
	  for (int i=0;i<inSize-l;i++)
		l0 += inPtr[i]*inPtr[i+l];
	  outPtr[l] = l0;
	}
}

}
