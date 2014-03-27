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

#include "FilterSmallValues.h"
#include "MathUtils.h"

using namespace std;

namespace YAAFE {

  FilterSmallValues::FilterSmallValues() :
    m_threshold(0) {

    }

  FilterSmallValues::~FilterSmallValues() {
  }

  ParameterDescriptorList FilterSmallValues::getParameterDescriptorList() const {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "FSVThreshold";
    p.m_description = "Values less than FSVThreshold will be set to eps.";
    p.m_defaultValue = "0.001";
    pList.push_back(p);

    return pList;
  }

  bool FilterSmallValues::init(const ParameterMap& params, const Ports<StreamInfo>& inp) {
    assert(inp.size()==1);
    const StreamInfo& in = inp[0].data;

    m_threshold = getDoubleParam("FSVThreshold",params);
    outStreamInfo().add(StreamInfo(in));
    return true;
  }

  bool FilterSmallValues::process(Ports<InputBuffer*>& inp, Ports<OutputBuffer*>& outp) {
    assert(inp.size()==1);
    InputBuffer* in = inp[0].data;
    if (in->empty()) return false;
    assert(outp.size()==1);
    OutputBuffer* out = outp[0].data;

    assert(in->info().size==out->info().size);
    while (!in->empty()) {
      const double* inData = in->readToken();
      double* outData = out->writeToken();
      for (int i=0;i<in->info().size;i++)
        outData[i] = (inData[i]>m_threshold) ? inData[i] : EPS;
      in->consumeToken();
    }
    return true;
  }

}
