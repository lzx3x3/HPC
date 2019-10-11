/* Distributed dense matrix-vector product examples */

#include <stdlib.h>
#include <math.h>
#include "dmv.h"
#include "dmv_impl.h"

enum {FILL_MATRIX, FILL_VECTOR};

int MatrixGetLocalSize(Args args, int *local_m, int *local_n)
{
  int scale = args->scale;
  int targetGlobalSize = (int) pow(2., scale);
  int mGlobal, nGlobal, iStart, iEnd, jStart, jEnd, m, n;
  int size, rank, err;

  err = MPI_Comm_size (args->comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank (args->comm, &rank); MPI_CHK(err);

  mGlobal = (int) pow(2., scale / 2);
  nGlobal = targetGlobalSize / mGlobal;

  if (args->verbosity) MPI_LOG(args->comm, "Random matrix: target global size %d x %d\n", mGlobal, nGlobal);

  iStart = (rank * mGlobal) / size;
  iEnd   = ((rank + 1) * mGlobal) / size;
  jStart = (rank * nGlobal) / size;
  jEnd   = ((rank + 1) * nGlobal) / size;

  m = iEnd - iStart;
  n = jEnd - jStart;

  if (args->verbosity > 1) {
    for (int i = 0; i < size; i++) {
      if (rank == i) {
        MPI_LOG(MPI_COMM_SELF, "Random matrix: local size %d x %d\n", m, n);
      }
      err = MPI_Barrier(args->comm); MPI_CHK(err);
    }
  }

  *local_m = m;
  *local_n = n;

  return 0;
}

int MatrixGetEntries(Args args, int rowStart, int rowEnd, int colStart, int colEnd, double *entries)
{
  int colDim = colEnd - colStart;
  threefry4x64_ctr_t count = {{0}};

  count.v[0] = FILL_MATRIX;

  for (int i = rowStart; i < rowEnd; i++) {
    count.v[1] = i;
    for (int j = (colStart / 4) * 4; j < colEnd; j += 4) {
      threefry4x64_ctr_t val;
      int kStart, kEnd;
      double scale = 1. / (UINT64_MAX + 1.);
      double shift = 0.5 * scale;

      count.v[1] = j;
      val = threefry4x64(count,args->key);

      kStart = j < colStart ? colStart : j;
      kEnd   = j + 4 > colEnd ? colEnd : j + 4;
      for (int k = kStart; k < kEnd; k++) {
        entries[colDim * (i - rowStart) + (k - colStart)] = 2. * (scale * val.v[k - j] + shift) - 1.;
      }
    }
  }

  return 0;
}

int VecGetEntries(Args args, int colStart, int colEnd, double *entries)
{
  threefry4x64_ctr_t count = {{0}};

  count.v[0] = FILL_VECTOR;

  for (int j = (colStart / 4) * 4; j < colEnd; j += 4) {
    threefry4x64_ctr_t val;
    int kStart, kEnd;
    double scale = 1. / (UINT64_MAX + 1.);
    double shift = 0.5 * scale;

    count.v[1] = j;
    val = threefry4x64(count,args->key);

    kStart = j < colStart ? colStart : j;
    kEnd   = j + 4 > colEnd ? colEnd : j + 4;
    for (int k = kStart; k < kEnd; k++) {
      entries[k - colStart] = 2. * (scale * val.v[k - j] + shift) - 1.;
    }
  }
  count.v[0] = FILL_MATRIX;

  return 0;
}



