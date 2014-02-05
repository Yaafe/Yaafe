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

#include "MathUtils.h"

#include <iostream>
#include <algorithm>
#include <string.h>
//#include "clapack.h"

using namespace std;
using namespace Eigen;

namespace YAAFE
{

  VectorXd ecalc_hanning(int m, int len)
  {
    VectorXd han(m);
    for (int i=0;i<m;i++)
      han(i) = 0.5 * (1.0 - cos((i+1) * 2.0 * PI / (len + 1.0)));
    return han;
  }

  Eigen::VectorXd ehanning(int len)
  {
    VectorXd han(len);
    if (len%2 == 0)
    {
      int half = len/2;
      VectorXd h = ecalc_hanning(half,len);
      han.segment(0,half) = h;
      han.segment(half,len-half) = h.segment(0,half).reverse();
    } else {
      int half = (len+1)/2;
      VectorXd h = ecalc_hanning(half,len);
      han.segment(0,half) = h;
      han.segment(half,len-half) = h.segment(0,half-1).reverse();
    }
    return han;
  }

  Eigen::VectorXd ehanningPeriodic(int len)
  {
    VectorXd han(len);
    han(0) = 0;
    han.segment(1,len-1) = ehanning(len-1);
    return han;
  }

  Eigen::VectorXd ehamming(int len)
  {
    VectorXd ham(len);
    for (int i=0;i<len;i++)
      ham(i) = 0.54 - 0.46 * cos(2.0 * PI * i / (len - 1));
    return ham;
  }

  Eigen::VectorXd ehammingPeriodic(int len)
  {
    VectorXd ham = ehamming(len+1);
    return ham.segment(0,len);
  }

  void ac2lpc(const double* ac, double* outlpc, int nbCoeffs)
  {
    long double e = ac[0];
    long double lpc[nbCoeffs];
    for (int i = 1; i < nbCoeffs + 1; i++)
    {
      // compute reflection coefficient
      long double r = -ac[i];
      for (int j = 1; j < i; j++)
        r -= lpc[j - 1] * ac[i - j];
      r /= e;
      // update lpc coefficients
      lpc[i - 1] = r;
      for (int j = 1; j < (i + 1) / 2; j++)
      {
        long double tmp = lpc[j - 1];
        lpc[j - 1] += r * lpc[i - j - 1];
        lpc[i - j - 1] += r * tmp;
      }
      if (i % 2 == 0)
        lpc[i / 2 - 1] *= (1 + r);
      // update error
      e *= 1 - r * r;
    }
    for (int i=0;i<nbCoeffs;i++)
      outlpc[i] = lpc[i];
  }

#ifdef WITH_LAPACK
  void a2lsf(const double* lpc, int displacement, double* lsf, int nbLSF)
  {
    // should test if lpc is minimum-phase
    int nbLPC = nbLSF + 1 - (displacement > 1 ? displacement : 1);
    int pLen = 1 + nbLPC + displacement;
    int qLen = pLen;
    double ak[pLen];
    ak[0] = 1;
    for (int i = 0; i < nbLPC; i++)
      ak[i + 1] = lpc[i];
    for (int i = 0; i < displacement; i++)
      ak[1 + nbLPC + i] = 0;
    double rootR[pLen];
    double rootI[pLen];
    int nbRoots = 0;
    roots(ak, nbLPC + 1, rootR, rootI, &nbRoots);
    for (int i = 0; i < nbRoots; i++)
    {
      if (pow2(rootR[i]) + pow2(rootI[i]) >= 1)
      {
        cerr
          << "ERROR: try to compute LSF of polynomial which is not minimum-phase !"
          << endl;
        break;
      }
    }

    // form the sum and difference filters
    double pk[pLen];
    double qk[qLen];
    for (int i = 0; i < pLen; i++)
    {
      pk[i] = ak[i] + ak[pLen - 1 - i];
      qk[i] = ak[i] - ak[pLen - 1 - i];
    }

    // If order is even, remove the known roots at z = -1 for P and z = 1 for Q
    // If odd, remove both the roots from Q
    if (pLen % 2 == 1)
    {
      static double qFilt3[] =
      { 1, 0, -1 };
      deconv(qk, qLen, qFilt3, 3);
      qLen -= 2;
    }
    else
    {
      static double pFilt2[] =
      { 1, 1 };
      static double qFilt2[] =
      { 1, -1 };
      deconv(pk, pLen, pFilt2, 2);
      pLen--;
      deconv(qk, qLen, qFilt2, 2);
      qLen--;
    }

    // Compute the roots of the polynomials
    int rootIndex = 0;
    roots(pk, pLen, rootR, rootI, &nbRoots);
    for (int i = 0; i < nbRoots; i += 2)
      lsf[rootIndex++] = atan2(rootI[i], rootR[i]);
    roots(qk, qLen, rootR, rootI, &nbRoots);
    for (int i = 0; i < nbRoots; i += 2)
      lsf[rootIndex++] = atan2(rootI[i], rootR[i]);
    // sort lsfs
    std::sort(lsf, lsf + rootIndex);
    for (int i = rootIndex; i < nbLSF; i++)
      lsf[i] = 0;
    // Append the scaling parameter for Schussler LSF
    if (displacement == 0)
      lsf[nbLSF - 1] = pk[0];
  }

  void deconv(double* a, int alen, const double* b, int blen)
  {
    for (int i = 0; i < alen; ++i)
      for (int j = 1; j < min(blen, i + 1); j++)
        a[i] -= a[i - j] * b[j];
  }

  extern "C"
  {
    void dgeev_(char *jobvl, char *jobvr, int *n, double *a, int *lda, double *wr,
        double *wi, double *vl, int *ldvl, double *vr, int *ldvr, double *work,
        int *lwork, int *info);
  }

  void roots(const double* ar, int size, double* rootR, double* rootI,
      int* nbRoots)
  {
    // remember input polynomials is ar[0], ar[1], ar[2], ..., ar[size-1]

    // Strip trailing zeros, but remember them as roots at zero.
    //    int nbRootsZeros;
    //    while (size>0 && ar[size-1]==0)
    //    {
    //        size--;
    //        nbRootsZeros++;
    //    }

    // build companion matrix
    int N = size - 1;
    double* a = new double[N * N];
    memset(a, 0, N*N * sizeof(double));
    for (int i = 0; i < N; i++)
      a[i * N] = -ar[1 + i] / ar[0];
    for (int i = 0; i < N - 1; i++)
      a[(1 + i) + i * N] = 1;
    // allocate work space
    int lWork = 10 * N;
    double work[10 * N];
    int info;
    // call lapack routine
    dgeev_("N", "N", &N, a, &N, rootR, rootI, NULL,&N, NULL,&N, work, &lWork,
        &info);
    // release companion matrix
    delete[] a;
    // return nbRoots
    *nbRoots = N;
  }
#endif

} // YAAFE

