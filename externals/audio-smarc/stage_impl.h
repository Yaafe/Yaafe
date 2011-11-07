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

#ifndef POLYPHASE_DECL_H_
#define POLYPHASE_DECL_H_

#include "filtering.h"

#define FILT(pfilt,m,l) &pfilt->filters[(m*L+l)*K]
#define FILT_STATE(pstate,m) &pstate->yi[m*(K-1)]

/**
 * Defines a filter stage.
 * - flen: length of remez filter. (L*M*K)
 * - L: interpolation factor
 * - M: decimation factor
 * - K: sub-filter length
 * - filters: array of L*M sub filters of length K. (total size is flen)
 */
struct PSFilter {
	int flen;
	int L;
	int M;
	int K;

	double* filters;
	int filter_delay;
};

/**
 * Initialize PSFilter struct with given parameters
 * - L [IN]: interpolation factor
 * - M [IN]: decimation factor
 * - fpass [IN]: start of lowpass filter transition band, in normalized form (0<fpass<0.5)
 * - fstop [IN]: end of lowpass filter transition band, in normalized form (fpass<fstop<0.5)
 * - rp [IN]: accepted ripple factor in pass band (in dB) within global structure.
 * - rs [IN]: accepted ripple factor in stop band (in dB)
 * - rpFactor [IN]: number of stage in global structure.
 *
 * Return pointer to PSFilter struct. This pointer must be deleted using destroy_psfilter function.
 */
struct PSFilter* init_psfilter(int L, int M,
		double fpass, double fstop,
		double rp, double rs, int rpFactor);

/**
 * Destroy PSFilter, release memory
 */
void destroy_psfilter(struct PSFilter*);

/**
 * Defines a filter stage state.
 * - inbuf: input buffer
 * - inbuf_size: input buffer allocated size
 * - available: available samples in inbuf
 * - skip: number of samples to skip (used to handle filter delay)
 */
struct PSState {
	int skip;
	int phase;
};

/**
 * Initialize a filter stage state for the given filter stage
 * The returned pointer must be freed by calling destroy_psstate
 */
struct PSState* init_psstate(struct PSFilter*);

/**
 * Destroy a filter stage state
 */
void destroy_psstate(struct PSState*);

/**
 * Reset a filter stage state so that an other signal can be filtered.
 */
void reset_psstate(struct PSState*, const struct PSFilter*);

#endif /* POLYPHASE_DECL_H_ */
