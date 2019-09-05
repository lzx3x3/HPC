#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;
  int i;

  /* Thus far the first thread has always received the start of the loop, but
   * we can control this with the schedule() clause: schedule(runtime) means
   * we can control the schedule with the OMP_SCHEDULE environment variable.
   * */
#pragma omp parallel for schedule(runtime)
  for (i = 0; i < N; i++) {
    int my_thread = omp_get_thread_num();

    printf("iteration %d, thread %d\n", i, my_thread);
  }

  return 0;
}
