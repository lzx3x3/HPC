#if !defined(PROJ2SORTER_IMPL_H)
#define      PROJ2SORTER_IMPL_H

#include "proj2sorter.h"

typedef struct _proj2memlink *Proj2MemLink;

struct _proj2memlink
{
  Proj2MemLink next;
  size_t size;
  char *array;
};

struct _proj2sorter
{
  MPI_Comm     comm;
  Proj2MemLink avail;
  Proj2MemLink inUse;
};

/* A parallel sort implementation based on divide and conquer, by choosing a
 * pivot and dividing into subproblems.  In parallel, each processor has a
 * communicating partner on the other half of the communicator.  Once the
 * pivot is known, each partner sends its entries less than the pivot to the
 * lower of the pair and the others to the greater of the pair. */
int Proj2SorterSort_quicksort(Proj2Sorter sorter, size_t numKeysLocal, int uniform, uint64_t *keys);

#endif
