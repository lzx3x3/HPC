#include <stdio.h>
#if defined(_OPENMP)
#include <omp.h>
#endif

int main(void)
{
  #pragma omp parallel
  {
    int thread = 0;
    int nthreads = 1;
    int place = 0;
    int nplaces = 1;
    int i;

#if defined(_OPENMP)
    thread = omp_get_thread_num();
    nthreads = omp_get_num_threads();
    place = omp_get_place_num();
    nplaces = omp_get_num_places();
#endif

    #pragma omp for ordered
    for (i = 0; i < nplaces; i++) {
      int procs[512] = {0};
      int nprocs = 1;

#if defined(_OPENMP)
      nprocs = omp_get_place_num_procs(i);
      omp_get_place_proc_ids(i,procs);
#endif

      #pragma omp ordered
      {
        int j;

        printf("Place %d has procs:",i);
        for (j = 0; j < nprocs; j++) {
          printf(" %d",procs[j]);
        }
        printf("\n");
        fflush(stdout);
      }
    }
    #pragma omp for ordered
    for (i = 0; i < nthreads; i++) {
      #pragma omp ordered
      {
        printf("Thread %d is on one of the processes in place %d\n",thread,place);
        fflush(stdout);
      }
    }
  }


  return 0;
}
