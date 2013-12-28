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

#include "Decrease.h"

using namespace std;

namespace YAAFE
{

  Decrease::Decrease()
  {
  }

  Decrease::~Decrease()
  {
  }

  StreamInfo Decrease::init(const ParameterMap& params, const StreamInfo& in)
  {
    return StreamInfo(in,1);
  }

  void Decrease::processToken(double* inData, const int N, double* out, const int outSize)
  {
    double firstCoeff = inData[0];
    double sumDec = 0;
    double sum = 0;
    for (int i=1;i<N;i++) {
      sum += inData[i];
      sumDec += (inData[i]-firstCoeff) / i;
    }
    *out = sumDec / sum;
  }

}
