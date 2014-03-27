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

#include "SlopeIntegrator.h"

using namespace Eigen;
using namespace std;

namespace YAAFE
{

  SlopeIntegrator::SlopeIntegrator() :
    m_nbFrames(0)
  {
  }

  SlopeIntegrator::~SlopeIntegrator()
  {
  }

  ParameterDescriptorList SlopeIntegrator::getParameterDescriptorList() const
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

    return pList;
  }

  bool SlopeIntegrator::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
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

    m_slopeCoeffs = VectorXd::LinSpaced(m_nbFrames,0,m_nbFrames-1);
    m_slopeCoeffs.array() -= m_slopeCoeffs.mean();
    m_slopeNorm = m_slopeCoeffs.squaredNorm();

    StreamInfo out;
    out.size = in.size;
    out.sampleRate = in.sampleRate;
    out.frameLength =  in.frameLength + (m_nbFrames - 1) * in.sampleStep;
    out.sampleStep = m_stepNbFrames * in.sampleStep;
    outStreamInfo().add(out);

    return true;
  }

  bool SlopeIntegrator::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
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
      double* outPtr = out->writeToken();
      for (int i = 0; i < in->info().size; i++)
      {
        int nbvalue = 0;
        double sumdata = 0;
        double sumslope = 0;
        double sumcoeffs = 0;
        double sumcoeffssqr = 0;
        for (int j=0;j<m_nbFrames;j++) {
          const double v = *(in->token(j)+i);
          if (!std::isnan(v)) {
            nbvalue++;
            sumdata += v;
            double coeff = j - (double)(m_nbFrames-1.0)/2.0;
            sumslope += v*coeff;
            sumcoeffs += coeff;
            sumcoeffssqr += coeff*coeff;
          }
        }
        outPtr[i] = (sumslope - sumcoeffs * sumdata) / (sumcoeffssqr - sumcoeffs*sumcoeffs/nbvalue);
      }
      in->consumeTokens(m_stepNbFrames);
    }
    return true;
  }

  void SlopeIntegrator::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    inp[0].data->appendZeros((m_nbFrames-1)/2);
    process(inp,outp);
  }

}
