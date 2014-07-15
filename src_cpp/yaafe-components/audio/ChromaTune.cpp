/*
 * ChromaBP.cpp
 *
 *  Created on: Dec 11, 2009
 *      Author: bmathieu
 */

#include "ChromaTune.h"
#include <Eigen/Dense>
#include <iostream>

using namespace std;
using namespace YAAFE;

namespace YAAFE_EXT {

ChromaTune::ChromaTune() {
	m_fmin = 0.0;
}

ChromaTune::~ChromaTune() {
}

ParameterDescriptorList ChromaTune::getParameterDescriptorList() const
{
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier = "CTInitDuration";
	p.m_description = "Duration on which perform chroma bias initialisation, in seconds.";
	p.m_defaultValue = "10";
	pList.push_back(p);

    p.m_identifier = "CQTMinFreq";
    p.m_description
            = "Minimal frequency of input CQT";
    p.m_defaultValue = "97.999";
    pList.push_back(p);

    p.m_identifier = "CQTBinsPerOctave";
    p.m_description = "Number of bins per octave of input CQT";
    p.m_defaultValue = "36";
    pList.push_back(p);

    return pList;
}

bool ChromaTune::init(const ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_fmin = getDoubleParam("CQTMinFreq",params);
	m_binsPerOctave = getIntParam("CQTBinsPerOctave",params);
	if (m_binsPerOctave%12!=0) {
		cerr << "ERROR: cannot initialize ChromaTune: CQTBinsPerOctave must be multiple of 12 !" << endl;
		return false;
	}
	m_initLength = (int) round(getDoubleParam("CTInitDuration",params) * in.sampleRate / in.sampleStep);

	outStreamInfo().add(StreamInfo(in, m_binsPerOctave));

	return true;
}

void ChromaTune::reset()
{
	m_binshift = -1;
}

bool ChromaTune::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;
	assert(out->info().size==m_binsPerOctave);

	if (m_binshift<0)
	{
		if (!in->hasTokens(m_initLength))
			return false;
		else {
			// init shift
			initshift(in,in->info().size,m_initLength);
		}
	}

	const int inSize = in->info().size;
	const int outSize = out->info().size;
	assert(inSize % outSize == 0);
	while (!in->empty()) {
		const double* inPtr = in->readToken();
		double* outPtr = out->writeToken();
		for (int i=0;i<outSize;i++)
			outPtr[i] = 0;
		int o = m_binshift;
		for (int i=0;i<inSize;i++)
		{
			outPtr[o] += inPtr[i];
			if (++o==outSize)
				o=0;
		}
		in->consumeToken();
	}
	return true;
}

void ChromaTune::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (m_binshift<0)
		initshift(in,in->info().size,in->availableTokens());
	process(inp,outp);
}

void ChromaTune::initshift(InputBuffer* in,int size, int nbFrames)
{
	m_binshift = 0;
	int bpo = m_binsPerOctave;
	assert(bpo%12 == 0);
	assert(size%bpo == 0);

	// init bias counting vars
	int bpst = bpo/12;
	int bias = 0;
	if (bpst!=1)
	{
		int biascount[bpst];
		for (int i=0;i<bpst;i++)
			biascount[i] = 0;

		// init chroma tmp vars
		double ch[bpo+2];

		for (int tok=0;tok<nbFrames;tok++)
		{
			const double* cq = in->token(tok);
			// compute chroma
			for (int i=0;i<bpo;i++)
				ch[i+1] = cq[i];
			for (int i=bpo;i<size;i++)
				ch[1+i%bpo] += cq[i];
			// report first and last bins
			ch[0] = ch[bpo];
			ch[bpo+1] = ch[1];
			// compute mean
			double mean = 0;
			for (int i=0;i<bpo;i++)
				mean += ch[1+i];
			mean /= size;
			// look at peaks
			for (int i=0;i<bpo;i++)
				if ((ch[i+1]>ch[i]) && (ch[i+1]>ch[i+2]) && (ch[i+1]>mean))
					biascount[i%bpst]++;
		}

		// compute bias as max
		int max=biascount[0];
		for (int i=1;i<bpst;i++)
			if (biascount[i]>max)
			{
				max = biascount[i];
				bias = i;
			}

		if (bias>((bpst)/2))
			m_binshift = bpst-bias;
		else
			m_binshift = -bias;

	} else {
		m_binshift = 0;
	}

	// compute shift from bias and fmin so that first bin is C (midi = 48)
	m_binshift -= bpst * (48 - (int) round(69 + 12*(log2(m_fmin/440))));
	m_binshift = m_binshift % m_binsPerOctave;
	if (m_binshift<0)
		m_binshift += m_binsPerOctave;
}

}
