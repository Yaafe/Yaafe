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

#include "Flatness.h"
#include "math.h"

namespace YAAFE
{

  Flatness::Flatness()
  {
  }

  Flatness::~Flatness()
  {
  }

  StreamInfo Flatness::init(const ParameterMap& params, const StreamInfo& in)
  {
    return StreamInfo(in,1);
  }

  void Flatness::processToken(double* inData, const int N, double* outData, const int outSize)
  {
    assert(outSize==1);
    double sum = 0;
    double sumlog = 0;
    for (int i=0;i<N;i++) {
      sum += inData[i];
      sumlog += log(inData[i]);
    }
    *outData = exp(sumlog / N) * N / sum;
  }

}
