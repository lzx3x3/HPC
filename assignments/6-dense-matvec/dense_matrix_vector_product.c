/* Distributed dense matrix-vector product: using only blocking synchronous sends and
 * receives */

/* Just a little preamble to make sure that tictoc work with c99 */
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif

#include "dmv.h"
#include <math.h>
#include <tictoc/tictoc.h>

static int GetNorm(MPI_Comm comm, int nLocal, double *entries, double *norm)
{
  int    err;
  double localNorm2 = 0., globalNorm2;

  for (int i = 0; i < nLocal; i++) {localNorm2 += entries[i] * entries[i];}

  err = MPI_Allreduce(&localNorm2, &globalNorm2, 1, MPI_DOUBLE, MPI_SUM, comm); MPI_CHK(err);

  *norm = sqrt(globalNorm2);
  return 0;
}

int main (int argc, char **argv)
{
  Args    args = NULL;
  int     err;
  int     lLocal, rLocal, rOffset, lOffset, rGlobal, lGlobal;
  int     lStart, lEnd, rStart, rEnd;
  int     mStart, mEnd, nStart, nEnd;
  int     rank, size;
  int     partitionType;
  int     numTests = 10;
  double *matrixEntries = NULL;
  double *vecRightLocal, *vecLeftLocal;
  double  time;

  /* Intialize the problem, read in arguments into a structure defined in
   * dmv.c */
  err = MPI_Init(&argc, &argv);                        MPI_CHK(err);
  err = MPI_Comm_size(MPI_COMM_WORLD, &size);          MPI_CHK(err);
  err = MPI_Comm_rank(MPI_COMM_WORLD, &rank);          MPI_CHK(err);
  err = ArgsCreate(MPI_COMM_WORLD, argc, argv, &args); MPI_CHK(err);

  /* Get the local sizes: the number of entries in the left/output vector (lLocal)
   * and right/input vector (rLocal) in this process's memory */
  err = VectorsGetLocalSize(args, &lLocal, &rLocal); MPI_CHK(err);

  /* Allocate space for the local portions of the input and output
   * vectors */
  vecLeftLocal = (double *) malloc(lLocal * sizeof (*vecLeftLocal));
  if (!vecLeftLocal) MPI_CHK(1);
  vecRightLocal = (double *) malloc(rLocal * sizeof (*vecRightLocal));
  if (!vecRightLocal) MPI_CHK(1);

  /* Now we need to create entries in the input (right) vector.  In this
   * example, the values in the vector are uniquely determined by their
   * *global* index.  The array vecRightLocal has indices in [0, rLocal),
   * but if we considered these local arrays to be partitions of a global
   * array, each entry also has a global index that assigned in order
   * over all processes.  So on process 0, the local indices correspond
   * to the global indicecs.  But on process 1, we start counting global
   * indices from rLocal_0, and on process 2 we start counting global indices
   * from (rLocal_0 + rLocal_1), etc.  We need to compute the *offset* from
   * local indices to global indices by adding up the local sizes for all of
   * the lesser ranks. This is what the VecGetOffset() function computes. */
  err = VecGetOffset(args, rLocal, &rOffset); MPI_CHK(err);

  /* Now that we know the indices, fill the values of the local portion of the
   * right vector */
  err = VecGetEntries(args, rOffset, rOffset + rLocal, vecRightLocal); MPI_CHK(err);
  rStart = rOffset;
  rEnd   = rOffset + rLocal;
  err = VecGetOffset(args, lLocal, &lOffset); MPI_CHK(err);
  lStart = lOffset;
  lEnd   = lOffset + lLocal;

  /* As a check on filling the entries correctly, the norm of the vector
   * should be independent of the partition */
  {
    double rightNorm;

    err = GetNorm(MPI_COMM_WORLD, rLocal, vecRightLocal, &rightNorm); MPI_CHK(err);
    if (!rank) {
      printf("Right vector norm: %g\n", rightNorm);
    }
  }

  err = ArgsGetPartitionType(args, &partitionType); MPI_CHK(err);

  switch (partitionType) {
  case PARTITION_ROWS:
    {
      /* We need to know the global size of the right vector so that we can
       * allocate and fill complete rows of the matrix */
      err = VecGetGlobalSize(args, rLocal, &rGlobal); MPI_CHK(err);

      mStart = lStart;
      mEnd   = lEnd;
      nStart = 0;
      nEnd   = rGlobal;
    }
    break;
  case PARTITION_COLS:
    {
      /* We need to know the global size of the right vector so that we can
       * allocate and fill complete rows of the matrix */
      err = VecGetGlobalSize(args, lLocal, &lGlobal); MPI_CHK(err);

      mStart = 0;
      mEnd   = lGlobal;
      nStart = rStart;
      nEnd   = rEnd;
    }
    break;
  case PARTITION_2D:
    {
      int *rLayout, *lLayout;

      rLayout = (int *) malloc((size + 1) * sizeof (*rLayout));
      if (!rLayout) MPI_CHK(1);
      lLayout = (int *) malloc((size + 1) * sizeof (*lLayout));
      if (!lLayout) MPI_CHK(1);

      /* Get the first global vector index for every process (with the last value in the
       * array being the full length of the vector) */
      err = VecGetLayout(args, rLocal, rLayout); MPI_CHK(err);
      err = VecGetLayout(args, lLocal, lLayout); MPI_CHK(err);

      /* Use the vector layouts to determine the compatible 2d matrix partition */
      err = MatrixGetLocalRange2d(args, lLayout, rLayout, &mStart, &mEnd, &nStart, &nEnd); MPI_CHK(err);
      free(lLayout);
      free(rLayout);
    }
    break;
  default:
    MPI_LOG(MPI_COMM_WORLD, "Unrecognized partition type %d\n", partitionType); MPI_CHK(1);
  }

  /* Allocate space for the matrix entries */
  matrixEntries = (double *) malloc((mEnd - mStart) * (nEnd - nStart) * sizeof(*matrixEntries));
  if (!matrixEntries) MPI_CHK(1);
  /* As with the vectors, the entries of the matrix are uniquely determined by
   * their indices, so we can compute them from knowing the bounds of the
   * local subset of the matrix */
  err = MatrixGetEntries(args, mStart, mEnd, nStart, nEnd, matrixEntries); MPI_CHK(err);

  {
    double matNorm;

    /* As a check on filling the entries correctly, the Frobenius norm of the
     * matrix should be independent of the partition: running this example
     * with different number of processes should yield the same norm */
    err = GetNorm(MPI_COMM_WORLD, (mEnd - mStart) * (nEnd - nStart), matrixEntries, &matNorm); MPI_CHK(err);
    if (!rank) {
      printf("Matrix Frobenius norm: %g\n", matNorm);
    }
  }

  time = 0.;
  for (int i = 0; i < numTests + 1; i++) {
    TicTocTimer timer;
    double      thistime;
    /* Perform the Matrix Vector Product */
    timer = tic();
    err = DenseMatVec(args, mStart, mEnd, nStart, nEnd, matrixEntries, rStart, rEnd, vecRightLocal, lStart, lEnd, vecLeftLocal); MPI_CHK(err);
    thistime = toc(&timer);
    if (i) {
      time += thistime;
    }
  }
  time /= numTests;
  MPI_LOG(MPI_COMM_WORLD, "Average time per matvec: %g\n", time);

  {
    double resultNorm;

    /* As a check on filling the entries correctly, the norm of the product should be independent of the partition */
    err = GetNorm(MPI_COMM_WORLD, lLocal, vecLeftLocal, &resultNorm); MPI_CHK(err);
    if (!rank) {
      printf("Left vector norm: %g\n", resultNorm);
    }
  }

  free(matrixEntries);
  free(vecRightLocal);
  free(vecLeftLocal);

  err = ArgsDestroy(&args);
  err = MPI_Finalize();
  return err;
}
