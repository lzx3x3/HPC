/* There was a debate in class about what `guided` scheduling means in
 * assigning loop interations to threads in OpenMP. 
 *
 * Supposing we have $N$ workers, $o$ of them are currently working, and $M$
 * unassigned loop iterations.  Is the next assignment size $M / N$, or $M /
 * (N - o)$?
 *
 * I'm trying to write this as a minimal working example to test the two
 * hypotheses.
 */

#include <stdio.h>
#include <omp.h>

int main(void)
{
  int i;

#pragma omp parallel num_threads(2)
  {
    int threadid = omp_get_thread_num();

#pragma omp for schedule(guided) ordered
    for (i = 0; i < 16; i++) {
#pragma omp ordered
      printf("iteration %d, thread %d\n", i, threadid);
    }
  }

  return 0;
}
