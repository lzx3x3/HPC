#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;

#pragma omp parallel
  {
    int my_thread = omp_get_thread_num();
    int i;

    /* But openmp has a directive "for" for for loops */
#pragma omp for
    for (i = 0; i < N; i++) {
      printf("iteration %d, thread %d\n", i, my_thread);
    }
  }

  return 0;
}
