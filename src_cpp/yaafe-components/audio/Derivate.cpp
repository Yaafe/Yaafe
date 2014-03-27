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

#include "Derivate.h"
#include <iostream>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

  Derivate::Derivate()
  {}

  Derivate::~Derivate()
  {}

  ParameterDescriptorList Derivate::getParameterDescriptorList() const
  {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "DOrder";
    p.m_description = "Order of the derivative to compute.";
    p.m_defaultValue = "1";
    pList.push_back(p);

    p.m_identifier = "DO1Len";
    p.m_description = "Horizon used to compute order 1 derivative.";
    p.m_defaultValue = "4";
    pList.push_back(p);

    p.m_identifier = "DO2Len";
    p.m_description
      = "Horizon used to compute order 2 derivative. Useless if DOrder=1.";
    p.m_defaultValue = "1";
    pList.push_back(p);

    return pList;
  }

  bool Derivate::initFilter(const ParameterMap& params, const StreamInfo& in)
  {
    int order = getIntParam("DOrder",params);
    int vlen = getIntParam("DO1Len",params);
    int alen = getIntParam("DO2Len",params);

    VectorXd filter;
    VectorXd vf;
    vf.setLinSpaced(2*vlen+1,-vlen,vlen);
    vf /= vf.squaredNorm();
    if (order == 1)
    {
      filter = vf;
    }
    else if (order == 2)
    {
      VectorXd af;
      af.setLinSpaced(2 * alen + 1, alen, -alen);
      af /= af.squaredNorm();
      filter.setZero(vf.size() + 2 * alen);
      RowVectorXd tmp;
      tmp.setZero(vf.size()+2 * (af.size()-1));
      tmp.segment(af.size()-1,vf.size()) = vf;
      for (int i=0;i<filter.size();i++)
        filter(i) = tmp.segment(i,af.size()) * af;
    }
    else
    {
      cerr << "Derivate: invalid DOrder value " << order << endl;
      return false;
    }

    m_delay = (filter.size() - 1) / 2;
    m_length = filter.size();
    m_filter = new double[m_length];
    memcpy(m_filter,filter.data(),m_length*sizeof(double));
    return true;
  }

}
