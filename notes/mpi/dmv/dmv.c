/* Distributed dense matrix-vector product examples */

#include <stdlib.h>
#include <math.h>
#include "dmv.h"
#include "dmv_impl.h"

enum {FILL_MATRIX, FILL_VECTOR};

int VectorsGetLocalSize(Args args, int *local_m, int *local_n)
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

/* Given the number of processes in the MPI communicator, compute a 2D
 * grid size (num_rows x num_cols) for the communicator and the coordinates of
 * the current rank (row, col).  Make sure to use a column major ordering,
 * so that rank 1 gets coordates (1, 0), not (0,1) */
int DMVCommGetRankCoordinates2D(MPI_Comm comm, int *num_rows_p, int *row_p, int *num_cols_p, int *col_p)
{
  int num_cols, num_rows, col, row;

  num_cols = num_rows = col = row = -1;
  /* TODO: HINT, lookup MPI_Dims_create() */
  *num_cols_p = num_cols;
  *num_rows_p = num_rows;
  *col_p = col;
  *row_p = row;
  return 0;
}

/* Given arguments (which include a communicator, args->comm),
 * offsets for each rank in the left and right vectors compatible with a matrix (lOffsets and rOffsets),
 * compute which entries in the matrix this MPI rank will own, given by the column ranges [mStart, mEnd) and row ranges [nStart, nEnd) */
int MatrixGetLocalRange2d(Args args, const int *lOffsets, const int *rOffset, int *mStart_p, int *mEnd_p, int *nStart_p, int *nEnd_p)
{
  MPI_Comm comm = args->comm;
  int      mStart, mEnd, nStart, nEnd;
  int      size, rank;
  int      err;

  /* initialize to bogus values */
  mStart = mEnd = nStart = nEnd = -1;
  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  /* TODO: compute mStart, mEnd, nStart, and nEnd. HINT: use DMVCommGetRankCoordinates2D() to get the
   * number of block columns and rows used to partition the matrix, mBlock and nBlock.
   * The block row i should contain the same rows as are in the left vector for
   * ranks (i * nBlock, i * nBlock + 1, ..., (i + 1) * nBlock - 1).
   * The block column j should contain the same columns as are in the right
   * vector for ranks (j * mBlock, j * mBlock + 1, ..., (j + 1) * mBlock - 1).
   */
  *mStart_p = mStart;
  *mEnd_p   = mEnd;
  *nStart_p = mStart;
  *nEnd_p   = mEnd;
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



