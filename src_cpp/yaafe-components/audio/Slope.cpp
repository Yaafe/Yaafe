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

#include "Slope.h"
#include "MathUtils.h"

using namespace Eigen;
using namespace std;

namespace YAAFE
{

Slope::Slope() :
    m_size(0)
{
}

Slope::~Slope()
{
}

bool Slope::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_size = in.size;
	m_freqs.resize(m_size);
	m_freqs = VectorXd::LinSpaced(m_size,0,m_size-1) * (in.sampleRate) / in.frameLength;
	m_sumFreqs = m_freqs.sum();
	m_slopeNorm = m_size * m_freqs.array().square().sum() - pow2(m_freqs.sum());

	outStreamInfo().add(StreamInfo(in,1));
    return true;
}

bool Slope::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

    while (!in->empty())
    {
    	Map<VectorXd> spec(in->readToken(),m_size);
    	double sumSpec = spec.sum();
    	sumSpec = (m_size * spec.dot(m_freqs) - m_sumFreqs * sumSpec) / (sumSpec * m_slopeNorm);
    	out->write(&sumSpec,1);
        in->consumeToken();
    }
    return true;
}

}
