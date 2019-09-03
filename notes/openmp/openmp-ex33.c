/* Although OpenMP emphasizes data parallelism, there are also constructs for
 * instruction parallelism */
#include <stdio.h>
#include <omp.h>

int main(void)
{
  int clients[2] = {-1, -1};
  #pragma omp parallel
  {
    int id = omp_get_thread_num();
    #pragma omp sections
    {
      #pragma omp section
      {
        int found[2] = {0, 0};

        printf ("I am %d and I am the server.\n",id);

        while (1) {
          int i;

          for (i = 0; i < 2; i++) {
            if (!found[i] && clients[i] >= 0) {
              found[i] = 1;
              printf("Thread %d has checked in as client %d\n",clients[i],i);
            }
          }
          if (found[0] && found[1]) break;
        }

      }
      #pragma omp section
      {
        printf("I am %d and I am client 0.\n",id);
        clients[0] = id;
      }
      #pragma omp section
      {
        printf("I am %d and I am client 1.\n",id);
        clients[1] = id;
      }
    }
  }

  return 0;
}
