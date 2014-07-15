/*
 * ChromaReduce.cpp
 *
 *  Created on: Dec 14, 2009
 *      Author: bmathieu
 */

#include "ChromaReduce.h"
#include <iostream>

#include <string.h>

using namespace std;
using namespace YAAFE;
using namespace Eigen;

namespace YAAFE_EXT {

ChromaReduce::ChromaReduce() {
	m_shift = 0;
}

ChromaReduce::~ChromaReduce() {
}

StreamInfo ChromaReduce::init(const ParameterMap& params, const StreamInfo& in)
{
	assert(in.size%12==0);
	if (in.size%12!=0) {
		cerr << "ERROR: invalid ChromaReduce input size " << in.size << " ! must be multiple of 12" << endl;
	}
	int bpst = in.size/12;
	if (bpst>1) {
		double extr = (double) (bpst-1)/2;
		m_gausswin = ((VectorXd::LinSpaced(bpst,-extr,bpst-1-extr) * 2.5 / extr).array().square() * (-0.5)).exp();
	} else {
		m_gausswin.resize(1);
		m_gausswin(0) = 1.0;
	}
	m_shift = (bpst - 1) / 2;

	return StreamInfo(in,12);
}

void ChromaReduce::processToken(double* inPtr, const int inSize, double* outPtr, const int outSize)
{
	assert(inSize%12==0);
	if (m_gausswin.size()>1) {
		MatrixXd tmp(inSize/12,12);
		memcpy(tmp.data(),inPtr + inSize - m_shift, m_shift*sizeof(double));
		memcpy(tmp.data() + m_shift, inPtr, (inSize-m_shift)*sizeof(double));
		Map<VectorXd> outData(outPtr,12);
		outData = m_gausswin * tmp;
		return;
	} else {
		assert(outSize==inSize);
		memcpy(outPtr,inPtr,inSize*sizeof(double));
	}
}

}
