#if !defined(CLOUD_UTIL_H)
#define      CLOUD_UTIL

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

int safe_malloc (size_t count, void *out, const char * file, const char * fn, int line);

#define safeMALLOC(count,out) safe_malloc (count, out, __FILE__, __func__, __LINE__)
#define CHK(Q) if (Q) return Q

#endif
