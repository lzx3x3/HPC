#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;
  int i;

  /* We can even concatenate directives for a very succinct syntax */
#pragma omp parallel for
  for (i = 0; i < N; i++) {
    /* `i` was declared out of scope, so you my worry that `i` is the same
     * variable for all threads and there is a race condition, but
     * *loop indices are automatically privatized* */
    int my_thread = omp_get_thread_num();

    printf("iteration %d, thread %d\n", i, my_thread);
  }

  return 0;
}
