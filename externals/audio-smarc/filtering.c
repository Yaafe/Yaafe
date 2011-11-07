/**
 * Smarc
 *
 * Copyright (c) 2009-2011 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Authors : Benoit Mathieu, Jacques Prado
 *
 * This file is part of Smarc.
 *
 * Smarc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Smarc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "filtering.h"

#ifndef __SSE2__

double filter(const double* restrict filt, const double* restrict signal, const int K)
{
	register double v = 0.0;
	for (int k=0;k<K;++k)
		v+=filt[k]*signal[k];
	return v;
}

#else

#include <emmintrin.h>

double basic_filter(const double* restrict filt, const double* restrict signal, const int K)
{
//	printf("basic filter for K=%i\n",K);
	register double v = 0.0;
	for (int k=0;k<K;++k)
		v+=filt[k]*signal[k];
	return v;
}

double sse_filtering_aligned(const double* restrict filt, const double* restrict signal, const int K) {
//	printf("sse_filtering filt=%p signal=%p K=%i\n",filt,signal,K);
	// filt and signal are 16 byte aligned, K % 2 = 0
	__m128d v = _mm_setzero_pd();
	int k=0;
	for (;k<K-1;k+=2) {
		__m128d f = _mm_load_pd(filt + k);
		__m128d s = _mm_load_pd(signal + k);
		v = _mm_add_pd(v,_mm_mul_pd(f,s));
	}
	double tmp[2];
	_mm_storeu_pd(tmp,v);
	if (k==K)
		return tmp[0] + tmp[1];
	// else
	return tmp[0] + tmp[1] + filt[k]*signal[k];
}

double sse_filtering_misaligned(const double* restrict filt, const double* restrict signal, const int K) {
//	printf("sse_filtering_misaligned filt=%p signal=%p K=%i\n",filt,signal,K);
	__m128d v = _mm_setzero_pd();
	__m128d s = _mm_load1_pd(signal);
	int k=0;
	for (;k<K-4;k+=4) {
		__m128d f = _mm_load_pd(filt + k);
		__m128d s2 = _mm_load_pd(signal + k + 1);
		s = _mm_shuffle_pd(s,s2,_MM_SHUFFLE2(0,1));
		v = _mm_add_pd(v,_mm_mul_pd(f,s));
		f = _mm_load_pd(filt + k + 2);
		s = _mm_load_pd(signal + k + 3);
		s2 = _mm_shuffle_pd(s2,s,_MM_SHUFFLE2(0,1));
		v = _mm_add_pd(v,_mm_mul_pd(f,s2));
	}
	double tmp[2];
	_mm_storeu_pd(tmp,v);
	for (;k<K;++k)
		tmp[0]+=filt[k]*signal[k];
	return tmp[0] + tmp[1];
}

double filter(const double* restrict filt, const double* restrict signal, int K)
{
	if (K<8)
		return basic_filter(filt,signal,K);
	double v = 0;
	if ((((unsigned long)filt) & 15) != 0) { // if filt is not 16-bytes aligned
		v += filt[0]*signal[0];
		++filt;
		++signal;
		--K;
	}
	if ((((unsigned long)signal) & 15) == 0) { // if signal is 16-bytes aligned
		return v + sse_filtering_aligned(filt,signal,K);
	}
	// else
	return v + sse_filtering_misaligned(filt,signal,K);
}


#endif


