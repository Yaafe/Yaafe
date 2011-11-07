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

#ifndef MULTI_STAGE_H_
#define MULTI_STAGE_H_

/**
 * Definition of a multistage filter plan.
 * - nb_stages: number of stages
 * - L: array of nb_stages interpolation factors
 * - M: array of nb_stages decimation factors
 */
struct PMultiStageDef {
	int nb_stages;
	int* L;
	int* M;
};

struct PMultiStageDef* get_predef_ratios(int fsin, int fsout);
struct PMultiStageDef* get_user_ratios(int fsin, int fsout, const char* userdef);
struct PMultiStageDef* build_auto_ratios(int fsin, int fsout, double tol);
struct PMultiStageDef* build_fast_ratios(int fsin, int fsout, double tol, double bandwidth,double rp,double rs);
void destroy_multistagedef(struct PMultiStageDef*);

#endif /* MULTI_STAGE_H_ */
