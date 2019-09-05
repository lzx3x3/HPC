/* Even named critical regions can't prevent deadlocks() */
#include <stdio.h>
#include <omp.h>

int main(void)
{
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    #pragma omp critical(A)
    {
      printf("I am thread %d and I am in A, waiting for B...",id);
      fflush(stdout);
      #pragma omp critical(B)
      {
        printf("got it!\n");
        fflush(stdout);
      }
    }
    #pragma omp critical(B)
    {
      printf("I am thread %d and I am in B, waiting for A...\n",id);
      fflush(stdout);
      #pragma omp critical(A)
      {
        printf("got it!\n");
        fflush(stdout);
      }
    }
  }

  return 0;
}
