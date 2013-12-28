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

#include "LoudnessSharpness.h"
#include "SpecificLoudness.h"

using namespace std;
using namespace Eigen;

namespace YAAFE
{

LoudnessSharpness::LoudnessSharpness()
{
}

LoudnessSharpness::~LoudnessSharpness()
{
}

bool LoudnessSharpness::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	if (in.size != NB_BARK_BANDS)
    {
        cerr << "Error : LoudnessSharpness should receive an input of size "
                << NB_BARK_BANDS
                << " (number of Bark bands for Loudness computation" << endl;
        return false;
    }

    m_coeffs.resize(NB_BARK_BANDS);
	for (int i = 0; i < NB_BARK_BANDS; i++)
	{
		if (i < 14)
		m_coeffs(i) = 0.11 * (i + 1);
		else
		m_coeffs(i) = 0.11 * (i + 1) * 0.066 * exp(0.171 * (i + 1));
	}

	outStreamInfo().add(StreamInfo(in,1));
    return true;
}

bool LoudnessSharpness::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	while (!in->empty()) {
		Map<VectorXd> loudness(in->readToken(),in->info().size);
		double* output = out->writeToken();
		*output = m_coeffs * loudness;
		in->consumeToken();
	}
	return true;
}


}
