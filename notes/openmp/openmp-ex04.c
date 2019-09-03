#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main(void)
{
  int num_threads, my_thread;

  num_threads = omp_get_num_threads();
  my_thread   = omp_get_thread_num();
  printf ("\"You're all individuals!\" said %d of %d.\n", my_thread, num_threads);

#pragma omp parallel
  {
    num_threads = omp_get_num_threads();
    my_thread   = omp_get_thread_num();
    /* what if the parallel region takes a little longer? */
    sleep(1);
    printf("\"Yes, we're all individuals!\" replied %d of %d, sleepily.\n", my_thread, num_threads);
  }

  num_threads = omp_get_num_threads();
  my_thread   = omp_get_thread_num();
  printf ("\"I'm not,\" said %d of %d.\n", my_thread, num_threads);

  return 0;
}
