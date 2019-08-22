
#include <stdlib.h>
#include <stdio.h>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "fma_omp.h"
#include "fma_host.h"

int
fma_host_initialize (int N, int T, float **a)
{
  if (!N) {
    *a = NULL;
  }
  else {
    *a = (float *) malloc (N * sizeof (float));
    if (!*a) {
      printf ("Failed to allocate a\n");
      return 1;
    }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
      (*a)[i] = (float) i;
    }
  }
  return 0;
}

int
fma_host_free (int N, int T, float **a)
{
  free (*a);
  *a = NULL;
  return 0;
}

int
fma_host_start (int N, int T, float *a, float b, float c)
{
  #pragma omp parallel
  {
    int num_threads, my_thread;
    int my_start, my_end;
    int my_N;

#if defined(_OPENMP)
    my_thread = omp_get_thread_num();
    num_threads = omp_get_num_threads();
#else
    my_thread = 0;
    num_threads = 1;
#endif

    /* get thread intervals */
    my_start = ((size_t) my_thread * (size_t) N) / (size_t) num_threads;
    my_end   = ((size_t) (my_thread + 1) * (size_t) N) / (size_t) num_threads;
    my_N     = my_end - my_start;
#if 0
    printf ("[%d/%d]: [%d, %d)\n", my_thread, num_threads, my_start, my_end);
#endif

    /* execute the loop */
    fma_loop_host (my_N, T, &a[my_start], b, c);
  }
  return 0;
}

int
fma_host_end (int N, int T, float *a, float b, float c)
{
  return 0;
}
