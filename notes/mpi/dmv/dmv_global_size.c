#include "dmv.h"
#include "dmv_impl.h"

static int VecGetGlobalSize_tree_subcomm(MPI_Comm comm, int mLocal, int *mGlobal_p)
{
  int      size, rank, color, err, mGlobalSub, mGlobalOtherSub, equivRank;
  MPI_Comm subcomm;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);

  if (size == 1) {
    *mGlobal_p = mLocal;

    return 0;
  }

  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);

  /* Split into a subproblem on the first half and second half of ranks */
  color = (2 * rank) >= size;
  err = MPI_Comm_split(comm, color, rank, &subcomm); MPI_CHK(err);

  /* Get the global size for the subproblems */
  err = VecGetGlobalSize_tree_subcomm(subcomm, mLocal, &mGlobalSub); MPI_CHK(err);

  /* Everyone gets the size of the other half of the subproblem from his
   * equivalent rank in that subproblem */

  equivRank = (rank + (size / 2) + size) % size;
  /* if there are an uneven number of ranks, we need to do a little arithmetic
   * to make sure everyone has a partner */
  if (size % 2) equivRank += color;
  if ((size % 2) == 0 || (rank % (size / 2)) != 0) {
    /* Easy case: matched send and receive */
    err = MPI_Sendrecv(&mGlobalSub,      1, MPI_INT, equivRank, TAG_TREE_SUBCOMM,
                       &mGlobalOtherSub, 1, MPI_INT, equivRank, TAG_TREE_SUBCOMM,
                       comm, MPI_STATUS_IGNORE);
    MPI_CHK(err);
  } else {
    /* hard case: 0 and (size / 2) are on one side, they both communicate with
     * (size - 1) */
    if (rank == size - 1) {
      err = MPI_Recv (&mGlobalOtherSub, 1, MPI_INT, 0,        TAG_TREE_SUBCOMM, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
      err = MPI_Ssend(&mGlobalSub,      1, MPI_INT, 0,        TAG_TREE_SUBCOMM, comm); MPI_CHK(err);
      err = MPI_Ssend(&mGlobalSub,      1, MPI_INT, size / 2, TAG_TREE_SUBCOMM, comm); MPI_CHK(err);
    } else {
      if (!rank) {
        err = MPI_Ssend(&mGlobalSub,    1, MPI_INT, size - 1, TAG_TREE_SUBCOMM, comm); MPI_CHK(err);
      }
      err = MPI_Recv (&mGlobalOtherSub, 1, MPI_INT, size - 1, TAG_TREE_SUBCOMM, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
    }
  }
  *mGlobal_p = mGlobalSub + mGlobalOtherSub;
  err = MPI_Comm_free(&subcomm); MPI_CHK(err);
  return 0;
}

static int VecGetGlobalSize_tree_recursive_inner(MPI_Comm comm, int rank, int rStart, int rEnd, int mLocal, int *mGlobal_p)
{
  int size  = rEnd - rStart;
  int rrank = rank - rStart;
  int color = (2 * rrank >= size);
  int equivrRank;
  int rSplit;
  int mLocalOther;
  int err;

  if (size == 1) {
    *mGlobal_p = mLocal;

    return 0;
  }
  equivrRank = (rrank + (size / 2) + size) % size;
  if (size % 2) equivrRank += color;
  if ((size % 2) == 0 || (rrank % (size / 2)) != 0) {
    /* Easy case: matched send and receive */
    err = MPI_Sendrecv(&mLocal,      1, MPI_INT, equivrRank + rStart, TAG_TREE_RECURSE,
                       &mLocalOther, 1, MPI_INT, equivrRank + rStart, TAG_TREE_RECURSE,
                       comm, MPI_STATUS_IGNORE);
    MPI_CHK(err);
  } else {
    /* hard case: 0 and (size / 2) are on one side, they both communicate with
     * (size - 1) */
    if (rrank == size - 1) {
      err = MPI_Ssend (&mLocal,      1, MPI_INT, rStart,            TAG_TREE_RECURSE, comm); MPI_CHK(err);
      err = MPI_Recv  (&mLocalOther, 1, MPI_INT, rStart,            TAG_TREE_RECURSE, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
      mLocal += mLocalOther;
      err = MPI_Recv  (&mLocalOther, 1, MPI_INT, rStart + size / 2, TAG_TREE_RECURSE, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
    } else {
      if (!rrank) {
        err = MPI_Recv(&mLocalOther, 1, MPI_INT, rStart + size - 1, TAG_TREE_RECURSE, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
      } else {
        mLocalOther = 0;
      }
      err = MPI_Ssend (&mLocal, 1, MPI_INT, rStart + size - 1, TAG_TREE_SUBCOMM, comm); MPI_CHK(err);

    }
  }
  mLocal += mLocalOther;
  rSplit = rStart + (size / 2) + (size % 2);
  if (color) {rStart = rSplit;}
  else       {rEnd   = rSplit;}
  return VecGetGlobalSize_tree_recursive_inner(comm, rank, rStart, rEnd, mLocal, mGlobal_p);
}

static int VecGetGlobalSize_tree_recursive(MPI_Comm comm, int mLocal, int *mGlobal_p)
{
  int size, rank, err;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  return VecGetGlobalSize_tree_recursive_inner(comm, rank, 0, size, mLocal, mGlobal_p);
}

static int VecGetGlobalSize_reduce_broadcast(MPI_Comm comm, int mLocal, int *mGlobal_p)
{
  int err;

  *mGlobal_p = 0;
  err = MPI_Reduce(&mLocal, mGlobal_p, 1, MPI_INT, MPI_SUM, 0, comm); MPI_CHK(err);
  err = MPI_Bcast(mGlobal_p, 1, MPI_INT, 0, comm); MPI_CHK(err);

  return 0;
}

int VecGetGlobalSize(Args args, int mLocal, int *mGlobal_p)
{
  int size, rank, err;

  switch (args->global_size_strategy) {
  case GLOBAL_SIZE_TREE_SUBCOMM:
    if (args->verbosity) MPI_LOG(args->comm, "Global size strategy tree via subcommunicators\n");
    err = VecGetGlobalSize_tree_subcomm(args->comm, mLocal, mGlobal_p); MPI_CHK(err);
    break;
  case GLOBAL_SIZE_TREE_RECURSE:
    if (args->verbosity) MPI_LOG(args->comm, "Global size strategy tree via recursion\n");
    err = VecGetGlobalSize_tree_recursive(args->comm, mLocal, mGlobal_p); MPI_CHK(err);
    break;
  case GLOBAL_SIZE_REDUCE_BCAST:
    if (args->verbosity) MPI_LOG(args->comm, "Global size strategy reduce / broadcast\n");
    err = VecGetGlobalSize_reduce_broadcast(args->comm, mLocal, mGlobal_p); MPI_CHK(err);
    break;
  case GLOBAL_SIZE_ALLREDUCE:
    if (args->verbosity) MPI_LOG(args->comm, "Global size strategy allreduce\n");
    err = MPI_Allreduce(&mLocal, mGlobal_p, 1, MPI_INT, MPI_SUM, args->comm); MPI_CHK(err);
    break;
  default:
    MPI_LOG(args->comm, "Unrecognized global size strategy %d\n", args->global_size_strategy);
    MPI_CHK(1);
  }
  err = MPI_Comm_size(args->comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(args->comm, &rank); MPI_CHK(err);
  if (args->verbosity > 1) {
    for (int i = 0; i < size; i++) {
      if (i == rank) {
        MPI_LOG(MPI_COMM_SELF, "Global size %d\n", *mGlobal_p);
      }
      err = MPI_Barrier(args->comm); MPI_CHK(err);
    }
  }
  return 0;
}

