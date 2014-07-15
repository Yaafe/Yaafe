/*
 * AdvancedFrameTokenizer.cpp
 *
 *  Created on: Feb 1, 2011
 *      Author: bmathieu
 */

#include "AdvancedFrameTokenizer.h"
#include <iostream>
#include <cmath>

using namespace std;

namespace YAAFE {

AdvancedFrameTokenizer::AdvancedFrameTokenizer() :
		m_blockSize(0), m_outStepSize(0), m_outSampleRate(0)
{}

AdvancedFrameTokenizer::~AdvancedFrameTokenizer()
{}

ParameterDescriptorList AdvancedFrameTokenizer::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "blockSize";
    p.m_description = "output frames size";
    p.m_defaultValue = "1024";
    pList.push_back(p);

    p.m_identifier = "outStepSize";
    p.m_description = "step between consecutive frames according to the forced output sampleRate";
    p.m_defaultValue = "512";
    pList.push_back(p);

    p.m_identifier = "outSampleRate";
    p.m_description = "Force output samplerate to given value";
    p.m_defaultValue = "16000";
    pList.push_back(p);

    return pList;
}

bool AdvancedFrameTokenizer::init(const YAAFE::ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

    if (in.size > 1)
    {
        cerr << "ERROR: input of AdvancedFrameTokenizer should be of size 1" << endl;
        return false;
    }

	m_outSampleRate = getDoubleParam("outSampleRate",params);
	m_blockSize = getIntParam("blockSize",params);
	m_outStepSize = getIntParam("outStepSize",params);

	StreamInfo out;
	out.frameLength = (int) floor(in.frameLength*m_blockSize*m_outSampleRate/in.sampleRate + 0.5);
	out.sampleRate = m_outSampleRate;
	out.sampleStep = m_outStepSize;
	out.size = m_blockSize;
	outStreamInfo().add(out);

	return true;
}

bool AdvancedFrameTokenizer::process(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;
	assert(in->size()==1);

	if ((out->tokenno()==0) && (in->tokenno()!=-m_blockSize/2))
		in->prependZeros(m_blockSize/2);

	if (!in->hasTokens(m_blockSize))
		return false;

	while (in->hasTokens(m_blockSize)) {
		in->read(out->writeToken(),m_blockSize);
		int nextInputToken = (int) floor(out->tokenno() * m_outStepSize * in->info().sampleRate / m_outSampleRate + 0.5);
		in->consumeTokens(nextInputToken - m_blockSize/2 - in->tokenno());
	}

	return true;
}

void AdvancedFrameTokenizer::flush(YAAFE::Ports<YAAFE::InputBuffer*>& inp, YAAFE::Ports<YAAFE::OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	in->appendZeros((m_blockSize-1)/2);
	process(inp,outp);
}

} // namespace YAAFE
