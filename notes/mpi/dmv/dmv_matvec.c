
#include "dmv.h"
#include "dmv_impl.h"


int DenseMatVec(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int nLocal, const double *vecRightLocal, int mLocal, double *vecLeftLocal)
{
  int err;

  for (int i = 0; i < mLocal; i++) {vecLeftLocal[i] = 0.;}
  switch (args->partition_strategy) {
  case PARTITION_ROWS:
    if (args->verbosity) {MPI_LOG(args->comm, "DenseMatVec row partition\n");}
    err = DenseMatVec_RowPartition(args, mStart, mEnd, nStart, nEnd, matrixEntries, nLocal, vecRightLocal, mLocal, vecLeftLocal); MPI_CHK(err);
    break;
  case PARTITION_COLS:
    if (args->verbosity) {MPI_LOG(args->comm, "DenseMatVec column partition\n");}
    err = DenseMatVec_ColPartition(args, mStart, mEnd, nStart, nEnd, matrixEntries, nLocal, vecRightLocal, mLocal, vecLeftLocal); MPI_CHK(err);
    break;
  case PARTITION_2D:
    if (args->verbosity) {MPI_LOG(args->comm, "DenseMatVec 2d partition\n");}
    err = DenseMatVec_2dPartition(args, mStart, mEnd, nStart, nEnd, matrixEntries, nLocal, vecRightLocal, mLocal, vecLeftLocal); MPI_CHK(err);
    break;
  default:
    MPI_LOG(args->comm, "Unknown partition type %d\n", args->partition_strategy);
    MPI_CHK(1);
  }
  return 0;
}
