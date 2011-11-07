/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Author : Jacques Prado, Benoit Mathieu
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

#ifndef FFT_H_
#define FFT_H_

#include "yaafe-core/ComponentHelpers.h"
#include <Eigen/Dense>
#ifdef WITH_FFTW3
#include <fftw3.h>
#else
#include <unsupported/Eigen/FFT>
#endif

#define FFT_ID "FFT"

namespace YAAFE
{

class FFT: public YAAFE::StateLessOneInOneOutComponent<FFT>
{
public:
    FFT();
    virtual ~FFT();

    const std::string getIdentifier() const { return FFT_ID; };

    virtual ParameterDescriptorList getParameterDescriptorList() const;

	StreamInfo init(const ParameterMap& params, const StreamInfo& in);
	void processToken(double* inData, const int inSize, double* out, const int outSize);

private:
    Eigen::VectorXd m_window;
    int m_nfft;
#ifdef WITH_FFTW3
    fftw_plan m_plan;
#else
    Eigen::FFT<double> m_plan;
#endif
};

}

#endif /* FFT_H_ */
