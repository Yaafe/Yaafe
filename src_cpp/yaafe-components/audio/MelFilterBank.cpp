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

#include "MelFilterBank.h"
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

MelFilterBank::MelFilterBank() : m_size(0)
{
}

MelFilterBank::~MelFilterBank()
{
}

ParameterDescriptorList MelFilterBank::getParameterDescriptorList() const
{
    ParameterDescriptorList params;
    ParameterDescriptor p;

    p.m_identifier = "MelNbFilters";
    p.m_description = "Number of mel filters";
    p.m_defaultValue = "40";
    params.push_back(p);

    p.m_identifier = "MelMinFreq";
    p.m_description = "Minimum frequency of the mel filter bank";
    p.m_defaultValue = "130.0";
    params.push_back(p);

    p.m_identifier = "MelMaxFreq";
    p.m_description = "Maximum frequency of the mel filter bank";
    p.m_defaultValue = "6854.0";
    params.push_back(p);

    return params;
}

bool MelFilterBank::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;

	// build mel filter bank
	m_size = in.size;
	int nbMelFilters = getIntParam("MelNbFilters",params);
	double sampleRate = in.sampleRate;
	double freqMin = getDoubleParam("MelMinFreq",params);
	double freqMax = getDoubleParam("MelMaxFreq",params);
	double melFreqMin = 1127 * log(1 + freqMin / 700);
	double melFreqMax = 1127 * log(1 + freqMax / 700);

	VectorXd melPeak(nbMelFilters+2);
	VectorXd freqs(nbMelFilters+2);

	melPeak = VectorXd::LinSpaced(nbMelFilters+2,melFreqMin,melFreqMax);
	freqs = ((melPeak / 1127).array().exp() - 1.0) * 700.0;
	VectorXd fftFreqs(m_size);
	fftFreqs = VectorXd::LinSpaced(m_size,0,m_size-1) * sampleRate / ((m_size-1)*2);
	for (int b=1;b<nbMelFilters+1;b++)
	{
		double norm = 2.0 / (freqs(b+1)-freqs(b-1));
		VectorXd fullfilt(m_size);
//		fullfilt.setZero(m_size);
//		firstIndex i;
//		fullfilt += where((fftFreqs(i)>freqs(b-1)) && (fftFreqs(i)<=freqs(b)),norm*(fftFreqs(i)-freqs(b-1))/(freqs(b)-freqs(b-1)),0.0);
//		fullfilt += where((fftFreqs(i)>freqs(b)) && (fftFreqs(i)<freqs(b+1)),norm*(freqs(b+1)-fftFreqs(i))/(freqs(b+1)-freqs(b)),0.0);
		double ffmin = freqs(b-1);
		double ffmiddle = freqs(b);
		double ffmax = freqs(b+1);
		for (int i=0;i<m_size;i++) {
			if ((fftFreqs(i)<ffmin) || (fftFreqs(i)>ffmax)) {
				fullfilt(i) = 0;
				continue;
			}
			if (fftFreqs(i)<ffmiddle)
				fullfilt(i) = norm*(fftFreqs(i)-ffmin)/(ffmiddle-ffmin);
			else
				fullfilt(i) = norm*(ffmax-fftFreqs(i))/(ffmax-ffmiddle);
		}

		int fStart=0;
		while (fullfilt(fStart)==0.0) fStart++;
		int fEnd=fStart+1;
		while (fullfilt(fEnd)!=0.0) fEnd++;
		m_filterStart.push_back(fStart);
		m_filters.push_back(RowVectorXd());
		m_filters.back() = fullfilt.segment(fStart,fEnd-fStart);
	}

	outStreamInfo().add(StreamInfo(in, m_filters.size()));
    return true;
}

bool MelFilterBank::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	if (in->empty()) return false;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	while (!in->empty()) {
		Map<VectorXd> inData(in->readToken(),in->info().size);
		double* outData = out->writeToken();
		for (int f=0;f<m_filters.size();f++)
		{
			RowVectorXd& filter = m_filters[f];
			outData[f] = filter * inData.segment(m_filterStart[f],filter.size());
		}
		in->consumeToken();
	}

    return true;
}

}
