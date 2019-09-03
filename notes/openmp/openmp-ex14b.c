#include <stdio.h>
#include <omp.h>

int main(void)
{
  int N = 10;
  int i;

  /* It is easier to see the way that schedulers work if we are able to print
   * out the iterations in order.  To do that, we need to add an `ordered`
   * clause to the `for` directive, followed by an `ordered` directive
   * specifying the region that should be executed in order.
   */
#pragma omp parallel for schedule(runtime) ordered
  for (i = 0; i < N; i++) {
    int my_thread = omp_get_thread_num();

#pragma omp ordered
    {
      printf("iteration %d, thread %d\n", i, my_thread);
    }
  }

  return 0;
}
