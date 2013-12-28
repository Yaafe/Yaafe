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

#include "SpectralFlatnessPerBand.h"
#include "MathUtils.h"
#include <math.h>

#define OVERLAP 0.05

using namespace std;

namespace YAAFE
{

SpectralFlatnessPerBand::SpectralFlatnessPerBand()
{
}

SpectralFlatnessPerBand::~SpectralFlatnessPerBand()
{
}

bool SpectralFlatnessPerBand::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
{
	assert(inp.size()==1);
	const StreamInfo& in = inp[0].data;
	m_inSize = in.size;

	double fs = in.sampleRate;
	int blockSize = (in.size-1)*2; // assume input is spectrum

	double hiedge = floor(fs/2);
	double loedge = hiedge * pow(2,round(log2(250.0/hiedge)));

	int k = 0;
	while (true)
	{
		k++;
		double f_lo_nom = loedge * pow(2,(double)(k-1)/4.0);
		double f_hi_nom = loedge * pow(2,(double)k/4.0);
		double f_lo = f_lo_nom * (1 - OVERLAP);
		double f_hi = f_hi_nom * (1 + OVERLAP);
		int i_lo = (int) round(f_lo / (fs/blockSize));
		int i_hi = (int) round(f_hi / (fs/blockSize));

		if (f_lo_nom >= hiedge) break;
		if (f_hi > fs/2) break;

		int grpsize = 1;
		if (f_lo_nom >= 1000)
		{
			grpsize = (int) round(pow(2, floor(log2(f_lo_nom/500.0))));
			i_hi = (int) round((double)(i_hi-i_lo+1)/grpsize)*grpsize + i_lo-1;
		}

    	bandinfo bi;
    	bi.start = i_lo;
    	bi.end = i_hi+1;
    	bi.group = grpsize;
        m_band.push_back(bi);
	}

	outStreamInfo().add(StreamInfo(in,m_band.size()));
    return true;
}

bool SpectralFlatnessPerBand::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
{
	assert(inp.size()==1);
	InputBuffer* in = inp[0].data;
	assert(outp.size()==1);
	OutputBuffer* out = outp[0].data;

	int nbBands = m_band.size();
    double* tmp = new double[in->info().size];

    for (;!in->empty();in->consumeToken())
    {
    	double* inData = in->readToken();
        double* output = out->writeToken();
        for (int k=0;k<nbBands;++k)
        {
    		bandinfo& bi = m_band[k];
    		double* data = &inData[bi.start];
    		int datalen = bi.length();
    		if (bi.group>1) // grpsize > 1
    		{
    			data = tmp;
    			datalen /= bi.group;
    			double* ptr = &inData[bi.start];
    			for (int d=0;d<datalen;d++) {
    				double s = 0;
    				for (int g=0;g<bi.group;g++)
    					s += *ptr++;
    				data[d] = s;
    			}
    		}
    		double am = 0;
    		double geomean = 0;
    		for (int i=0;i<datalen;i++)
    		{
    			const double t = data[i] + EPS;
    			am += t;
    			geomean += log(t);
    		}
    		geomean = exp(geomean/datalen);
    		if (am!=0)
    		{
    			output[k] = geomean * datalen / am;
    			continue;
    		}
    		output[k] = geomean / EPS;
        }
    }

    delete [] tmp;

    return true;
}

}
