#include "dmv.h"
#include "dmv_impl.h"

int VecGetOffset_gather(MPI_Comm comm, int nLocal, int *nOffset)
{
  int size, rank, err;
  int *buffer = NULL;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);

  /* one disadvantage of this strategy is that O(size) storage is needed on one
   * process */
  if (!rank) {
    buffer = (int *) malloc(size * sizeof(*buffer));
    if (!buffer) MPI_CHK(1);
  }
  err = MPI_Gather(&nLocal, 1, MPI_INT, buffer, 1, MPI_INT, 0, comm); MPI_CHK(err);

  /* one advantage of this strategy is that the computation of the offsets
   * from the counts is a fast stream over an array, although it is O(size)
   * work */
  if (!rank) {
    int offset = 0;
    for (int i = 0; i < size; i++) {
      int count = buffer[i];

      buffer[i] = offset;
      offset += count;
    }
  }

  /* another disadvantage of this strategy is that there are two rounds of
   * communication */
  err = MPI_Scatter(buffer, 1, MPI_INT, nOffset, 1, MPI_INT, 0, comm); MPI_CHK(err);

  if (!rank) {
    free(buffer);
  }
  return 0;
}

int VecGetOffset_allgather(MPI_Comm comm, int nLocal, int *nOffset)
{
  int size, rank, err;
  int *buffer = NULL;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);

  /* one disadvantage of this strategy is that O(size) storage is needed on each
   * process */
  buffer = (int *) malloc(size * sizeof(*buffer));
  if (!buffer) MPI_CHK(1);

  err = MPI_Allgather(&nLocal, 1, MPI_INT, buffer, 1, MPI_INT, comm); MPI_CHK(err);

  /* one advantage of this strategy is that the computation of the offsets
   * from the counts is a fast stream over an array, although it is O(size)
   * work */
  {
    int offset = 0;
    for (int i = 0; i <= rank; i++) {
      int count = buffer[i];

      buffer[i] = offset;
      offset += count;
    }
  }

  /* one advantage of this strategy is that there is only one round of
   * communication */

  *nOffset = buffer[rank];

  free(buffer);
  return 0;
}

int VecGetOffset(Args args, int nLocal, int *nOffset)
{
  int err;

  switch (args->layout_strategy) {
  case LAYOUT_GATHER:
    if (args->verbosity) MPI_LOG(args->comm, "Offset strategy gather / scatter\n");
    err = VecGetOffset_gather(args->comm, nLocal, nOffset); MPI_CHK(err);
    break;
  case LAYOUT_ALLGATHER:
    if (args->verbosity) MPI_LOG(args->comm, "Offset strategy allgather\n");
    err = VecGetOffset_allgather(args->comm, nLocal, nOffset); MPI_CHK(err);
    break;
  case LAYOUT_SCAN:
    if (args->verbosity) MPI_LOG(args->comm, "Offset strategy exscan\n");
    /* we have nOffset to zero on process 0, but we go ahead and do it on each
     * process */
    *nOffset = 0;
    err = MPI_Exscan(&nLocal, nOffset, 1, MPI_INT, MPI_SUM, args->comm); MPI_CHK(err);
    break;
  default:
    MPI_LOG(args->comm, "Unrecognized layout strategy %d\n", args->global_size_strategy);
    MPI_CHK(1);
  }
  if (args->verbosity > 1) {
    int size, rank;

    err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);
    err = MPI_Comm_rank(args->comm, &rank); MPI_CHK(err);
    for (int i = 0; i < size; i++) {
      if (i == rank) {
        MPI_LOG(MPI_COMM_SELF, "Offset: %d\n", *nOffset);
      }
      err = MPI_Barrier(args->comm); MPI_CHK(err);
    }
  }
  return 0;
}
