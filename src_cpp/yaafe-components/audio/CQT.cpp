/*
 * CQT.cpp
 *
 *  Created on: May 11, 2009
 *      Author: bmathieu
 */

#include "CQT.h"
#include "MathUtils.h"
#include <algorithm>
#include <unsupported/Eigen/FFT>
#include <math.h>

using namespace std;
using namespace YAAFE;
using namespace Eigen;

namespace YAAFE
{

CQT::CQT() :
    m_size(0), m_fftLen(0)
{
}

CQT::~CQT()
{
}

ParameterDescriptorList CQT::getParameterDescriptorList() const
{
    ParameterDescriptorList params;
    ParameterDescriptor p;

    p.m_identifier = "CQTMinFreq";
    p.m_description
            = "Minimal frequency. If <0.5 then assume it's a factor of sampleRate else assume it's expressed in Hertz.";
    p.m_defaultValue = "97.999";
    params.push_back(p);

    p.m_identifier = "CQTMaxFreq";
    p.m_description
            = "Maximum frequency. 0.5 then assume it's a factor of sampleRate else assume it's expressed in Hertz.";
    p.m_defaultValue = "0.5";
    params.push_back(p);

    p.m_identifier = "CQTBinsPerOctave";
    p.m_description = "Number of bins per octave to consider";
    p.m_defaultValue = "3";
    params.push_back(p);

    p.m_identifier = "CQTAlign";
    p.m_description = "Alignment of cqt kernels on analysis frame. 'l' to the left, 'c' to the center, 'r' to the right";
    p.m_defaultValue = "c";
    params.push_back(p);

    return params;
}

StreamInfo CQT::init(const ParameterMap& params, const StreamInfo& in)
{
    double sampleRate = in.sampleRate;
    double minFreq = getDoubleParam("CQTMinFreq",params);
    double maxFreq = getDoubleParam("CQTMaxFreq",params);
    int bins = getIntParam("CQTBinsPerOctave",params);
    string align = getStringParam("CQTAlign",params);

    if (minFreq <= 0.5)
        minFreq *= in.sampleRate;
    if (maxFreq <= 0.5)
        maxFreq *= in.sampleRate;


    double Q = 2.0 / (pow(2.0, (1.0 / bins)) - 1.0);
    int nbCoeffs = (int) ceil(bins * log2(maxFreq / minFreq));
    m_fftLen = nextpow2((int) ceil(Q * sampleRate / minFreq));

    if (nbCoeffs < 1)
    {
        cerr << "CQT: Error in parameters : minFreq > maxFreq !" << endl;
        //return false;
        return StreamInfo();
    }

	double thres = 0.0075;
	FFT<double> fftPlan;
	VectorXcd tempKernel(m_fftLen);
	VectorXcd specKernel(m_fftLen);
	VectorXcd posFreqs(m_fftLen/2+1);
	VectorXcd negFreqs(m_fftLen/2+1);
	negFreqs.setZero();
	for (int k = nbCoeffs; k > 0; k--)
	{
		double currentFreq = (minFreq * pow(2.0,((double) (k - 1)) / bins));
		int len = (int) ceil(Q * sampleRate / currentFreq);
		tempKernel.setZero();
		if (align=="c")
			len -= len%2;
		VectorXcd kernel(len);
		kernel.setZero();
		kernel.real()  = hammingPeriodic(len); // set real part
		kernel.real() /= kernel.real().sum();
		int index = 0;
		if (align=="l")
		{
			tempKernel.segment(0,len) = kernel;
		} else if (align == "c")
		{
			index = m_fftLen/2 - len/2;
			tempKernel.segment(index,len) = kernel;
		} else if (align == "r")
		{
			index = m_fftLen - len;
			tempKernel.segment(index,len) = kernel;
		} else {
			cerr << "CQT: invalid align parameter value: " << align << " (should be l|c|r)" << endl;
			// return false;
			return StreamInfo();
		}
		for (int i=0;i<m_fftLen;i++)
			tempKernel(i) *= complex<double>(cos((double)(2*PI*currentFreq*(i-index))/sampleRate),sin((double)(2*PI*currentFreq*(i-index))/sampleRate));
		fftPlan.fwd(specKernel,tempKernel,m_fftLen);
		// filter significative coeffs
		for (int i=0;i<m_fftLen;i++)
			if (abs(specKernel(i))<=thres)
				specKernel(i) = 0;
		// normalizes
		specKernel = specKernel.array().conjugate() / m_fftLen;
		// rearrange positive and negative freqs
		posFreqs.segment(0,m_fftLen/2+1) = specKernel.segment(0,m_fftLen/2+1);
		negFreqs.segment(1,m_fftLen/2-1) = specKernel.segment(m_fftLen/2+1,m_fftLen/2-1).reverse();

		int first = 0;
		// sparsify pos
		while (first<posFreqs.size() && (abs(posFreqs(first))==0))
		    first++;
		if (first<posFreqs.size())
		{
            int last = posFreqs.size() - 1;
            while (abs(posFreqs(last))==0 && last>first)
                last--;
            m_kernPosStart.push_back(first);
            m_kernPosFilters.push_back(VectorXcd(last-first+1));
            m_kernPosFilters.back() = posFreqs.segment(first,last-first+1);
		} else {
		    m_kernPosFilters.push_back(VectorXcd());
		    m_kernPosStart.push_back(-1);
		}
		// sparsify neg
		first = 0;
        while (first<negFreqs.size() && (abs(negFreqs(first))==0))
            first++;
        if (first<negFreqs.size())
        {
            int last = negFreqs.size() - 1;
            while (abs(negFreqs(last))==0 && last>first)
                last--;
            m_kernNegStart.push_back(first);
            m_kernNegFilters.push_back(VectorXcd(last - first+1));
            m_kernNegFilters.back() = negFreqs.segment(first,last-first+1);
        } else {
            m_kernNegFilters.push_back(VectorXcd());
            m_kernNegStart.push_back(-1);
        }
	}

	reverse(m_kernPosFilters.begin(), m_kernPosFilters.end());
	reverse(m_kernPosStart.begin(), m_kernPosStart.end());
	reverse(m_kernNegFilters.begin(), m_kernNegFilters.end());
    reverse(m_kernNegStart.begin(), m_kernNegStart.end());

    return StreamInfo(in,nbCoeffs);
}

void CQT::processToken(double* inPtr, const int inSize, double* outPtr, const int outSize)
{
	assert(outSize==m_kernPosFilters.size());
    int fftSize = inSize / 2;
	Map<ArrayXcd> inData((complex<double>*) inPtr, fftSize);
	for (size_t k = 0; k < m_kernPosFilters.size(); k++)
	{
		outPtr[k] = abs( (m_kernPosStart[k]!=-1 ? (inData.segment(m_kernPosStart[k],m_kernPosFilters[k].size()) * m_kernPosFilters[k]).sum() : 0) +
				         (m_kernNegStart[k]!=-1 ? (inData.segment(m_kernNegStart[k],m_kernNegFilters[k].size()).conjugate() * m_kernNegFilters[k]).sum() : 0));
	}
}

}
