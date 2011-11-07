/**
 * Yaafe
 *
 * Copyright (c) 2009-2010 Institut Télécom - Télécom Paristech
 * Télécom ParisTech / dept. TSI
 *
 * Author : Benoit Mathieu
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

#ifndef MATHUTILS_H_
#define MATHUTILS_H_

#include <Eigen/Dense>

#define PI 3.14159265358979323846
#define EPS 2.220446049250313e-16

namespace YAAFE {

Eigen::VectorXd ehanning(int len);
Eigen::VectorXd ehanningPeriodic(int len);
Eigen::VectorXd ehamming(int len);
Eigen::VectorXd ehammingPeriodic(int len);

inline double pow2(double x) { return x*x; }
inline double pow3(double x) { return x*x*x; }
inline double pow4(double x) { return x*x*x*x; }

/**
 * Compute LPC coefficients from autocorrelation coefficients.
 * Return's LPC coefficients corresponding to lag 1 to nbCoeffs
 * Needs nbCoeffs+1 autocorrelation coefficients.
 */
void ac2lpc(const double* ac, double* lpc, int nbCoeffs);


#ifdef WITH_LAPACK
/**
 * Compute LSF from LPC coefficients.
 * If displacement <= 1, requires nbLSF LPC coefficients
 * if displacement > 1, requires nbLSF+1-displacement LPC coefficients
 */
void a2lsf(const double* lpc, int displacement, double* lsf, int nbLSF);

/**
 * Deconvolution and polynomial division
 * identical to matlab's deconv function
 */
void deconv(double* a, int alen, const double* b, int blen);

/**
 * compute roots of polynomial defined by coefficients [ar[0] ar[1] ... ar[size-1]]
 * return real part and imaginary part in rootR and rootI arrays of size 'size-1'
 * with consecutive conjugate roots
 */
void roots(const double* ar, int size,double* rootR,double* rootI, int* nbRoots);
#endif


} // YAAFE

#endif /* MATHUTILS_H_ */
