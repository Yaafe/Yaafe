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

#ifndef MEDIANFILTER_H_
#define MEDIANFILTER_H_

#include "yaafe-core/Component.h"

#define MEDIANFILTER_ID "MedianFilter"

namespace YAAFE {

  class MedianFilter: public YAAFE::ComponentBase<MedianFilter> {
   public:
     MedianFilter();
     virtual ~MedianFilter();

     virtual const std::string getIdentifier() const { return MEDIANFILTER_ID;};
     virtual bool stateLess() const { return false; }

     virtual ParameterDescriptorList getParameterDescriptorList() const;

     virtual bool init(const ParameterMap& params, const Ports<StreamInfo>& in);
     virtual void reset();
     virtual bool process(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);
     virtual void flush(Ports<InputBuffer*>& in, Ports<OutputBuffer*>& out);

   private:
     int m_order;
     int m_delay;
     int m_medianIndex;
     int m_pos;
     int m_dataSize;
     double* m_data;


  };

}

#endif /* MEDIANFILTER_H_ */
