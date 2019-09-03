#include <stdio.h>
/* OpenMP includes some library calls, for which we need the header file */
#include <omp.h>

int main(void)
{
  int max_threads = 5;

  printf ("You're all individuals!\n");

  /* one library call sets the number of threads in the next parallel region */
  omp_set_num_threads(max_threads);
#pragma omp parallel
  {
    printf("Yes, we're all individuals!\n");
  }

  return 0;
}
