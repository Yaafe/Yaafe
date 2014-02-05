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

#include "FrameTokenizer.h"

#include <iostream>
#include <string.h>

using namespace std;

namespace YAAFE
{

  FrameTokenizer::FrameTokenizer() :
    m_blockSize(0), m_stepSize(0)
  {
  }

  FrameTokenizer::~FrameTokenizer()
  {
  }

  ParameterDescriptorList FrameTokenizer::getParameterDescriptorList() const
  {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "blockSize";
    p.m_description = "output frames size";
    p.m_defaultValue = "1024";
    pList.push_back(p);

    p.m_identifier = "stepSize";
    p.m_description = "step between consecutive frames";
    p.m_defaultValue = "512";
    pList.push_back(p);

    return pList;
  }

  bool FrameTokenizer::init(const ParameterMap& params, const Ports<StreamInfo>& inp)
  {
    assert(inp.size()==1);
    const StreamInfo& in = inp[0].data;

    if (in.size > 1)
    {
      cerr << "ERROR: input of FrameTokenizer should be of size 1" << endl;
      return false;
    }
    m_blockSize = getIntParam("blockSize", params);
    if (m_blockSize<=0) {
      cerr << "ERROR: invalid blockSize parameter !" << endl;
    }
    m_stepSize = getIntParam("stepSize", params);
    if (m_stepSize<=0) {
      cerr << "ERROR: invalid stepSize parameter !" << endl;
      return false;
    }

    outStreamInfo().add(StreamInfo());
    StreamInfo& outInfo = outStreamInfo()[0].data;
    outInfo.sampleRate = in.sampleRate;
    outInfo.frameLength = m_blockSize * in.frameLength;
    outInfo.sampleStep = m_stepSize * in.sampleStep;
    outInfo.size = m_blockSize;

    return true;
  }

  bool FrameTokenizer::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;
    assert(in->size()==1);

    if ((out->tokenno()==0) && (in->tokenno()!=-m_blockSize/2))
      in->prependZeros(m_blockSize/2);

    if (!in->hasTokens(m_blockSize))
      return false;

    while (in->hasTokens(m_blockSize)) {
      in->read(out->writeToken(),m_blockSize);
      in->consumeTokens(m_stepSize);
    }

    return true;
  }

  void FrameTokenizer::flush(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp)
  {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    in->appendZeros((m_blockSize-1)/2);
    process(inp,outp);
  }

}
