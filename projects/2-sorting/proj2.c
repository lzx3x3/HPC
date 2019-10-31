#define _POSIX_C_SOURCE 200112L
#include <stdlib.h>
#include "proj2.h"

int Proj2Malloc(size_t size, void **array)
{
  int err;

  err = posix_memalign(array, 64, size); PROJ2CHK(err);

  return 0;
}

int Proj2Free(void **array)
{
  free(*array);
  *array = NULL;
  return 0;
}

