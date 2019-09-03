#include <stdio.h>
#include <omp.h>

int main(void)
{
  int max_threads = 5;

  printf ("You're all individuals!\n");

  omp_set_num_threads(max_threads);
  /* now we have two competing values for the number of threads in this
   * region: who wins? */
#pragma omp parallel num_threads(7)
  {
    printf("Yes, we're all individuals!\n");
  }

  return 0;
}
