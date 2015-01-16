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

#include "SpecificLoudness.h"
#include <iostream>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

  SpecificLoudness::SpecificLoudness() : m_blockSize(0), m_bkBdLimits(0)
  {
  }

  SpecificLoudness::~SpecificLoudness()
  {
    if (m_bkBdLimits)
      delete [] m_bkBdLimits;
  }

  bool SpecificLoudness::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
  {
    assert(inp.size()==1);
    const StreamInfo& in = inp[0].data;

    // assume in->info().size is fft size
    // assume in->info().frameLength is frame size
    m_blockSize = in.frameLength;
    m_fftSize = in.size;
    m_bkBdLimits = new int[NB_BARK_BANDS+1];

    double tmp[m_fftSize];
    for (int i = 0; i < m_fftSize; i++)
    {
      tmp[i] = i * in.sampleRate / (double) m_blockSize;
      tmp[i] = 13 * atan(tmp[i] / 1315.8) + 3.5 * atan(pow(
            (tmp[i] / 7518), 2));
    }

    m_bkBdLimits[0] = 0;
    double currentBandEnd = tmp[m_fftSize-1] / NB_BARK_BANDS;
    int currentBarkBand = 1;
    for (int i = 0; i < m_fftSize; i++)
    {
      while (tmp[i] > currentBandEnd)
      {
        m_bkBdLimits[currentBarkBand++] = i;
        currentBandEnd = currentBarkBand * tmp[m_fftSize-1] / NB_BARK_BANDS;
      }
    }
    assert(currentBarkBand == NB_BARK_BANDS);
    m_bkBdLimits[NB_BARK_BANDS] = m_fftSize-1; // ignore last coeff

    outStreamInfo().add(StreamInfo(in,NB_BARK_BANDS));
    return true;
  }

  bool SpecificLoudness::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    if (in->empty()) return false;
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;

    const int N = in->info().size;
    const int M = out->info().size;
    while (!in->empty())
    {
      Map<VectorXd> inData(in->readToken(),N);
      double* outData = out->writeToken();
      for (int i=0;i<NB_BARK_BANDS;i++)
        outData[i] = pow(inData.segment(m_bkBdLimits[i],m_bkBdLimits[i+1]-m_bkBdLimits[i]).sum(),0.23);
      in->consumeToken();
    }

    return true;
  }

}
