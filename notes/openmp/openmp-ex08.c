#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main(void)
{
  int num_threads, my_thread;

  num_threads = omp_get_num_threads();
  my_thread   = omp_get_thread_num();
  printf ("\"You're all individuals!\" said %d of %d.\n", my_thread, num_threads);

#pragma omp parallel private(num_threads,my_thread)
  {
    /* what if we don't given an initialization for the private variables:
     * will they take on the value of the original shadowed variable?
    num_threads = omp_get_num_threads();
    my_thread   = omp_get_thread_num();
    sleep(1);
    */
    printf("\"Yes, we're all individuals!\" replied %d of %d, sleepily.\n", my_thread, num_threads);
  }

  printf ("\"I'm not,\" said %d of %d.\n", my_thread, num_threads);

  return 0;
}
