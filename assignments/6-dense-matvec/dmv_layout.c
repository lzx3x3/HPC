#include "dmv.h"
#include "dmv_impl.h"

static int VecGetLayout_gather(MPI_Comm comm, int mLocal, int *localOffsets)
{
  int size, rank, err;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  localOffsets[0] = 0;
  err = MPI_Gather(&mLocal, 1, MPI_INT, &localOffsets[1], 1, MPI_INT, 0, comm); MPI_CHK(err);
  if (!rank) {
    for (int i = 1; i < size; i++) {
      localOffsets[i + 1] += localOffsets[i];
    }
  }
  err = MPI_Bcast(localOffsets, size + 1, MPI_INT, 0, comm); MPI_CHK(err);

  return 0;
}

static int VecGetLayout_allgather(MPI_Comm comm, int mLocal, int *localOffsets)
{
  int size, rank, err;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  localOffsets[0] = 0;
  err = MPI_Allgather(&mLocal, 1, MPI_INT, &localOffsets[1], 1, MPI_INT, comm); MPI_CHK(err);
  for (int i = 1; i < size; i++) {
    localOffsets[i + 1] += localOffsets[i];
  }

  return 0;
}

static int VecGetLayout_scan(MPI_Comm comm, int mLocal, int *localOffsets)
{
  int size, rank, offset, err;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  localOffsets[0] = 0;
  err = MPI_Scan(&mLocal, &offset, 1, MPI_INT, MPI_SUM, comm); MPI_CHK(err);
  err = MPI_Allgather(&offset, 1, MPI_INT, &localOffsets[1], 1, MPI_INT, comm); MPI_CHK(err);

  return 0;
}

int VecGetLayout(Args args, int mLocal, int *localOffsets)
{
  int size, rank;
  int err;

  switch (args->layout_strategy) {
  case LAYOUT_GATHER:
    if (args->verbosity) MPI_LOG(args->comm, "Layout strategy gather / broadcast\n");
    err = VecGetLayout_gather(args->comm, mLocal, localOffsets); MPI_CHK(err);
    break;
  case LAYOUT_ALLGATHER:
    if (args->verbosity) MPI_LOG(args->comm, "Layout strategy allgather\n");
    err = VecGetLayout_allgather(args->comm, mLocal, localOffsets); MPI_CHK(err);
    break;
  case LAYOUT_SCAN:
    if (args->verbosity) MPI_LOG(args->comm, "Layout strategy scan\n");
    err = VecGetLayout_scan(args->comm, mLocal, localOffsets); MPI_CHK(err);
    break;
  default:
    MPI_LOG(args->comm, "Unrecognized layout strategy %d\n", args->global_size_strategy);
    MPI_CHK(1);
  }
  err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(args->comm, &rank); MPI_CHK(err);
  if (args->verbosity > 1) {
    for (int i = 0; i <= size; i++) {
      MPI_LOG(args->comm, "[%d] %d\n", i, localOffsets[i]);
    }
  }
  if (args->debug) {
    int *localOffsetsTranspose;

    if (args->verbosity) {
      MPI_LOG(args->comm, "Checking offsets against alltoall\n");
    }
    if ((localOffsets[rank + 1] - localOffsets[rank]) != mLocal) {
      MPI_LOG(MPI_COMM_SELF, "Offsets failed: consecutive offsets not equal to local size\n");
      MPI_CHK(1);
    }
    localOffsetsTranspose = (int *) malloc(size * sizeof(*localOffsetsTranspose));
    if (!localOffsetsTranspose) MPI_CHK(1);
    err = MPI_Alltoall(localOffsets, 1, MPI_INT, localOffsetsTranspose, 1, MPI_INT, args->comm); MPI_CHK(err);
    for (int i = 0; i < size; i++) {
      if (localOffsetsTranspose[i] != localOffsets[rank]) {
        MPI_LOG(MPI_COMM_SELF, "Offsets failed: offsets for rank to not agree on all processes\n");
        MPI_CHK(1);
      }
    }
    free(localOffsetsTranspose);
  }
  return 0;
}
