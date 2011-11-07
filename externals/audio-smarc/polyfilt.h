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

#ifndef POLYFILTLM_H_
#define POLYFILTLM_H_

#include "smarc.h"
#include "stage_impl.h"

/**
 * Filter signal with a L/M filter (interpolation factor and decimation factor are not 1)
 * - pfilt [IN]: filter to use
 * - pstate [IN/OUT]: filter state
 * - signal [IN]: input signal
 * - signalLen [IN]: input signal length
 * - nbRead [OUT]: number of samples read from input signal
 * - output [OUT]: output array
 * - outputLen [IN]: length of output array. Maximum number of samples to write
 * - nbWritten [OUT]: number of samples effectively written
 * - w [IN]: workspace to use for temporary results.
 */
void polyfiltLM(struct PSFilter* pfilt, struct PSState* pstate,
		const double* restrict signal, const int signalLen, int* nbConsume,
		double* restrict output, const int outputLen, int* nbWritten);

/**
 * Filter signal with a decimation filter (interpolation factor L is 1)
 * - pfilt [IN]: filter to use
 * - pstate [IN/OUT]: filter state
 * - signal [IN]: input signal
 * - signalLen [IN]: input signal length
 * - nbRead [OUT]: number of samples read from input signal
 * - output [OUT]: output array
 * - outputLen [IN]: length of output array. Maximum number of samples to write
 * - nbWritten [OUT]: number of samples effectively written
 * - w [IN]: workspace to use for temporary results.
 */
void polyfiltM(struct PSFilter* pfilt, struct PSState* pstate,
		const double* restrict signal, const int signalLen, int* restrict nbConsume,
		double* restrict output, const int outputLen, int* restrict nbWritten);

/**
 * Filter signal with a interpolation filter (decimation factor M is 1)
 * - pfilt [IN]: filter to use
 * - pstate [IN/OUT]: filter state
 * - signal [IN]: input signal
 * - signalLen [IN]: input signal length
 * - nbRead [OUT]: number of samples read from input signal
 * - output [OUT]: output array
 * - outputLen [IN]: length of output array. Maximum number of samples to write
 * - nbWritten [OUT]: number of samples effectively written
 * - w [IN]: workspace to use for temporary results.
 */
void polyfiltL(struct PSFilter* pfilt, struct PSState* pstate,
		const double* restrict signal, const int signalLen, int* restrict nbConsume,
		double* restrict output, const int outputLen, int* restrict nbWritten);

#endif /* POLYFILTLM_H_ */
