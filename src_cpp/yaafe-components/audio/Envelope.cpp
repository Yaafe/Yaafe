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

#include "Envelope.h"
#include "MathUtils.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/FFT>
#include <string.h>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

class Envelope::ComputingContext
{
public:
    ComputingContext(double sampleRate, int blockSize,int decim);
    virtual ~ComputingContext() {};

    int m_blockSize;
    FFT<double> m_fftForward;
    FFT<double> m_fftBackward;

    VectorXd m_filter;
    int m_decim;
    int m_filterDecSize;
    int m_envSize;
};

Envelope::ComputingContext::ComputingContext(double sampleRate, int blockSize,int decim) :
    m_blockSize(blockSize), m_fftForward(), m_fftBackward(), m_decim(decim)
{
    // compute filter window
    int wlen = (int) round(0.05 * sampleRate);
    VectorXd han(ehanning(2 * wlen));
    m_envSize = (int)ceil((double)blockSize / (double)decim);
    m_filter.resize((int)ceil(wlen /(double)decim) * decim);
    m_filter.segment(0,wlen) = han.segment(wlen,wlen);
    m_filter.segment(0,wlen) /= m_filter.segment(0,wlen).sum();
    if (m_filter.size() > wlen)
        m_filter.segment(wlen,m_filter.size()-wlen).setZero();
    m_filterDecSize = m_filter.size()/decim;
}

Envelope::Envelope() : m_context(0)
{
}

Envelope::~Envelope()
{
    if (m_context)
        delete m_context;
}

ParameterDescriptorList Envelope::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "EnDecim";
    p.m_description = "Decimation factor to compute envelope";
    p.m_defaultValue = "200";
    pList.push_back(p);

    return pList;
}

bool Envelope::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	int decim = getIntParam("EnDecim",params);
    m_context = new ComputingContext(in.sampleRate, in.size, decim);

    outStreamInfo().add(StreamInfo(in,m_context->m_envSize));
    return true;
}

bool Envelope::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	assert(out->info().size==m_context->m_envSize);

	const int N = in->info().size;
	VectorXcd fftOut(N);
	fftOut.setZero();
	VectorXcd hilbert(N);
	hilbert.setZero();
	ArrayXd absHilbert(m_context->m_envSize*m_context->m_decim);
	absHilbert.setZero();
	while (!in->empty())
	{
		Map<VectorXd> inData(in->readToken(),N);
		m_context->m_fftForward.fwd(fftOut,inData);
		fftOut.segment(1,N/2-1) *= 2.0;
		fftOut.segment(N/2+1,N/2-1).setZero();
		m_context->m_fftBackward.inv(hilbert,fftOut);
		absHilbert.segment(0,N) = hilbert.array().abs();

		// filter with decimation
		const int decim = m_context->m_decim;
		double* outPtr = out->writeToken();
		for (int i=0;i<out->info().size;i++)
		{
			const double* h = absHilbert.data() + max(0,i-m_context->m_filterDecSize+1)*decim;
			const double* hend = absHilbert.data() + (i+1)*decim;
			const double* f = m_context->m_filter.data() + max(m_context->m_filterDecSize-1-i,0)*decim;
			double sum = 0;
			for (;h!=hend;)
				sum += *h++ * *f++;
			outPtr[i] = sum;
		}

		in->consumeToken();
	}

    return true;
}

}
