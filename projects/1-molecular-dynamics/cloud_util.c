
#include "cloud_util.h"

int safe_malloc (size_t count, void *out, const char * file, const char * fn, int line)
{
  if (!count) {
    *((void **) out) = NULL;
    return 0;
  }
  *((void **) out) = malloc (count);
  if (!(*((void **) out))) {
    fprintf (stderr, "%s, %s (%d): failed to malloc %zu bytes\n", file, fn, line, count);
    return 1;
  }
  return 0;
}

