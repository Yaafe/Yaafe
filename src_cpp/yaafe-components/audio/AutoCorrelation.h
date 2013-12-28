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

#ifndef AUTOCORRELATION_H_
#define AUTOCORRELATION_H_

#include "yaafe-core/ComponentHelpers.h"

#define AUTOCORRELATION_ID "AutoCorrelation"

namespace YAAFE
{

class AutoCorrelation: public YAAFE::StateLessOneInOneOutComponent<AutoCorrelation>
{
public:
    AutoCorrelation();
    virtual ~AutoCorrelation();

    virtual const std::string getIdentifier() const { return AUTOCORRELATION_ID;};

    virtual ParameterDescriptorList getParameterDescriptorList() const;
    StreamInfo init(const ParameterMap& params, const StreamInfo& in);
    void processToken(double* inData, const int inSize, double* outData, const int outSize);

};

}

#endif /* AUTOCORRELATION_H_ */
