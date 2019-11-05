#include "dmv.h"
#include "dmv_impl.h"

int DenseMatVec_2dPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, 
                            int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  MPI_Comm comm = args->comm;
  MPI_Comm rowComm, colComm;
  int      size, rank, err, row, col, buffsize=0, pairDest=-1, pairOrigin=-1;
  int num_cols, num_rows;
  double *vecRight = NULL, *recvBuf;
  int *nOffsets = NULL, *nLocals, *mLocals;
  int rLocal = rEnd - rStart, lLocal = lEnd - lStart, nGlobal = nEnd - nStart, mGlobal = mEnd - mStart;
  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  /* TODO: implement a matrix-vector multiplication on a 2d matrix partition */
  /* HINT:
   * 1. Use DMVCommGetRankCoordinates2D() to get the coordinates of the current rank in a 2d grid of MPI processes.
   * 2. Use the coordinates as colorings to split the communicator into row and column communicators.
   * 3. Use MPI_Allgatherv() on the column communicator so that every process has the correct entries to multiply against its local portion of the matrix ([nStart, nEnd]).
   *      Look at DenseMatVec_RowPartition_Allgatherv() in dmv_matvec_row.c for an example of using MPI_Algatherv() in this way, but adapt it to the column communicator.
   *      You will need to allocate temporary space for the result of the allgatherv, and free it when you are done with it.
   * 4. Compute the local portion of the matrix vector product.
   * 5. Use MPI_Reduce_scatter() on the row communicator to sum all of the row contributions to vecLeftLocal.
   *      Look at DenseMatVec_ColPartition() in dmv_matvec_col.c for an example of use MPI_Reduce_scatter() in this wary, but adapt it to the row communicator.
   */
  DMVCommGetRankCoordinates2D(comm, &num_rows, &row, &num_cols, &col);
  err = MPI_Comm_split(comm, row, rank, &rowComm); MPI_CHK(err);
  err = MPI_Comm_split(comm, col, rank, &colComm); MPI_CHK(err);
    
  // vecRight
  vecRight = (double *) malloc(nGlobal * sizeof(*vecRight));
  nLocals = (int *) malloc(num_rows * sizeof(*nOffsets));
  err = MPI_Allgather(&rLocal, 1, MPI_INT, nLocals, 1, MPI_INT, colComm); MPI_CHK(err);
  nOffsets = (int *) malloc((num_rows + 1) * sizeof(*nOffsets));
  nOffsets[0] = 0;
  for (int q = 0; q < num_rows; q++) {
    nOffsets[q + 1] = nOffsets[q] + nLocals[q];
  }
  
  //Allgatherv
  err = MPI_Allgatherv(vecRightLocal, rLocal, MPI_DOUBLE, vecRight, nLocals, nOffsets, MPI_DOUBLE, colComm); MPI_CHK(err);
  
  //vecLeft
  double *vecLeft;
  vecLeft = (double *)malloc((mGlobal) * sizeof(double));
    //outer row, inner column
  for (int q = 0; q < mGlobal; q++) { 
    vecLeft[q] = 0;
    for (int i = 0; i < nGlobal; i++) { 
      vecLeft[q] += vecRight[i] *  matrixEntries[q*nGlobal + i];
    }
  }
  // free the vec temp
  free(vecRight);

  //get the origin , dest pair for send/receive
  pairOrigin = (rank/num_cols) + (rank%num_cols)*num_rows; 
  pairDest = row*num_cols + col;
    
  //Sendrecv
  MPI_Sendrecv(&lLocal, 1, MPI_INT, pairOrigin, 100, &buffsize, 1, MPI_INT, pairDest, 100, comm, MPI_STATUS_IGNORE);
  recvBuf = (double *) malloc(buffsize * sizeof(*recvBuf));
    
  if (!recvBuf) MPI_CHK(1);
 
  mLocals = (int *) malloc(num_cols * sizeof(int));
  err = MPI_Allgather(&buffsize, 1, MPI_INT, mLocals, 1, MPI_INT, rowComm); MPI_CHK(err);
  err = MPI_Reduce_scatter(MPI_IN_PLACE, vecLeft, mLocals, MPI_DOUBLE, MPI_SUM, rowComm); MPI_CHK(err);
  err = MPI_Sendrecv(vecLeft, buffsize,  MPI_DOUBLE, pairDest, 100, vecLeftLocal, lLocal, MPI_DOUBLE, pairOrigin, 100, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
    
  //free the temp
  free(recvBuf);
  free(mLocals);
  free(nLocals);
  free(nOffsets);
  free(vecLeft);
  err = MPI_Comm_free(&rowComm);
  MPI_CHK(err);
  err = MPI_Comm_free(&colComm);
  MPI_CHK(err);
  return 0;
}



