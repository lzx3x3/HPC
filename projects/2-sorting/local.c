#include <stdlib.h>
#include "proj2sorter.h"
#include "proj2sorter_impl.h"
#define SORT_NAME proj2_swenson
#define SORT_TYPE uint64_t
#include "swensonsort/sort.h"

int Proj2SorterSortLocal_merge(Proj2Sorter sorter, size_t numKeysIn, uint64_t *keysIn, 
                               size_t numKeysIn2, uint64_t *keysOut, int direction, int depth)
{
  int cnt, left = 0, right = 0;
  for(cnt=0; cnt < numKeysIn + numKeysIn2; cnt++)
  {
    if (left == numKeysIn)
    {
      keysOut[cnt] = keysOut[right + numKeysIn];
      right++;
    }
    else if (right == numKeysIn2)
    {
      keysOut[cnt] = keysIn[left];
      left++;
    }
    else if ( ! ((keysIn[left] > keysOut[right + numKeysIn]) ^ direction ))//XNOR
    {
      keysOut[cnt] = keysIn[left];
      left++;
    }
    else
    {
      keysOut[cnt] = keysOut[right + numKeysIn];
      right++;
    }
  }

  return 0;
}

int Proj2SorterSortLocal_swenson_quick_sort(Proj2Sorter sorter, size_t numKeysLocal, uint64_t *keys, int direction)
{
  proj2_swenson_quick_sort(keys, numKeysLocal);
  if (direction == PROJ2SORT_BACKWARD) {
    for (int i = 0; i < numKeysLocal / 2; i++) {
      uint64_t swap = keys[i];
      keys[i] = keys[numKeysLocal - 1 - i];
      keys[numKeysLocal - 1 - i] = swap;
    }
  }
  return 0;
}
int Proj2SorterSortLocal_qsort(Proj2Sorter sorter, size_t numKeysLocal, uint64_t *keys, int direction)
{
  if (direction == PROJ2SORT_FORWARD) {
    qsort(keys, numKeysLocal, sizeof(*keys), uint64_comp_forward);
  } else {
    qsort(keys, numKeysLocal, sizeof(*keys), uint64_comp_backward);
  }
  return 0;
}
int Proj2SorterSortLocal(Proj2Sorter sorter, size_t numKeysLocal, uint64_t *keys, int direction)
{
  int err;
#if 0
  err = Proj2SorterSortLocal_qsort(sorter, numKeysLocal, keys, direction); PROJ2CHK(err);
#else
  err = Proj2SorterSortLocal_swenson_quick_sort(sorter, numKeysLocal, keys, direction); PROJ2CHK(err);
#endif
  return 0;
}