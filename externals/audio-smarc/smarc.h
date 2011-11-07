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

#ifndef SMARC_H_
#define SMARC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * PFilter represent a multi-stage polyphase filter
 */
struct PFilter;

/**
 * Build a PFilter with given parameter. Given pointer must be free by calling destroy_pfilter function
 * - fsin (IN) : frequency samplerate of input signal
 * - fsout (IN) : desired frequency samplerate for output signal
 * - bandwidth (IN) : bandwidth to keep, real in 0..1. For example, 0.95 means that 95% of maximum
 *                       possible bandwidth is kept. Too high value may results in impossible filter to design.
 * - rp (IN) : Passband ripple factor, in dB.
 * - rs (IN) : Stopband ripple factor, in dB.
 * - tol (IN) : samplerate conversion error tolerance. Usual value for tol is 0.000001. Usual frequencies conversion
 *              has exact samplerate conversion, but for unusual frequencies, in particular when fsin or fsout
 *              are large prime numbers, the user should accept a tolerance otherwise the filter may
 *              be impossible to design.
 * - userratios (IN) : ratios to use to build multistage samplerate converter, in following format :
 *                     'L1/M2 L2/M2 ...'. This parameter is optional.
 * - searchfastconversion (IN) : if 1 try to search fastest conversion stages. This may fail.
 *                               if 0 use safe default conversion stages.
 */
struct PFilter* smarc_init_pfilter(int fsin, const int fsout,
		double bandwidth, double rp, double rs,
		double tol, const char* userratios, int searchfastconversion);

/**
 * release PFilter
 */
void smarc_destroy_pfilter(struct PFilter*);

/**
 * returns input frequency samplerate for PFilter
 */
int smarc_get_fs_in(struct PFilter*);

/**
 * return output frequency samplerate for PFilter
 */
int smarc_get_fs_out(struct PFilter*);

/**
 * return a recommended output buffer size to safely resample
 * input signal chunks of length inSize with filter pfilt
 */
int smarc_get_output_buffer_size(struct PFilter* pfilt,int inSize);

/**
 * print PFilter informations to standard output
 */
void smarc_print_pfilter(struct PFilter*);

/**
 * PState represent a filter state. A PFilter may be used to filter several channels, each channels having its own PState.
 */
struct PState;

/**
 * Create a PState for a given PFilter. Returned pointer must be freed by destroy_pstate()
 */
struct PState* smarc_init_pstate(struct PFilter*);

/**
 * Free PState
 */
void smarc_destroy_pstate(struct PState*);

/**
 * Reset PState so that it can be used to process another signal.
 */
void smarc_reset_pstate(struct PState*, struct PFilter*);

/**
 * Resample a chunk of signal.
 *  - pfilter [IN]: PFilter used to resample
 *  - pstate [IN/OUT]: current state of resampler
 *  - signal [IN]: array holding signal to resample
 *  - signalLength [IN]: length of signal to resample
 *  - output [OUT]: buffer where to write resampled signal
 *  - outputLength [IN]: size of output buffer.
 * Returns the number of output samples written.
 */
int smarc_resample(struct PFilter* pfilter, struct PState* pstate,
		const double* signal,
		int signalLength,
		double* output,
		int outputLength);

/**
 * Flush all sample in state considering there will be no more input signal.
 * This function must be called only once.
 *  - pfilter [IN]: PFilter used to resample
 *  - pstate [IN/OUT]: current state of resampler
 *  - output [OUT]: buffer where to write resampled signal
 *  - outputLength [IN]: size of output buffer.
 * Returns the number of output samples written.
 */
int smarc_resample_flush(struct PFilter*, struct PState*,
		double* output,
		int outputLength);

#ifdef __cplusplus
}
#endif


#endif /* SMARC_H_ */
