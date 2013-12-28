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

#include "HistogramIntegrator.h"
#include "math.h"

using namespace std;

namespace YAAFE
{

HistogramIntegrator::HistogramIntegrator() :
    m_nbFrames(0)
{
}

HistogramIntegrator::~HistogramIntegrator()
{
}

ParameterDescriptorList HistogramIntegrator::getParameterDescriptorList() const
{
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "NbFrames";
    p.m_description = "Number of frames to integrate together";
    p.m_defaultValue = "60";
    pList.push_back(p);

    p.m_identifier = "StepNbFrames";
    p.m_description = "Number of frames to skip between two integration";
    p.m_defaultValue = "30";
    pList.push_back(p);

    p.m_identifier = "HInf";
    p.m_description = "Minimal value to take into consideration";
    p.m_defaultValue = "0";
    pList.push_back(p);

    p.m_identifier = "HSup";
    p.m_description = "Maximal value to take into consideration";
    p.m_defaultValue = "1";
    pList.push_back(p);

    p.m_identifier = "HNbBins";
    p.m_description = "Nb bins of histogram";
    p.m_defaultValue = "10";
    pList.push_back(p);

    p.m_identifier = "HWeighted";
    p.m_description = "Set it to 1 if input values are weighted. If 1, input is considered to be a list of couple (value,weight).";
    p.m_defaultValue = "0";
    pList.push_back(p);

    return pList;
}

bool HistogramIntegrator::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	m_nbFrames = getIntParam("NbFrames",params);
    if (m_nbFrames<=0) {
    	cerr << "ERROR: Invalid NbFrames parameter !" << endl;
    	return false;
    }
	m_stepNbFrames = getIntParam("StepNbFrames",params);
    if (m_stepNbFrames<=0) {
    	cerr << "ERROR: Invalid stepNbFrames parameter !" << endl;
    	return false;
    }

	m_hinf = getDoubleParam("HInf",params);
	m_hsup = getDoubleParam("HSup",params);
	m_nbbins = getIntParam("HNbBins",params);
	m_hstep = (m_hsup-m_hinf) / m_nbbins;
	m_weigthed = (getIntParam("HWeighted",params)==1);

    assert(!m_weigthed || ((in.size%2)==0));

    StreamInfo out;
    out.size = m_nbbins;
    out.sampleRate = in.sampleRate;
    out.frameLength = in.frameLength + (m_nbFrames-1)*in.sampleStep;
    out.sampleStep = m_stepNbFrames*in.sampleStep;
    outStreamInfo().add(out);

    return true;
}

bool HistogramIntegrator::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	if ((out->tokenno()==0) && (in->tokenno()!=-m_nbFrames/2))
	{
		in->prependZeros(m_nbFrames/2);
	}
	if (!in->hasTokens(m_nbFrames)) return false;

	double* inData = new double[in->info().size*m_nbFrames];
    while (in->hasTokens(m_nbFrames))
    {
    	in->read(inData,m_nbFrames);
        computeHistogram(inData,m_nbFrames*in->info().size,out->writeToken());
        in->consumeTokens(m_stepNbFrames);
    }
    delete [] inData;
    return true;
}

void HistogramIntegrator::computeHistogram(const double* inData, int inSize, double* outData)
{
    for (int i=0;i<m_nbbins;i++)
        outData[i] = 0;

    const double* inPtr = inData;
    const double* inPtrEnd = inData + inSize;
    for (;inPtr!=inPtrEnd;inPtr++)
    {
        if (*inPtr>=m_hinf && *inPtr<m_hsup)
        {
            outData[(int)floor((inPtr[0] - m_hinf) / m_hstep)] += (m_weigthed ? inPtr[1] : 1);
        }
        if (m_weigthed)
            inPtr++;
    }
}

void HistogramIntegrator::flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out)
{
	in[0].data->appendZeros((m_nbFrames-1)/2);
	process(in,out);
}

}
