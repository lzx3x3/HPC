#include "dmv.h"
#include "dmv_impl.h"

static int DenseMatVec_RowPartition_Ssend(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  double      *recvBuf, *sendBuf;
  int         err, rank, size, nOffset = rStart, nLocal = rEnd - rStart, mLocal = lEnd - lStart, nGlobal = nEnd - nStart;
  int         nRecv;

  err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(args->comm, &rank); MPI_CHK(err);

  /* Set up a buffer to receive the other local portions of the right vector */
  recvBuf = (double *) malloc(nGlobal * sizeof(*recvBuf));
  if (!recvBuf) MPI_CHK(1);
  /* Set up a buffer to send the other local portions of the right vector */
  sendBuf = (double *) malloc(nGlobal * sizeof(*recvBuf));
  if (!sendBuf) MPI_CHK(1);

  /* Copy vecLocal into recvBuf so that we can do the same thing in every
   * iteration: compute the portion of the matvec for the section in the
   * recvBuf, copy the recvBuf to the SendBuf, and send it along to the next
   * process */
  for (int i = 0; i < nLocal; i++) {recvBuf[i] = vecRightLocal[i];}
  nRecv = nLocal;

  /* For each value of `shift`, we are computing the portion of the matvec
   * that corresponds to the submatrix A_{rank, (rank + shift) % size} */
  for (int shift = 0; shift < size; shift++) {
    MPI_Status recvStatus;
    int recvFrom = (rank + 1) % size;
    int sendTo   = (rank - 1 + size) % size;
    int nSend;

    /* Compute the local portion A_{rank, (rank + shift) mod size} * vecRight_{(rank + shift) mod size} */
    for (int j = 0; j < mLocal; j++) {
      double val = 0.;
      for (int k = 0; k < nRecv; k++) {
        val += matrixEntries[j * nGlobal + nOffset + k] * recvBuf[k];
      }
      vecLeftLocal[j] += val;
    }
    nOffset = (nOffset + nRecv) % nGlobal;
    if (shift == size - 1) continue; /* No need to communicate anymore */
    /* Copy recvBuf to sendBuf */
    for (int i = 0; i < nRecv; i++) {sendBuf[i] = recvBuf[i];}
    nSend = nRecv;
    /* To avoid deadlock, have odd proceses receive then send and even
     * processes send then receive.  We will have an issue with an odd number
     * of processes: size - 1 is even and receives from 0, which is also even.
     * Both will try to send first, so the send initiated from process 0 will
     * have to wait for the send initiated by send - 1 to complete.  It will
     * therefore only complete when process size -1 posts its recv in the
     * second round.  But then the send from process 1 to process 0 will be
     * blocked.  Therefore there whole process will take 3 rounds to complete
     * */
    if (rank % 2) { /* Odd processes receive then send */
      err = MPI_Recv (recvBuf, nGlobal, MPI_DOUBLE, recvFrom, TAG_SSEND, args->comm, &recvStatus); MPI_CHK(err);
      err = MPI_Ssend(sendBuf, nSend,  MPI_DOUBLE, sendTo,   TAG_SSEND, args->comm); MPI_CHK(err);
    } else { /* Even processes send then receive */
      err = MPI_Ssend(sendBuf, nSend,  MPI_DOUBLE, sendTo,   TAG_SSEND, args->comm); MPI_CHK(err);
      err = MPI_Recv (recvBuf, nGlobal, MPI_DOUBLE, recvFrom, TAG_SSEND, args->comm, &recvStatus); MPI_CHK(err);
    }
    err = MPI_Get_count(&recvStatus, MPI_DOUBLE, &nRecv); MPI_CHK(err);
  }
  free(sendBuf);
  free(recvBuf);
  return 0;
}

