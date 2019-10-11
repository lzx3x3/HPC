#include "dmv.h"
#include "dmv_impl.h"

int DenseMatVec_ColPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int nLocal, const double *vecRightLocal, int mLocal, double *vecLeftLocal)
{
  double *vecLeft;
  int    *mLocals;
  int    size, err;
  int    mGlobal = mEnd - mStart;

  err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);

  /* Create my own copy of vecLeft */
  vecLeft = (double *) malloc(mGlobal * sizeof(*vecLeft));
  if (!vecLeft) MPI_CHK(1);
  /* Get the offsets of the column space */
  mLocals = (int *) malloc((size + 1) * sizeof(*mLocals));
  if (!mLocals) MPI_CHK(1);

  err = MPI_Allgather(&mLocal, 1, MPI_INT, mLocals, 1, MPI_INT, args->comm); MPI_CHK(err);

  /* Start computing right away */
  for (int i = 0; i < mGlobal; i++) {
    double val = 0.;

    for (int j = 0; j < nLocal; j++) {
      val += matrixEntries[i * nLocal + j] * vecRightLocal[j];
    }
    vecLeft[i] = val;
  }

  /* reduce the results from every process and scatter them into the local
   * portions */
  err = MPI_Reduce_scatter(vecLeft, vecLeftLocal, mLocals, MPI_DOUBLE, MPI_SUM, args->comm); MPI_CHK(err);
  free(mLocals);
  free(vecLeft);
  return 0;
}
