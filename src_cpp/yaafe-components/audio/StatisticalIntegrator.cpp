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

#include "StatisticalIntegrator.h"
#include "MathUtils.h"

#include <iostream>

using namespace std;

namespace YAAFE
{

  StatisticalIntegrator::StatisticalIntegrator()
  {
  }

  StatisticalIntegrator::~StatisticalIntegrator()
  {
  }

  ParameterDescriptorList StatisticalIntegrator::getParameterDescriptorList() const
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

    p.m_identifier = "SICompute";
    p.m_description = "if 'MeanStddev' then compute mean and standard deviation, if 'Mean' compute only mean, if 'Stddev' compute only stantard deviation.";
    p.m_defaultValue = "MeanStddev";
    pList.push_back(p);

    return pList;
  }

  bool StatisticalIntegrator::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
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

    string str = getStringParam("SICompute",params);
    m_mean = true;
    m_stddev = true;
    if (str=="Mean")
    {
      m_stddev = false;
    } else if (str=="Stddev") {
      m_mean = false;
    }
    int sizefactor = (m_stddev?1:0) + (m_mean?1:0);

    StreamInfo out;
    out.size = in.size * sizefactor;
    out.sampleRate = in.sampleRate;
    out.frameLength = in.frameLength + (m_nbFrames - 1) * in.sampleStep;
    out.sampleStep = (m_stepNbFrames * in.sampleStep);
    outStreamInfo().add(out);

    return true;
  }

  bool StatisticalIntegrator::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;

    if ((out->tokenno()==0) && (in->tokenno()!=-m_nbFrames/2))
      in->prependZeros(m_nbFrames/2);
    if (!in->hasTokens(m_nbFrames)) return false;

    const int N = in->info().size;
    assert(out->info().size==2*in->info().size);
    while (in->hasTokens(m_nbFrames))
    {
      double* outPtr = out->writeToken();
      for (int j = 0; j < N; j++)
      {
        int nb = 0;
        double m = 0;
        for (int i=0;i<m_nbFrames;i++)
        {
          double v = *(in->token(i) + j);
          if (!isnan(v)) {
            ++nb;
            m += v;
          }
        }
        m /= nb;
        if (m_mean)
          *outPtr++ = m;
        if (m_stddev) {
          double stddev = 0;
          for (int i=0;i<m_nbFrames;i++) {
            double v = *(in->token(i)+j);
            if (!isnan(v))
              stddev += pow2(v - m);
          }
          *outPtr++ = sqrt(stddev/nb);
        }
      }
      in->consumeTokens(m_stepNbFrames);
    }
    return true;
  }

  void StatisticalIntegrator::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    InputBuffer* in = inp[0].data;
    in->appendZeros((m_nbFrames-1)/2);
    process(inp,outp);
  }

}
