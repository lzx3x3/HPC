#if !defined(PROJ2SORT_H)
#define      PROJ2SORT_H

#include "proj2.h"
#include <mpi.h>

enum {
  PROJ2SORT_FORWARD  = 0,
  PROJ2SORT_BACKWARD = 1
};

enum {
  PROJ2TAG_BITONIC,
  PROJ2TAG_QUICKSORT
};

typedef struct _proj2sorter *Proj2Sorter;

static inline int uint64_comp_forward(const void *a, const void *b)
{
  uint64_t A = *((const uint64_t *)a);
  uint64_t B = *((const uint64_t *)b);

  return (A < B) ? -1 : (A == B) ? 0 : 1;
}

static inline int uint64_comp_backward(const void *a, const void *b)
{
  uint64_t A = *((const uint64_t *)a);
  uint64_t B = *((const uint64_t *)b);

  return (A < B) ? 1 : (A == B) ? 0 : -1;
}

/* Creation and destruction (define these in proj2sorter.c) */
int Proj2SorterCreate(MPI_Comm comm, Proj2Sorter *sorter);
int Proj2SorterDestroy(Proj2Sorter *sorter);

/* This is the default implementation of sorting:
 * \param[in] sorter       The sorting context.  Put all of your customizations
 *                         in this object.  Defined in proj2sorter_impl.h, where
 *                         you can change the struct to include more data
 * \param[in] numKeysLocal The number of keys on this process.
 * \param[in] uniform      The number of keys per process is uniform
 * \param[in/out] keys     The input array.  On output, should be globally
 *                         sorted in ascending order.
 * \return                 Non-zero if an error occured.
 */
int Proj2SorterSort(Proj2Sorter sorter, size_t numKeysLocal, int uniform, uint64_t *keys);

/* Defined in local.c */
/* This is the default local implementation of sorting:
 * \param[in] sorter       The sorting context.
 * \param[in] numKeysLocal The number of keys on this process.
 * \param[in/out] keys     The input array.  On output, should be *locally*
 *                         sorted in the order specified by \a direction.
 * \param[in] direction    Either PROJ2SORT_FORWARD for ascending or
 *                         PROJ2SORT_BACKWARD for descending.
 * \return                 Non-zero if an error occured.
 */
int Proj2SorterSortLocal(Proj2Sorter sorter, size_t numKeysLocal, uint64_t *keys, int direction);

/* Defined in bitonic.c */
/* A parallel sort that works for power of 2 processes.
 * \param[in] sorter       The sorting context.
 * \param[in] numKeysLocal The number of keys on this process.
 * \param[in/out] keys     The input array.  On output, should be *locally*
 *                         sorted in the order specified by \a direction.
 * \param[in] direction    Either PROJ2SORT_FORWARD for ascending or
 *                         PROJ2SORT_BACKWARD for descending.
 * \return                 Non-zero if an error occured.
 */
int Proj2SorterSortBitonic(Proj2Sorter sorter, size_t numKeysLocal, int uniform, uint64_t *keys);

int Proj2SorterGetWorkArray(Proj2Sorter sorter, size_t num, size_t size, void *workArray);
int Proj2SorterRestoreWorkArray(Proj2Sorter sorter, size_t num, size_t size, void *workArray);

#endif
