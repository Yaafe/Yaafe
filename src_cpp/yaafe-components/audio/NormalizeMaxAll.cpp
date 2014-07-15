/*
 * NormalizeMaxAll.cpp
 *
 *  Created on: Jan 5, 2010
 *      Author: bmathieu
 */

#include "NormalizeMaxAll.h"
#include <Eigen/Dense>

using namespace std;
using namespace YAAFE;
using namespace Eigen;


namespace YAAFE_EXT {

NormalizeMaxAll::NormalizeMaxAll() : m_nbFrames(-1) {
}

NormalizeMaxAll::~NormalizeMaxAll() {
}

ParameterDescriptorList NormalizeMaxAll::getParameterDescriptorList() const {
	ParameterDescriptorList pList;
	ParameterDescriptor p;

	p.m_identifier = "NMANbFrames";
	p.m_description = "Number of frames to normalize together, -1 means all frames";
	p.m_defaultValue = "-1";
	pList.push_back(p);

	return pList;
}

bool NormalizeMaxAll::init(const ParameterMap& params, const YAAFE::Ports<YAAFE::StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_nbFrames = getIntParam("NMANbFrames",params);

	outStreamInfo().add(in);

	return true;
}

void NormalizeMaxAll::processBlock(InputBuffer* in, OutputBuffer* out, int nbFrames)
{
	if (nbFrames<=0)
		return;
	assert(in->hasTokens(nbFrames));
	assert(in->info().size==out->info().size);
	const int size = in->info().size;
	double maxvalue = *(in->readToken());
	for (int i=0;i<nbFrames;i++) {
		Map<ArrayXd> inData(in->token(i),size);
		double m = inData.maxCoeff();
		if (m>maxvalue)
			maxvalue = m;
	}
	for (int i=0;i<nbFrames;i++) {
		Map<ArrayXd> inData(in->readToken(),size);
		Map<ArrayXd> outData(out->writeToken(),size);
		outData = inData / maxvalue;
		in->consumeToken();
	}
}

bool NormalizeMaxAll::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	if (m_nbFrames<0 || !in->hasTokens(m_nbFrames))
		return false;
	while (in->hasTokens(m_nbFrames))
		processBlock(in,out,m_nbFrames);
	return true;
}

void NormalizeMaxAll::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	process(inp,outp);
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;
	processBlock(in,out,in->availableTokens());
}


}
