#include <stdio.h>
#include <omp.h>

int main(void)
{
  int num_threads, my_thread;

  /* OpenMP implements "fork-join", where one master thread runs outside of
   * the parallel regions, forks to create them, and joins them at the end of
   * the region.  Let's see if we can confirm this. */

  /* Count the number of threads and my thread number before ... */
  num_threads = omp_get_num_threads();
  my_thread   = omp_get_thread_num();
  printf ("\"You're all individuals!\" said %d of %d.\n", my_thread, num_threads);

#pragma omp parallel
  {
    /* during ... */
    num_threads = omp_get_num_threads();
    my_thread   = omp_get_thread_num();
    printf("\"Yes, we're all individuals!\" replied %d of %d.\n", my_thread, num_threads);
  }

  /* and after the parallel region */
  num_threads = omp_get_num_threads();
  my_thread   = omp_get_thread_num();
  printf ("\"I'm not,\" said %d of %d.\n", my_thread, num_threads);

  return 0;
}
