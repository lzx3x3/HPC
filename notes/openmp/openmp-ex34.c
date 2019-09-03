/* ... and task parallelism */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(void)
{
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    #pragma omp single
    {
      int i, count;

      printf("I am %d and I am the dispatcher\n",id);
      count = 0;
      for (i = 0; i < 20; i++) {
        if (rand() & 1) {
          #pragma omp task
          {
            printf("Iteration %d spawned task %d, picked up by %d\n",i,count,omp_get_thread_num());
          }
          count++;
        }
      }
    }
  }

  return 0;
}
