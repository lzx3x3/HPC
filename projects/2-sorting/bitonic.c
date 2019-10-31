
#include <mpi.h>
#include <stdint.h>
#include <math.h>
#include "proj2sorter.h"
#include "proj2sorter_impl.h"

static int uint64_swap_bitonic(Proj2Sorter sorter, size_t nkeys, uint64_t * keys, int level, int rank, int direction)
{
  uint64_t    *recv;
  MPI_Request recv_req;
  int         comm_rank = rank ^ (1 << level);
  int         err;

  err = Proj2SorterGetWorkArray(sorter, nkeys, sizeof(uint64_t), &recv); PROJ2CHK(err);
  err = MPI_Irecv(recv, nkeys, MPI_UINT64_T, comm_rank, PROJ2TAG_BITONIC, sorter->comm, &recv_req); MPI_CHK(err);
  err = MPI_Ssend(keys, nkeys, MPI_UINT64_T, comm_rank, PROJ2TAG_BITONIC, sorter->comm); MPI_CHK(err);
  err = MPI_Wait(&recv_req, MPI_STATUS_IGNORE); MPI_CHK(err);
  if ((rank < comm_rank) ^ direction) { /* take the lower half */
    for (size_t i = 0; i < nkeys; i++) {
      keys[i] = recv[i] < keys[i] ? recv[i] : keys[i];
    }
  } else {                              /* take the upper half */
    for (size_t i = 0; i < nkeys; i++) {
      keys[i] = recv[i] > keys[i] ? recv[i] : keys[i];
    }
  }
  err = Proj2SorterRestoreWorkArray(sorter, nkeys, sizeof(uint64_t), &recv); PROJ2CHK(err);
  return 0;
}

static int uint64_sort_bitonic(Proj2Sorter sorter, size_t nkeys, uint64_t *keys, int level, int rank, int direction, int input_bitonic)
{
  int err;

  if (level == 0) {
    if (!input_bitonic) {
      err = Proj2SorterSortLocal(sorter, nkeys, keys, direction); PROJ2CHK(err);
    } else if (nkeys > 1) {
      size_t limit = nkeys / 2;
      size_t shift = nkeys - limit; /* this will be the same as limit if nkeys is even, one more if nkeys is odd */
      for (size_t i = 0; i < limit; i++) {
        uint64_t lo = keys[i];
        uint64_t hi = keys[i + shift];
        uint64_t minmax[2];

        minmax[0 ^ direction] = lo < hi ? lo : hi;
        minmax[1 ^ direction] = lo < hi ? hi : lo;

        keys[i] = minmax[0];
        keys[i + shift] = minmax[1];
      }
      if (shift > limit) {
        uint64_t midkey = keys[limit];
        /* Odd number of keys.  The two halves (keys[0..limit-1] and
         * keys[shift..nkeys-1] are both bitonic.  The middle key
         * (keys[limit]) can belong to either the first half or the second
         * half while staying bitonic. */

        /* The invariant of bitonic swap is that we have split the radices
         * of keys[0..limit-1] and keys[shift..nkeys - 1], so that if
         * keys[limit] is within the radix range of one of them, it may be
         * concatenated to that sequence and still be bitonic */

        /* our default assumption will be that midkey belongs to the
         * second bitonic sequence, which we will change only if midkey is in
         * the middle of the first range, or if midkey is an extremum */
        if ((keys[0] <= midkey && midkey <= keys[limit - 1]) ||
            (keys[0] >= midkey && midkey >= keys[limit - 1]) ||
            (midkey < keys[0] && (direction == PROJ2SORT_FORWARD)) ||
            (midkey > keys[0] && (direction == PROJ2SORT_BACKWARD))) {
          /* midkey is a part of the first bitonic sequence */
          limit++;
          shift--;
        }
      }
      err = uint64_sort_bitonic(sorter, limit, keys, 0, rank, direction, 1); PROJ2CHK(err);
      err = uint64_sort_bitonic(sorter, shift, &keys[limit], 0, rank, direction, 1); PROJ2CHK(err);
    }
    return 0;
  }
  if (!input_bitonic) {
    err = uint64_sort_bitonic(sorter, nkeys, keys, level - 1, rank, direction ^ ((rank >> (level - 1)) & 1), input_bitonic); PROJ2CHK(err);
  }
  err = uint64_swap_bitonic(sorter, nkeys, keys, level - 1, rank, direction); PROJ2CHK(err);
  err = uint64_sort_bitonic(sorter, nkeys, keys, level - 1, rank, direction, 1); PROJ2CHK(err);

  return 0;
}

int Proj2SorterSortBitonic(Proj2Sorter sorter, size_t nkeys, int uniform, uint64_t *keys)
{
  MPI_Comm comm = sorter->comm;
  int      size, rank;
  int      log2Size;
  int      err;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);

  log2Size = (int) log2((double) size);
  /* Bitonic sort is designed for power of 2 number of processes */
  if (size != (1 << log2Size)) {PROJ2ERR(comm,1,"Cannot use bitonic sort on %d processes\n",size);}
  if (!uniform) {PROJ2ERR(comm,1,"Cannot use bitonic sort on non-uniform distributions\n");}
  err = uint64_sort_bitonic(sorter, nkeys, keys, log2Size, rank, PROJ2SORT_FORWARD, 0); PROJ2CHK(err);

  return 0;
}
