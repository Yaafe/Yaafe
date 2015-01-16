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

#include "OBSI.h"

using namespace Eigen;
using namespace std;

namespace YAAFE
{

  OBSI::OBSI() : m_inSize(0)
  {
  }

  OBSI::~OBSI()
  {
  }

  ParameterDescriptorList OBSI::getParameterDescriptorList() const
  {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "OBSIMinFreq";
    p.m_description = "Minimum frequency for OBSI filter.";
    p.m_defaultValue = "27.5";
    pList.push_back(p);

    return pList;
  }

  bool OBSI::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
  {
    assert(inp.size()==1);
    const StreamInfo& in = inp[0].data;

    m_inSize = in.size;
    double minFreq = getDoubleParam("OBSIMinFreq",params);
    double sampleRate = in.sampleRate;
    int blockSize = in.frameLength;

    int nbFilters = 1 + (int) floor(log((sampleRate/2)/minFreq)/log(2));
    ArrayXd freqs(nbFilters);
    ArrayXd fftPeak(nbFilters+2);
    ArrayXi ceilfftPeak(nbFilters+2);
    ArrayXi floorfftPeak(nbFilters+2);

    for (int i=0;i<nbFilters;i++)
      freqs(i) = minFreq * pow(2.0,i);
    fftPeak(0) = 0;
    fftPeak.segment(1,nbFilters) = freqs * blockSize / sampleRate;
    fftPeak(nbFilters+1) = blockSize / 2;
    for (int i=0;i<fftPeak.size();i++)
    {
      ceilfftPeak(i) = ceil(fftPeak(i));
      floorfftPeak(i) = floor(fftPeak(i));
    }

    float norm = 2.0;

    for (int i=1;i<nbFilters+1;i++)
    {
      int fStart = ceilfftPeak(i-1);
      int fSize = floorfftPeak(i+1) - ceilfftPeak(i-1) + 1;
      m_filterStarts.push_back(fStart);
      m_filters.push_back(ArrayXd());
      ArrayXd& filter = m_filters.back();
      filter.resize(fSize);
      for (int f=fStart;f<=floorfftPeak(i);f++)
        filter(f-fStart) = norm * (f - fftPeak(i-1)) / (fftPeak(i)-fftPeak(i-1));
      for (int f=ceilfftPeak(i);f<=floorfftPeak(i+1);f++)
        filter(f-fStart) = norm * (1 - (f - fftPeak(i))/(fftPeak(i+1)-fftPeak(i)));
    }

    outStreamInfo().add(StreamInfo(in,m_filters.size()));
    return true;
  }

  bool OBSI::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    if (in->empty()) return false;
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;

    while (!in->empty())
    {
      Map<ArrayXd> sqrSpec(in->readToken(),in->info().size);
      double ath = sqrt(sqrSpec.maxCoeff() * 1.0e-6);
      double* output = out->writeToken();
      for (size_t i=0;i<m_filters.size();i++)
      {
        //            output[i] = log(max(sum(sqrSpec(m_filterRanges[i]) * m_filters[i]),ath));
        output[i] = log(max((m_filters[i] * sqrSpec.segment(m_filterStarts[i],m_filters[i].size())).sum(),ath));
      }
      in->consumeToken();
    }
    return true;
  }

}
