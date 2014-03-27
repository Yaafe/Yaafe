/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Authors : Jacques Prado, Benoit Mathieu
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

#include "FFT.h"
#include "MathUtils.h"

#include <iostream>
#include <string.h>

using namespace std;
using namespace Eigen;

namespace YAAFE
{

  FFT::FFT() :
    m_window(), m_plan(), m_nfft(0)
  {
#ifdef WITH_FFTW3
    m_plan = NULL;
#else
    m_plan.SetFlag(Eigen::FFT<double>::HalfSpectrum);
#endif
  }

  FFT::~FFT()
  {
#ifdef WITH_FFTW3
    if (m_plan)
      fftw_destroy_plan(m_plan);
#endif
  }

  ParameterDescriptorList FFT::getParameterDescriptorList() const
  {
    ParameterDescriptorList pList;
    ParameterDescriptor p;

    p.m_identifier = "FFTLength";
    p.m_description
      = "Frame's length on which perform FFT. Original frame is padded with zeros or truncated to reach this size. If 0 then use original frame length.";
    p.m_defaultValue = "0";
    pList.push_back(p);

    p.m_identifier = "FFTWindow";
    p.m_description
      = "Weighting window to apply before fft. Hanning|Hamming|None";
    p.m_defaultValue = "Hanning";
    pList.push_back(p);

    return pList;
  }

  StreamInfo FFT::init(const ParameterMap& params, const StreamInfo& in)
  {
    int len = getIntParam("FFTLength", params);
    if (len == 0)
      len = in.size;
    string w = getStringParam("FFTWindow", params);
    if (w != "None")
    {
      if (w == "Hanning")
        m_window = ehanningPeriodic(in.size);
      else if (w == "Hamming")
        m_window = ehammingPeriodic(in.size);
      else
      {
        cerr << "FFT: invalid Window parameter value " << w << " ignore it !" << endl;
      }
    }
    // init plan
    m_nfft = len;
#ifdef WITH_FFTW3
    double* inFFT = (double*) fftw_malloc(m_nfft*sizeof(double));
    complex<double>* outFFT = (complex<double>*) fftw_malloc((m_nfft/2+1)*sizeof(complex<double>));
    m_plan = fftw_plan_dft_r2c_1d(m_nfft,inFFT,(fftw_complex*)outFFT,FFTW_MEASURE);
    fftw_free(inFFT);
    fftw_free(outFFT);
#else
    VectorXd infft(m_nfft);
    VectorXcd outfft(m_nfft/2+1);
    m_plan.fwd(outfft.data(),infft.data(),m_nfft);
#endif

    return StreamInfo(in,len+2);
  }

  void FFT::processToken(double* inPtr, const int N, double* out, const int outSize)
  {
#ifdef WITH_FFTW3
    double* inFFT = (double*) fftw_malloc(m_nfft*sizeof(double));
    complex<double>* outFFT = (complex<double>*) fftw_malloc((m_nfft/2+1)*sizeof(complex<double>));
    Map<VectorXd> infft(inFFT,m_nfft);
#else
    VectorXd infft(m_nfft);
#endif
    Map<VectorXd> inData(inPtr,N);
    if (m_window.size()>0)
      infft.segment(0,N) = m_window.array() * inData.array();
    else
      infft.segment(0,N) = inData;
    if (N<m_nfft)
      infft.segment(N,m_nfft-N).setZero();
#ifdef WITH_FFTW3
    fftw_execute_dft_r2c(m_plan,inFFT,(fftw_complex*)outFFT);
    memcpy(out,outFFT,outSize*sizeof(double));
    fftw_free(inFFT);
    fftw_free(outFFT);
#else
    m_plan.fwd((std::complex<double>*) out,infft.data(),m_nfft);
#endif
  }

}
