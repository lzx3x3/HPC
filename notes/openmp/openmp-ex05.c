#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main(void)
{
  int orig_num_threads, orig_my_thread;

  orig_num_threads = omp_get_num_threads();
  orig_my_thread   = omp_get_thread_num();
  printf ("\"You're all individuals!\" said %d of %d.\n", orig_my_thread, orig_num_threads);

#pragma omp parallel
  {
    /* The last example showed that variables are shared by default in
     * parallel regions: having multiple threads write to the same variable
     * creates a race condition.
     *
     * But, variables declared inside the scope of the parallel region are
     * private: each thread has its own private variables */
    int my_thread, num_threads;

    num_threads = omp_get_num_threads();
    my_thread   = omp_get_thread_num();
    sleep(1);
    printf("\"Yes, we're all individuals!\" replied %d of %d, sleepily.\n", my_thread, num_threads);
  }

  orig_num_threads = omp_get_num_threads();
  orig_my_thread   = omp_get_thread_num();
  printf ("\"I'm not,\" said %d of %d.\n", orig_my_thread, orig_num_threads);

  return 0;
}
