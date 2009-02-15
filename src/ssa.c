/*
 *   R package for Singular Spectrum Analysis
 *   Copyright (c) 2008 Anton Korobeynikov <asl@math.spbu.ru>
 *
 *   This program is free software; you can redistribute it
 *   and/or modify it under the terms of the GNU General Public
 *   License as published by the Free Software Foundation;
 *   either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
 *   MA 02139, USA.
 */

#include <R.h>
#include <Rinternals.h>
#include <R_ext/Utils.h>

/* This is just direct R-to-C translation and will need to be rethought in the
 * future */

static R_INLINE void hankelize(double *F,
                               double *U, double *V,
                               R_len_t L, R_len_t K) {
  R_len_t i, N = K + L - 1;
  R_len_t leftu, rightu, leftv, rightv, l, j;

  for (i = 0; i < N; ++i) {
    double s = 0;

    if (i < L) {
      leftu = i;
      leftv = 0;
    } else {
      leftu = L - 1;
      leftv = i - L + 1;
    }
    if (i < K) {
      rightu = 0;
      rightv = i;
    } else {
      rightu = i - K + 1;
      rightv = K - 1;
    }

    l = leftu - rightu + 1;

    for (j = 0; j < l; ++j) {
      s += U[leftu - j] * V[leftv + j];
    }

    F[i] = s / (double) l;
  }
}

SEXP hankelize_one(SEXP U, SEXP V) {
  double *rU = REAL(U), *rV = REAL(V), *rF;
  R_len_t L, K;
  SEXP F;

  /* Calculate length of inputs */
  L = length(U); K = length(V);

  /* Allocate buffer for output */
  PROTECT(F = allocVector(REALSXP, K + L - 1));
  rF = REAL(F);

  /* Perform the actual hankelization */
  hankelize(rF, rU, rV, L, K);

  UNPROTECT(1);
  return F;
}

SEXP hankelize_multi(SEXP U, SEXP V) {
  double *rU = REAL(U), *rV = REAL(V), *rF;
  R_len_t L, K, N, i, count;
  SEXP F;
  int *dimu, *dimv;

  /* Calculate length of inputs and output */
  dimu = INTEGER(getAttrib(U, R_DimSymbol));
  dimv = INTEGER(getAttrib(V, R_DimSymbol));
  L = dimu[0]; K = dimv[0];
  if ((count = dimu[1]) != dimv[1])
    error("Both 'U' and 'V' should have equal number of columns");
  N = K + L - 1;

  /* Allocate buffer for output */
  PROTECT(F = allocMatrix(REALSXP, N, count));
  rF = REAL(F);

  /* Perform the actual hankelization */
  for (i = 0; i < count; ++i) {
    R_CheckUserInterrupt();
    /* TODO: nice target for OpenMP stuff */
    hankelize(rF+i*N, rU+i*L, rV+i*K, L, K);
  }

  UNPROTECT(1);
  return F;
}