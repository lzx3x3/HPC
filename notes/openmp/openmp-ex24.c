#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main(void)
{
  int num_threads;
  int fib = 1;
  int fib_prev = 0;
  #pragma omp parallel
  {
    /* calculating the fibonacci number of the number of threads is trivial,
     * but sometimes there is work that must be serial (typically I/O) that is inconvenient to
     * have outside of a parallel block */
    int  i;

    num_threads = omp_get_num_threads();
    for (i = 2; i < num_threads; i++) {
      int fib_next = fib + fib_prev;
      fib_prev = fib;
      fib = fib_next;
    }
    printf("fib(num_threads = %d) = %d\n",num_threads,fib);
  }
  return 0;
}
