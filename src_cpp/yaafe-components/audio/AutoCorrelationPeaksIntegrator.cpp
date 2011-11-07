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

#include "AutoCorrelationPeaksIntegrator.h"

#include <Eigen/Dense>
#include <string.h>

using namespace Eigen;
using namespace std;

namespace YAAFE
{

AutoCorrelationPeaksIntegrator::AutoCorrelationPeaksIntegrator() : m_nbFrames(0)
{
}

AutoCorrelationPeaksIntegrator::~AutoCorrelationPeaksIntegrator()
{
}

ParameterDescriptorList AutoCorrelationPeaksIntegrator::getParameterDescriptorList() const
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

    p.m_identifier = "ACPNbPeaks";
    p.m_description = "Number of autocorrelation peaks to keep";
    p.m_defaultValue = "3";
    pList.push_back(p);

    p.m_identifier = "ACPInterPeakMinDist";
    p.m_description = "Minimal distance between consecutive autocorrelation peaks, expressed in lags.";
    p.m_defaultValue = "5";
    pList.push_back(p);

    p.m_identifier = "ACPNorm";
    p.m_description = "can be No|BPM|Hz. Normalize output to be expressed respectively in lag, BPM, Hz";
    p.m_defaultValue = "No";
    pList.push_back(p);

    return pList;

}


bool AutoCorrelationPeaksIntegrator::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
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

	m_nbPeaks = getIntParam("ACPNbPeaks",params);
	m_interPeakMinDist = getIntParam("ACPInterPeakMinDist",params);
	m_normFactor = 0;
	if (getStringParam("ACPNorm",params)=="BPM")
		m_normFactor = 60;
	else if (getStringParam("ACPNorm",params)=="Hz")
		m_normFactor = 1;

	StreamInfo out;
	out.size = 2*m_nbPeaks*in.size;
	out.sampleRate = in.sampleRate;
	out.sampleStep = m_stepNbFrames * in.sampleStep;
	out.frameLength = in.frameLength + (m_nbFrames-1) * in.sampleStep;
	outStreamInfo().add(out);

    return true;
}

bool AutoCorrelationPeaksIntegrator::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	if ((out->tokenno()==0) && (in->tokenno()!=-m_nbFrames/2))
		in->prependZeros(m_nbFrames/2);
	if (!in->hasTokens(m_nbFrames)) return false;

    while (in->hasTokens(m_nbFrames))
    {
    	ArrayXXd inData(in->info().size,m_nbFrames);
    	in->read(inData.data(),m_nbFrames);
    	double* outPtr = out->writeToken();

        for (int i = 0; i < in->info().size; i++)
        {
            findAutocorrPeaks(inData.data() + i,m_nbFrames,in->info().size,outPtr + (2*m_nbPeaks*i));
        }

        if (m_normFactor)
        {
            // normalize indices
//            DVec outIndices(outPtr,in->info().size*m_nbPeaks,2);
//            outIndices = where(outIndices>0,(m_normFactor * in->info().sampleRate / (double)in->info().sampleStep) / outIndices,0);
            double norm = m_normFactor * in->info().sampleRate / (double)in->info().sampleStep;
            for (int i=0;i<in->info().size*m_nbPeaks;i++)
            	outPtr[2*i] = (outPtr[2*i]>0) ? norm / outPtr[2*i] : 0;
        }
        in->consumeTokens(m_stepNbFrames);
    }
    return true;
}

void AutoCorrelationPeaksIntegrator::findAutocorrPeaks(double* data, int len, int stride, double* out)
{
    Map<ArrayXd> inData(data,len,stride);

    // set out to 0
    for (int i=0;i<2*m_nbPeaks;i++)
    	out[i] = 0.0;

    // compute autocorr
    ArrayXd corr(len);
    for (int j=0;j<len;j++)
        corr(j) = (inData.segment(0,len-j) * inData.segment(j,len-j)).sum();
    corr /= corr(0);

    // set first descending slope to 0
    int i=0;
    while (i<(len-1) && corr(i)>corr(i+1))
    {
        corr(i) = 0;
        i++;
    }

    // find peaks
    if (2*m_interPeakMinDist<len)
    {
        // current index is i
        const int iMax = len - m_interPeakMinDist;
        for (int candidateIndex = m_interPeakMinDist;
                candidateIndex<iMax;)
        {
            double candidateValue = corr(candidateIndex);

            // explore neighborhood
            bool skip=false;
            for (int i=1;i<=m_interPeakMinDist;i++)
            {
                if (corr(candidateIndex+i)>candidateValue)
                {
                    candidateIndex += i;
                    skip = true;
                    break;
                } else if (corr(candidateIndex-i)>=candidateValue)
                {
                    candidateIndex += i+1;
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;

            // remember peak
            {
                int p=m_nbPeaks-1;
                while (p>=0 && candidateValue>out[2*p+1])
                    --p;
                p++;
                if (p<m_nbPeaks)
                {
                    int toMove = 2*(m_nbPeaks-p-1);
                    if (toMove>0)
                        memmove(&out[2*(p+1)],&out[2*p],toMove*sizeof(double));
                    out[2*p] = candidateIndex;
                    out[2*p+1] = candidateValue;
                }
            }

            // skip to next candidate
            candidateIndex += m_interPeakMinDist + 1;
        }

    }

}

void AutoCorrelationPeaksIntegrator::flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out)
{
	in[0].data->appendZeros((m_nbFrames-1)/2);
	process(in,out);
}


}
