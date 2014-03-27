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

#ifndef LPC2LSF_H_
#define LPC2LSF_H_

#include "yaafe-core/ComponentHelpers.h"

#define LPC2LSF_ID "LPC2LSF"

namespace YAAFE
{

  class LPC2LSF: public YAAFE::StateLessOneInOneOutComponent<LPC2LSF>
  {
   public:
     LPC2LSF();
     virtual ~LPC2LSF();
     virtual const std::string getIdentifier() const { return LPC2LSF_ID;};

     virtual ParameterDescriptorList getParameterDescriptorList() const;

     virtual StreamInfo init(const ParameterMap& params, const StreamInfo& in);
     virtual void processToken(double* inData, const int inSize, double* outData, const int outSize);

   private:
     int m_displacement;
     int m_nbCoeffs;
  };

}

#endif /* LPC2LSF_H_ */
