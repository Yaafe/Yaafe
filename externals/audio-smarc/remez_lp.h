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

#ifndef __REMEZ_LP_H__
#define __REMEZ_LP_H__

#define PI             3.1415926535897932
#define PI2            6.2831853071795865

/**
 * Compute minimal length of lowpass filter for given parameters (Remez's method)
 * - fcuts [IN]: frequency bands (normalized form). Should be [0 fpass fcut 0.5]
 * - mag [IN]: amplitudes in given bands. should be [1 0] for a lowpass filter
 * - dev [IN]: accepted ripple factor in given bands.
 * - weight [OUT]: weights to use in remez_lp
 * return filter length.
 */
int remez_lp_order(const double* fcuts, const double* mag, const double* dev,
		double* weight);

/**
 * Applying Remez method to build a lowpass filter matching the given parameters.
 * - h [OUT]: array where to write result filter (already allocated)
 * - filterLen [IN]: filter length (computed by remez_lp_order)
 * - bands [IN]: frequency bands (normalized form). Should be [0 fpass fcut 0.5]
 * - dev [IN]: accepted ripple factor in given bands
 * - weight [IN]: weight computed by remez_lp_order
 * return 0 if success -1 if failed
 */
int remez_lp(double h[], int filterLen,
           double bands[], double dev[], double weight[]);

#endif /* __REMEZ_H__ */

