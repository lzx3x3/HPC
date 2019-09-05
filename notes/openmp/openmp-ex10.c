#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;

  /* We could to loop parallelization with just what we've seen so far */
#pragma omp parallel
  {
    int my_thread   = omp_get_thread_num();
    int num_threads = omp_get_num_threads();
    int istart      = (N * my_thread) / num_threads;
    int iend        = (N * (my_thread+1)) / num_threads;
    int i;

    for (i = istart; i < iend; i++) {
      printf("iteration %d, thread %d\n", i, my_thread);
    }
  }

  return 0;
}