static int DenseMatVec_RowPartition_Issend(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  double      *recvBuf;
  int         *nOffsets;
  MPI_Request *recvReqs;
  MPI_Request *sendReqs;
  int         *indices;
  int         err, rank, size, nGlobal = nEnd - nStart;
  int         nLocal = rEnd - rStart, mLocal = lEnd - lStart;

  err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(args->comm, &rank); MPI_CHK(err);

  /* Set up a buffer to receive the other local portions of the right vector */
  recvBuf = (double *) malloc(nGlobal * sizeof(*recvBuf));
  if (!recvBuf) MPI_CHK(1);
  /* To put all of the recvs in the right place, we need to know the offsets
   * of every other process */
  nOffsets = (int *) malloc((size + 1) * sizeof(*nOffsets));
  if (!nOffsets) MPI_CHK(1);
  err = VecGetLayout(args, nLocal, nOffsets); MPI_CHK(err);

  /* We will have non-blocking requests for every Issend/Irecv, so we need
   * arrays of requests */
  recvReqs = (MPI_Request *) malloc(size * sizeof(*recvReqs));
  if (!recvReqs) MPI_CHK(1);
  sendReqs = (MPI_Request *) malloc(size * sizeof(*sendReqs));
  if (!sendReqs) MPI_CHK(1);
  indices = (int *) malloc(size * sizeof(*indices));
  if (!indices) MPI_CHK(1);

  /* Post the receives and the sends */
  for (int q = 0; q < size; q++) {
    err = MPI_Irecv(recvBuf + nOffsets[q], nOffsets[q+1] - nOffsets[q], MPI_DOUBLE, q, TAG_ISSEND, args->comm, recvReqs + q); MPI_CHK(err);
  }
  for (int q = 0; q < size; q++) {
    err = MPI_Issend(vecRightLocal, nOffsets[rank+1] - nOffsets[rank], MPI_DOUBLE, q, TAG_ISSEND, args->comm, sendReqs + q); MPI_CHK(err);
  }

  for (int nProcessed = 0; nProcessed < size;) {
    int nReady;
    err = MPI_Waitsome(size, recvReqs, &nReady, indices, MPI_STATUSES_IGNORE); MPI_CHK(err);
    if (args->verbosity > 1) {MPI_LOG(MPI_COMM_SELF, "Processing %d requests\n", nReady);}
    for (int i = 0; i < nReady; i++) {
      int q = indices[i];

      if (args->verbosity > 1) {MPI_LOG(MPI_COMM_SELF, "Processing recv from %d\n", q);}
      for (int j = 0; j < mLocal; j++) {
        double val = 0.;
        for (int k = nOffsets[q]; k < nOffsets[q+1]; k++) {
          val += matrixEntries[j * nGlobal + k] * recvBuf[k];
        }
        vecLeftLocal[j] += val;
      }
    }
    nProcessed += nReady;
  }
  if (args->verbosity > 1) {MPI_LOG(MPI_COMM_SELF, "Waiting for sends to finish\n");}
  err = MPI_Waitall(size, sendReqs, MPI_STATUSES_IGNORE); MPI_CHK(err);
  free(indices);
  free(sendReqs);
  free(recvReqs);
  free(recvBuf);
  return 0;
}

static int DenseMatVec_RowPartition_Allgatherv(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  double      *vecRight;
  int         *nLocals;
  int         *nOffsets;
  int         err, rank, size, nGlobal = nEnd - nStart;
  int         nLocal = rEnd - rStart, mLocal = lEnd - lStart;

  err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(args->comm, &rank); MPI_CHK(err);

  /* Set up a buffer to receive the other local portions of the right vector */
  vecRight = (double *) malloc(nGlobal * sizeof(*vecRight));
  if (!vecRight) MPI_CHK(1);
  /* Get the counts for every process */
  nLocals = (int *) malloc(size * sizeof(*nOffsets));
  if (!nLocals) MPI_CHK(1);
  /* Get the offsets for every process */
  nOffsets = (int *) malloc((size + 1) * sizeof(*nOffsets));
  if (!nOffsets) MPI_CHK(1);

  /* Gather the counts for every process */
  err = MPI_Allgather(&nLocal, 1, MPI_INT, nLocals, 1, MPI_INT, args->comm); MPI_CHK(err);
  /* Turn the counts into the offsets */
  nOffsets[0] = 0;
  for (int q = 0; q < size; q++) {
    nOffsets[q + 1] = nOffsets[q] + nLocals[q];
  }
  /* Gather the whole vector on each process */
  err = MPI_Allgatherv(vecRightLocal, nLocal, MPI_DOUBLE, vecRight, nLocals, nOffsets, MPI_DOUBLE, args->comm); MPI_CHK(err);

  for (int j = 0; j < mLocal; j++) {
    double val = 0.;
    for (int k = 0; k < nGlobal; k++) {
      val += matrixEntries[j * nGlobal + k] * vecRight[k];
    }
    vecLeftLocal[j] += val;
  }
  free(nOffsets);
  free(nLocals);
  free(vecRight);
  return 0;
}

int DenseMatVec_RowPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int rStart, int rEnd, const double *vecRightLocal, int lStart, int lEnd, double *vecLeftLocal)
{
  int strat, err;

  err = ArgsGetMatVecType(args, &strat); MPI_CHK(err);
  switch (strat) {
  case MATVEC_SSEND:
    if (args->verbosity) {MPI_LOG(args->comm, "DenseMatVec, Row Partition, Ssend\n");}
    err = DenseMatVec_RowPartition_Ssend(args, mStart, mEnd, nStart, nEnd, matrixEntries, rStart, rEnd, vecRightLocal, lStart, lEnd, vecLeftLocal); MPI_CHK(err);
    break;
  case MATVEC_ISSEND:
    if (args->verbosity) {MPI_LOG(args->comm, "DenseMatVec, Row Partition, Issend\n");}
    err = DenseMatVec_RowPartition_Issend(args, mStart, mEnd, nStart, nEnd, matrixEntries, rStart, rEnd, vecRightLocal, lStart, lEnd, vecLeftLocal); MPI_CHK(err);
    break;
  case MATVEC_ALLGATHERV:
    if (args->verbosity) {MPI_LOG(args->comm, "DenseMatVec, Row Partition, Allgatherv\n");}
    err = DenseMatVec_RowPartition_Allgatherv(args, mStart, mEnd, nStart, nEnd, matrixEntries, rStart, rEnd, vecRightLocal, lStart, lEnd, vecLeftLocal); MPI_CHK(err);
    break;
  default:
    MPI_LOG(args->comm, "Unrecognized matvec type %d\n", strat); MPI_CHK(err);
  }
  return 0;
}
