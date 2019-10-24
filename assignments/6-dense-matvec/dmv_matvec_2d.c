#include "dmv.h"
#include "dmv_impl.h"
#include "dmv_indices.h"

int DenseMatVec_2dPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{

  MPI_Comm comm = args->comm;
  int size, rank, err;

  err = MPI_Comm_size(comm, &size);
  MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank);
  MPI_CHK(err);


  /* implement a matrix-vector multiplication on a 2d matrix partition */
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
  // printf("%d-th node:\tmStart: %d\t, mEnd: %d\t, nStart: %d\t, nEnd: %d\t, lStart: %d\t, lEnd: %d\t, rStart: %d\t, rEnd: %d\n", rank, mStart, mEnd, nStart, nEnd, lStart, lEnd, rStart, rEnd);

  int numRows, row, numCols, col;
  numRows = numCols = row = col = -1;
  err = DMVCommGetRankCoordinates2D(comm, &numRows, &row, &numCols, &col);
  MPI_CHK(err);

  MPI_Comm colComm, rowComm;
  err = MPI_Comm_split(comm, col, rank, &colComm);
  MPI_CHK(err);
  err = MPI_Comm_split(comm, row, rank, &rowComm);
  MPI_CHK(err);

  int colCommSize = 0;
  int rowCommSize = 0;
  err = MPI_Comm_size(colComm, &colCommSize);
  MPI_CHK(err);
  err = MPI_Comm_size(rowComm, &rowCommSize);
  MPI_CHK(err);


  int rLocal = rEnd - rStart;
  double *temp_vec_right;
  temp_vec_right = (double *)malloc((nEnd - nStart) * sizeof(double));
  if (!temp_vec_right)
    MPI_CHK(1);

  int *nLocals;
  nLocals = (int *)malloc(numRows * sizeof(int));
  if (!nLocals)
    MPI_CHK(1);
  int *nOffsets;
  nOffsets = (int *)malloc((numRows + 1) * sizeof(int));
  if (!nOffsets)
    MPI_CHK(1);

  err = MPI_Allgather(&rLocal, 1, MPI_INT, nLocals, 1, MPI_INT, colComm);
  MPI_CHK(err);

  nOffsets[0] = 0;
  for (int q = 0; q < numRows; q++)
  {
    nOffsets[q + 1] = nOffsets[q] + nLocals[q];
  }
  err = MPI_Allgatherv(vecRightLocal, rLocal, MPI_DOUBLE, temp_vec_right, nLocals, nOffsets, MPI_DOUBLE, colComm);

  MPI_CHK(err);

  int num_cols = nEnd - nStart;
  int num_rows = mEnd - mStart;

  double *vecLeft; 
  vecLeft = (double *)malloc((nEnd - nStart) * sizeof(double));
  if (!vecLeft)
    MPI_CHK(1);
 
  for (int r = 0; r < num_rows; r++)
  {
    double val = 0;
    for (int c = 0; c < num_cols; c++)
    {
      val += matrixEntries[r * num_cols + c] * temp_vec_right[c];
    }
    vecLeft[r] = val;
  }

  int lLocal = lEnd - lStart;
  int dest, origin;
  dest = GetDest(numRows, numCols, row, col);
  origin = GetOrigin(numRows, numCols, row, col);


  err = MPI_Sendrecv_replace(&lLocal, 1, MPI_INT, dest, 100, origin, 100, comm, MPI_STATUS_IGNORE);
  MPI_CHK(err);

  int *lLocals;
  lLocals = (int *)malloc(numCols * sizeof(int));
  if (!lLocals)
    MPI_CHK(1);
  err = MPI_Allgather(&lLocal, 1, MPI_INT, lLocals, 1, MPI_INT, rowComm);
  MPI_CHK(err);



  err = MPI_Reduce_scatter(MPI_IN_PLACE, vecLeft, lLocals, MPI_DOUBLE, MPI_SUM, rowComm);
  MPI_CHK(err);


  err = MPI_Sendrecv(vecLeft, lLocal, MPI_DOUBLE, origin, 100,
                     vecLeftLocal, lEnd - lStart, MPI_DOUBLE, dest, 100, comm, MPI_STATUS_IGNORE);
  MPI_CHK(err);


  err = MPI_Comm_free(&rowComm);
  MPI_CHK(err);
  err = MPI_Comm_free(&colComm);
  MPI_CHK(err);
  free(nOffsets);
  free(nLocals);
  free(lLocals);

  return 0;
}
