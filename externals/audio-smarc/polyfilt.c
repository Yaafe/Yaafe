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

#include "polyfilt.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void polyfiltLM(struct PSFilter* pfilt, struct PSState* pstate,
		const double* signal, int signalLen, int* nbRead, double* output,
		int outputLen, int* nbWritten) {
	const int M = pfilt->M;
	const int L = pfilt->L;
	const int K = pfilt->K;

	int signalPos = 0;
	int outPos = 0;
	int phase = pstate->phase;

	// skip first sample for delays
	if (pstate->skip>0)
	{
		const int maxAdvance = (M + L - 1) / L;
		while (pstate->skip>0 && ((signalPos+maxAdvance)<signalLen)) {
			pstate->skip--;
			phase += M;
			signalPos += phase / L;
			phase = phase % L;
		}
	}

	// process filtering
	while ((signalPos+K<=signalLen) && (outPos<outputLen))
	{
		// compute value
		output[outPos++] = filter(pfilt->filters + phase*K,signal + signalPos, K);

		// consume samples
		phase += M;
		signalPos += phase / L;
		phase = phase % L;
	}

	// report state values
	pstate->phase = phase;
	*nbRead = signalPos;
	*nbWritten = outPos;
}


void polyfiltM(struct PSFilter* pfilt, struct PSState* pstate,
		const double* restrict signal, const int signalLen, int* restrict nbConsume,
		double* restrict output, const int outputLen, int* restrict nbWritten) {
	const int M = pfilt->M;
	const int K = pfilt->K;
	const double* filt = pfilt->filters;

	int signalPos = 0;
	int outPos = 0;

	// skip first sample for delays
	while (pstate->skip>0 && ((signalPos+M)<signalLen)) {
		pstate->skip--;
		signalPos += M;
	}

	// process filtering
	while (((signalPos+K)<=signalLen) && (outPos<outputLen))
	{
		// compute value
//		double v = 0.0;
//		const double* inPtr = signal + signalPos;
//		for (int k=0;k<K;k++)
//			v += inPtr[k] * filt[k];
//		output[outPos++] = v;
		output[outPos++] = filter(filt,signal+signalPos,K);

		// consume samples
		signalPos += M;
	}

	// report state values
	*nbWritten = outPos;
	*nbConsume = signalPos;
}

void polyfiltL(struct PSFilter* pfilt, struct PSState* pstate,
		const double* signal, int signalLen, int* nbRead, double* output,
		int outputLen, int* nbWritten) {
	const int L = pfilt->L;
	const int K = pfilt->K;

	int signalPos = 0;
	int outPos = 0;
	int phase = pstate->phase;

	// skip first sample for delays
	while (pstate->skip>0 && signalPos<signalLen) {
		pstate->skip--;
		phase++;
		if (phase==L) {
			signalPos++;
			phase = 0;
		}
	}

	// compute first output to reach phase 0
	while (signalPos+K<=signalLen && outPos<outputLen)
	{
//		double v=0;
//		const double* inPtr = signal + signalPos;
//		const double* filtPtr = pfilt->filters + phase*K;
//		for (int k=0;k<K;++k)
//			v += inPtr[k] * filtPtr[k];
//		output[outPos++] = v;
		output[outPos++] = filter(pfilt->filters + phase*K, signal+signalPos, K);
		phase++;
		if (phase==L) {
			signalPos++;
			phase=0;
		}
	}

	// report state values
	pstate->phase = phase;
	*nbRead = signalPos;
	*nbWritten = outPos;
}
