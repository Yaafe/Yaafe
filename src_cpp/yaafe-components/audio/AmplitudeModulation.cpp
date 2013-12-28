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

#include "AmplitudeModulation.h"

#include "MathUtils.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>

#define TREM_FREQ 0
#define TREM_STREN 1
#define TREM_STREN_HEUR 2
#define TREM_PROD 3
#define GRAN_FREQ 4
#define GRAN_STREN 5
#define GRAN_STREN_HEUR 6
#define GRAN_PROD 7

#define TREM_HZ_START 4.0
#define TREM_HZ_END 8.0
#define GRAIN_HZ_START 10.0
#define GRAIN_HZ_END 40.0

using namespace std;
using namespace Eigen;

namespace YAAFE
{

class AmplitudeModulation::ComputingContext
{
public:
    ComputingContext(const StreamInfo& in, int decim);
    virtual ~ComputingContext() {};

    int m_blockSize;
    int m_decim;
    int m_ncx;
    int m_tremStart;
    int m_tremSize;
    int m_granStart;
    int m_granSize;
    VectorXd m_envHanning;
    VectorXd m_ek;
    FFT<double> m_fftPlan;
};

AmplitudeModulation::ComputingContext::ComputingContext(const StreamInfo& in, int decim) :
    m_blockSize(in.frameLength), m_decim(decim), m_ek(in.frameLength/2), m_fftPlan()
{
    m_ncx = in.frameLength / decim + 1;
    // compute band boundaries
    m_tremStart = (int) ceil(TREM_HZ_START * (in.frameLength) * decim / in.sampleRate) - 1;
    m_tremSize = (int) ceil(TREM_HZ_END * (in.frameLength) * decim / in.sampleRate) - m_tremStart;
    m_granStart = (int) ceil(GRAIN_HZ_START * (in.frameLength) * decim / in.sampleRate) - 1;
    m_granSize = (int) ceil(GRAIN_HZ_END * (in.frameLength) * decim / in.sampleRate) - m_granStart;

    // compute hanning window for enveloppe
    m_envHanning = ehanning(m_ncx);

    m_fftPlan.SetFlag(Eigen::FFT<double>::HalfSpectrum);
}

AmplitudeModulation::AmplitudeModulation() :
    m_context(0)
{
}

AmplitudeModulation::~AmplitudeModulation()
{
    if (m_context)
        delete m_context;
}

ParameterDescriptorList AmplitudeModulation::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "EnDecim";
    p.m_description = "Decimation factor to compute envelope";
    p.m_defaultValue = "200";
    pList.push_back(p);

    return pList;

}

bool AmplitudeModulation::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

    int decim = getIntParam("EnDecim",params);
    m_context = new ComputingContext(in, decim);
    outStreamInfo().add(StreamInfo(in, 8));
    return true;
}

bool AmplitudeModulation::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

    assert(in->info().size==m_context->m_ncx);
	VectorXd fftinput(in->info().frameLength);
	VectorXcd fftoutput(in->info().frameLength/2+1);
	for (;!in->empty();in->consumeToken())
	{
		double* values = out->writeToken();

		// compute ek
		Map<VectorXd> env(in->readToken(),in->info().size);

		fftinput.segment(0,m_context->m_ncx) = env.array() * m_context->m_envHanning.array();
		fftinput.segment(m_context->m_ncx,fftinput.size()-m_context->m_ncx).setZero();
		m_context->m_fftPlan.fwd(fftoutput,fftinput);
		m_context->m_ek = fftoutput.segment(0, m_context->m_ek.size()).array().abs();
		double mean_ek = m_context->m_ek.mean();

		// compute tremolo features on ek
		{
		        VectorXd::Index index;
			m_context->m_ek.segment(m_context->m_tremStart,m_context->m_tremSize).maxCoeff(&index);
			index += m_context->m_tremStart;
			double max = m_context->m_ek(index);
			values[TREM_FREQ] = index * in->info().sampleRate / (in->info().frameLength
					* m_context->m_decim);
			values[TREM_STREN] = max - mean_ek;
			if (values[TREM_STREN] < 0)
				values[TREM_STREN] = 0.0;
			values[TREM_STREN_HEUR] = max - m_context->m_ek.segment(m_context->m_tremStart,m_context->m_tremSize).mean();
			if (values[TREM_STREN_HEUR] < 0)
				values[TREM_STREN_HEUR] = 0.0;
			values[TREM_PROD] = values[TREM_FREQ]
					* values[TREM_STREN];
		}

		// compute grain features on ek
		{
		        VectorXd::Index index;
			m_context->m_ek.segment(m_context->m_granStart,m_context->m_granSize).maxCoeff(&index);
			index += m_context->m_granStart;
			double max = m_context->m_ek(index);
			values[GRAN_FREQ] = (double) index * in->info().sampleRate / (in->info().frameLength
					* m_context->m_decim);
			values[GRAN_STREN] = max - mean_ek;
			if (values[GRAN_STREN] < 0)
				values[GRAN_STREN] = 0.0;
			values[GRAN_STREN_HEUR] = max - m_context->m_ek.segment(m_context->m_granStart,m_context->m_granSize).mean();
			if (values[GRAN_STREN_HEUR] < 0)
				values[GRAN_STREN_HEUR] = 0.0;
			values[GRAN_PROD] = values[GRAN_FREQ]
					* values[GRAN_STREN];
		}
	}
    return true;
}

}
