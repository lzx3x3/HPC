#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;
  int i;

/* The "fork" in fork-join incurs some overhead for spawning the threads */
#pragma omp parallel for
  for (i = 0; i < N; i++) {
    int my_thread = omp_get_thread_num();

    printf("iteration %d, thread %d\n", i, my_thread);
  }

  printf("\n");

/* but modern implementations of OpenMP keep threads in a pool after a join,
 * so that the overhead of subsequent parallel regions is smaller */
#pragma omp parallel for
  for (i = 0; i < N; i++) {
    int my_thread = omp_get_thread_num();

    printf("iteration %d, thread %d\n", i, my_thread);
  }

  return 0;
}
