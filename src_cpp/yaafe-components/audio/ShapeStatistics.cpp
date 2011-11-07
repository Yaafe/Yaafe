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

#include "ShapeStatistics.h"
#include "MathUtils.h"

using namespace std;
using namespace Eigen;

namespace YAAFE
{

ShapeStatistics::ShapeStatistics()
{
}

ShapeStatistics::~ShapeStatistics()
{
}

bool ShapeStatistics::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	outStreamInfo().add(StreamInfo(in,4));
    return true;
}

bool ShapeStatistics::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;
	const int N = in->info().size;

    while (!in->empty())
    {
        double* input = in->readToken();

        // compute moments
    	double moments[4] = { 0.0,0.0,0.0,0.0 };
    	{
			double dataSum = 0;
			for (int i=0;i<N;i++)
			{
				double v = abs(input[i]);
				dataSum += v;
				v *= i;
				moments[0] += v;
				v *= i;
				moments[1] += v;
				v *= i;
				moments[2] += v;
				v *= i;
				moments[3] += v;
			}
			if (dataSum==0)
				dataSum = EPS;
			moments[0] /= dataSum;
			moments[1] /= dataSum;
			moments[2] /= dataSum;
			moments[3] /= dataSum;
    	}

        double* output = out->writeToken();
    	// centroid
        output[0] = moments[0];
        // spread
        output[1] = sqrt(moments[1] - pow2(moments[0]));
        if (output[1] == 0)
            output[1] = EPS;
        // skewness
        output[2] = (2 * pow3(moments[0]) - 3 * moments[0]
                * moments[1] + moments[2]) / pow3(output[1]);
        // kurtosis
        output[3] = (-3 * pow4(moments[0]) + 6 * moments[0]
                * moments[1] - 4 * moments[0] * moments[2] + moments[3])
                / pow4(output[1]) - 3;

        in->consumeToken();
    }
    return true;
}

}

