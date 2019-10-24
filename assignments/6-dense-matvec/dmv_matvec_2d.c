#include "dmv.h"
#include "dmv_impl.h"

int DenseMatVec_2dPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  MPI_Comm comm = args->comm;
  MPI_Comm row_comm;
  MPI_Comm col_comm;
  int color_row, color_col;
  int      size, rank, err;
  int num_rows_p, row_p, num_cols_p, col_p;

//   printf("matrix_M: %d\n %d\n", rStart,rEnd);
//   printf("matrix_V: %d\n %d\n", lStart,lEnd);
  

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
  DMVCommGetRankCoordinates2D(comm, &num_rows_p, &row_p, &num_cols_p, &col_p);
//   printf("rank %d dim: col: %d ,row %d\n", rank, col_p, row_p);
  color_row = row_p;
  color_col = col_p;
//   printf("COLOR ROW: %d COLOR_COL : %d\n",row_p,col_p);
  MPI_Comm_split(comm, color_row, rank, &row_comm);
  MPI_Comm_split(comm, color_col, rank, &col_comm);
  int row_size, row_rank;
  err = MPI_Comm_size(row_comm, &row_size); MPI_CHK(err);
  err = MPI_Comm_rank(row_comm, &row_rank); MPI_CHK(err);
  int col_size, col_rank;
  err = MPI_Comm_size(col_comm, &col_size); MPI_CHK(err);
  err = MPI_Comm_rank(col_comm, &col_rank); MPI_CHK(err);
    
  printf("%d-th node:\tm: %d\t, n: %d\t, l: %d\t, r: %d\n", rank, mStart, nStart, lStart , rStart );
    
    
  /////////////////////////////////////////////////////////////////////////////////
  
  /* Set up a buffer to receive the other local portions of the right vector */
  double      *vecRight;
  int         *nLocals;
  int         *nOffsets;
  int         nLocal = rEnd - rStart, mLocal = lEnd - lStart, lLocal = lEnd - lStart;
  vecRight = (double *) malloc((nEnd-nStart) * sizeof(double));
  if (!vecRight) MPI_CHK(1);
  /* Get the counts for every process */
  nLocals = (int *) malloc(num_rows_p* sizeof(int));
  if (!nLocals) MPI_CHK(1);
  /* Get the offsets for every process */
  nOffsets = (int *) malloc((num_rows_p + 1) * sizeof(int));
  if (!nOffsets) MPI_CHK(1);

  /* Gather the counts for every process */
  err = MPI_Allgather(&nLocal, 1, MPI_INT, nLocals, 1, MPI_INT, col_comm); MPI_CHK(err);
  /* Turn the counts into the offsets */
  nOffsets[0] = 0;
  for (int q = 0; q < num_rows_p; q++) {
    nOffsets[q + 1] = nOffsets[q] + nLocals[q];
  }
  /* Gather the whole vector on each process */
  err = MPI_Allgatherv(vecRightLocal, nLocal, MPI_DOUBLE, vecRight, nLocals, nOffsets, MPI_DOUBLE, col_comm); MPI_CHK(err);
 
  for (int j = 0; j <  nLocal; j++) {
    double val = 0.;
    for (int k = 0; k < mLocal; k++) {
      val += matrixEntries[j * mLocal + k] * vecRight[k];
    }
    vecLeftLocal[j] += val;
  }
    int origin_rank = row_p*num_cols_p+col_p;
    int dest_rank = origin_rank/num_cols_p+origin_rank%num_cols_p*num_rows_p;
    err = MPI_Sendrecv_replace(&lLocal, 1, MPI_INT, dest_rank, 100, origin_rank, 100, comm, MPI_STATUS_IGNORE);
  MPI_CHK(err);
    err = MPI_Reduce_scatter(MPI_IN_PLACE, vecLeft, lLocals, MPI_DOUBLE, MPI_SUM, rowComm);
  MPI_CHK(err);
    
//   err = MPI_Sendrecv_replace(&lLocal, 1, MPI_INT, buddy_to, 100, buddy_from, 100, comm, MPI_STATUS_IGNORE);
//   MPI_CHK(err);  
//   err = MPI_Reduce_scatter(MPI_IN_PLACE, vecLeft, lLocals, MPI_DOUBLE, MPI_SUM, rowComm);
//   MPI_CHK(err);

//   // printf("I am %d and lLocal: %d, lEnd-Start: %d\n", rank, lLocal, lEnd - lStart);
//   err = MPI_Sendrecv(vecLeft, lLocal, MPI_DOUBLE, buddy_from, 100,
//                      vecLeftLocal, lEnd - lStart, MPI_DOUBLE, buddy_to, 100, comm, MPI_STATUS_IGNORE);
//   MPI_CHK(err);  
    
    
    
    
  free(vecRight);
  free(nLocals);
  free(nOffsets);

 
 
  ////////////////////////////////////////////////////////////////////////////////////
    
    
//   printf("WORLD RANK/SIZE: %d/%d \t ROW RANK/SIZE: %d/%d\n", rank, size, row_rank, row_size);
//   printf("WORLD RANK/SIZE: %d/%d \t COL RANK/SIZE: %d/%d\n", rank, size, col_rank, col_size);
    
//   printf("-----------------------------\n");
  return 0;
}
