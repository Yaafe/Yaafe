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

#include "stage_impl.h"
#include "filtering.h"
#include "polyfilt.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "remez_lp.h"

#define MAX_FILTER_LENGTH 8192

void build_filter(double fpass, double fstop,
		double rp, double rs, int rpFactor, double** h, int* len, int lenStep) {
	int i;
	double *bands, *mag, *dev, *weight;

//	printf("build filter %f %f %f, %f/%i %f (lenstep=%i)\n",fpass,fstop,fmax,rp,rpFactor, rs,lenStep);

	bands = malloc(10 * sizeof(double));
	mag = &bands[4];
	dev = &mag[2];
	weight = &dev[2];

	bands[0] = 0;
	bands[1] = fpass;
	bands[2] = fstop;
	bands[3] = 0.5;

	mag[0] = 1;
	mag[1] = 0;

	dev[0] = (pow(10, rp / 20.0) - 1) / (rpFactor *(pow(10, rp / 20.0) + 1));
	dev[1] = pow(10, -rs / 20.0);

	int n = remez_lp_order(bands, mag, dev, weight);

	// filter length must be 2*K*lenStep+1 so that delay is integer
	{
		int k = 1;
		while (2*k*lenStep + 1 < n)
			k++;
		*len = 2*k*lenStep + 1;
	}

	if (n>MAX_FILTER_LENGTH)
	{
		free(bands);
		*len = 0;
		printf("ERROR: cannot build remez filter, it's too long ! (%i) try with other parameters\n",n);
		return;
	}

	*h = malloc((*len) * sizeof(double));
	for (i = 0; i < *len; i++)
		(*h)[i] = 0;
	int iRc = remez_lp(*h, *len, bands, mag, weight);
	if (iRc)
	{
		free(*h);
		*h = NULL;
		*len = 0;
	}

	free(bands);
}

struct PSFilter* init_psfilter(int L, int M,
		double fpass, double fstop,
		double rp, double rs, int rpFactor) {

	double* h = 0;
	int Lenh;

	build_filter(fpass,fstop,rp, rs, rpFactor, &h, &Lenh,M);
	if (Lenh==0)
	{
		printf("ERROR: cannot build filter %i/%i (within a %i stage filter) with parameters fpass=%0.2f fstop=%0.2f rp=%0.2f rs=%0.2f\n",L,M,rpFactor, fpass,fstop,rp,rs);
		return NULL;
	}
	// ensure Lenh > M+L-1
	// (this is generally already true, but must check it for polyfiltLM)
	while (Lenh<(L+M-1))
		Lenh += 2 * M;

	struct PSFilter* pfilt = malloc(sizeof(struct PSFilter));

	// adjust filter gain
	if (L>1)
		for (int i = 0; i < Lenh; i++)
			h[i] *= L;

	// filters
	int K = Lenh / L;
	if (Lenh > K*L)
		K++;
	pfilt->filters = malloc(L*K*sizeof(double));

	if (L==1)
	{
		// decimation filter (only one filter)
		for (int k=0;k<K;k++)
			pfilt->filters[K-1-k] = h[k];
	} else {
		// filter with interpolation (one filter per interpolation tap)
		for (int i=0;i<L;i++)
			pfilt->filters[i*K] = 0.0;
		for (int i=0;i<Lenh;i++)
		{
			pfilt->filters[(i%L)*K+K-1-i/L] = h[i];
		}
	}

	free(h);

	pfilt->flen = Lenh;
	pfilt->M = M;
	pfilt->L = L;
	pfilt->K = K;
	pfilt->filter_delay = (Lenh - 1) / (2*M);

	return pfilt;
}

void destroy_psfilter(struct PSFilter* pfilt) {
	free(pfilt->filters);
	free(pfilt);
}

struct PSState* init_psstate(struct PSFilter* pfilt) {
	struct PSState* pstate = (struct PSState*) malloc(sizeof(struct PSState));
	pstate->skip = 0;
	pstate->phase = 0;
	reset_psstate(pstate,pfilt);
	return pstate;
}

void destroy_psstate(struct PSState* pstate) {
	free(pstate);
}

void reset_psstate(struct PSState* pstate, const struct PSFilter* pfilt) {
	pstate->skip=pfilt->filter_delay;
	pstate->phase = 0;
}

