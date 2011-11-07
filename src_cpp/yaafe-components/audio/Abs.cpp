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

#include "Abs.h"
//#include "yaafe-core/NumericUtils.h"
#include <Eigen/Dense>

#define EPS 2.220446049250313e-16

using namespace std;
//using namespace blitz;
using namespace Eigen;


namespace YAAFE
{

Abs::Abs()
{
}

Abs::~Abs()
{
}

StreamInfo Abs::init(const ParameterMap& params, const StreamInfo& in)
{
    if (in.size%2 != 0)
    {
        cerr << "ERROR: Abs input size should be even" << endl;
        return StreamInfo();
    }
    return StreamInfo(in, in.size/2);
}

inline double nonZeroNorm(double r, double i)
{
    double n = std::sqrt(r*r + i*i);
    return n ? n : EPS;
}

void Abs::processToken(double* inData, const int inSize, double* outData, const int outSize)
{
	for (int i=0;i<outSize;++i)
		outData[i] = nonZeroNorm(inData[2*i],inData[2*i+1]);
}

}
