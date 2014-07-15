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

int nextpow2(int v);
inline double pow2(double x) { return x*x; }
inline double pow3(double x) { return x*x*x; }
inline double pow4(double x) { return x*x*x*x; }

} // YAAFE

#endif /* MATHUTILS_H_ */
