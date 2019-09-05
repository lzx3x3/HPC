#include <stdio.h>

int main(void)
{
  int tickets_out = 0;

  #pragma omp parallel
  {
    int my_ticket;

    /* Only one thread may enter a critical region at a time.
     * In fact, for most of these examples I have had threads writing to the
     * stdout stream without any kind of protection between them.  This has
     * worked only because of the low number of threads and luck.  In general,
     * only one thread should own a stream at a time to prevent garbled
     * messages */
    #pragma omp critical
    {
      my_ticket = tickets_out++;
      printf("My ticket is %d\n",my_ticket);
    }
  }

  return 0;
}
