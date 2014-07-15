/*
 * ChromaZhu.cpp
 *
 *  Created on: May 12, 2009
 *      Author: bmathieu
 */

#include "Chroma2.h"

using namespace std;
using namespace YAAFE;
using namespace Eigen;

namespace YAAFE_EXT
{

Chroma2::Chroma2() : m_cqtSize(0)
{
}

Chroma2::~Chroma2()
{
}

ParameterDescriptorList Chroma2::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "CQTMinFreq";
    p.m_description = "inherited from CQT";
    p.m_defaultValue = "27.5";
    pList.push_back(p);

    p.m_identifier = "CQTMaxFreq";
    p.m_description = "inherited from CQT";
    p.m_defaultValue = "3520";
    pList.push_back(p);

    p.m_identifier = "CQTBinsPerOctave";
    p.m_description = "inherited from CQT";
    p.m_defaultValue = "48";
    pList.push_back(p);

    p.m_identifier = "CZBinsPerSemitone";
    p.m_description = "number of bins per semitone for the PCP";
    p.m_defaultValue = "1";
    pList.push_back(p);

    p.m_identifier = "CZNbCQTBinsAggregatedToPCPBin";
    p.m_description
            = "number of CQT bins which are aggregated for each PCP bin. if -1 then use CQTBinsPerOctave / 24";
    p.m_defaultValue = "-1";
    pList.push_back(p);

    p.m_identifier = "CZTuning";
    p.m_description = "frequency of the A4, in Hz.";
    p.m_defaultValue = "440";
    pList.push_back(p);

    return pList;
}

StreamInfo Chroma2::init(const ParameterMap& params, const StreamInfo& in)
{
	m_cqtSize = in.size;
	m_cqtMinFreq = getDoubleParam("CQTMinFreq",params);
	m_cqtMaxFreq = getDoubleParam("CQTMaxFreq",params);
	m_cqtNbBins = getIntParam("CQTBinsPerOctave",params);
	m_nbBinsSemitone = getIntParam("CZBinsPerSemitone",params);
	m_nbBins = getIntParam("CZNbCQTBinsAggregatedToPCPBin",params);
	if (m_nbBins<0)
		m_nbBins = m_cqtNbBins / 24;
	m_tuning = getDoubleParam("CZTuning",params);

	m_deviation = m_cqtNbBins * log2(m_tuning / m_cqtMinFreq);
	double deviationRealPart = m_deviation - floor(m_deviation);
	m_cqtMinFreq = m_cqtMinFreq * pow(2.0, deviationRealPart / m_cqtNbBins);
	m_tuningBin = 1 + ((int) floor(m_deviation) - 1) % (m_cqtNbBins / 12);
	m_Q = 1 / (pow(2.0, 1.0 / (double) m_cqtNbBins) - 1);
	m_K = log2(m_cqtMaxFreq / m_cqtMinFreq);
	m_nbNote = (int) floor(m_cqtSize * 12.0 * m_nbBinsSemitone / m_cqtNbBins);
	m_notePartial.resize(m_nbNote);
	m_notePartialA = (int) floor((double) m_nbBins / 2.0);
	if (m_tuningBin > (m_cqtNbBins / 12 - m_notePartialA))
		m_tuningBin = m_tuningBin - m_cqtNbBins / 12;
	m_notePartialFactor = m_cqtNbBins / (12 * m_nbBinsSemitone);
	m_pcpSize = 12 * m_nbBinsSemitone;
	m_pcp.resize(m_pcpSize);

	int firstCode = (int) round(m_nbBinsSemitone * (69.0 + 12.0 * log2(m_cqtMinFreq
			* pow(2.0, ((double)m_tuningBin / m_cqtNbBins)) / m_tuning )));
	m_pcpShift = firstCode % m_pcpSize;

    return StreamInfo(in,m_pcpSize);
}

void Chroma2::processToken(double* inData, const int inSize, double* pcp, const int outSize)
{
	assert(m_cqtSize==inSize);
	// get cqt
	Map<VectorXd> cqt(inData,m_cqtSize);

	// compute tuning frequency need cqt of all frames.
	// we can't do it within this framework

	// Extract "note partials"
	for (int i = 0; i < m_nbNote; i++)
	{
		int c = m_notePartialFactor * i + m_tuningBin;
		if ((c < -m_notePartialA) || c > m_cqtSize - 1
				+ m_notePartialA)
			m_notePartial( i) = 0;
		else {
//			m_notePartial( i) = max(cqt(Range(max(1, c
//					- m_notePartialA), min(m_cqtSize - 1,
//				c + m_notePartialA))));
			int start = max(1,c-m_notePartialA);
			int stop = min(m_cqtSize,c+m_notePartialA+1);
			m_notePartial(i) = cqt.segment(start,stop-start).maxCoeff();
		}
	}

	// compute pcp
	int notePartialRange = m_nbNote - m_pcpSize;
	for (int i = 0; i < m_pcpSize; i++)
	{
		double s = 0;
		for (int n=i;n<(i+notePartialRange);n+=m_pcpSize)
			s += m_notePartial(n);
		pcp[(i + m_pcpShift) % m_pcpSize] = s;
	}
}

}
