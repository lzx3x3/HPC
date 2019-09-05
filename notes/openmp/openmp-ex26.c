#include <stdio.h>

int main(void)
{
  int tickets_out = 0;

  #pragma omp parallel
  {
    int my_ticket;

    /* we should know from studying the data model before that having all of
     * the threads try to increment tickets_out at the same time won't work,
     * but especially for task based parallelism, synchronization of this sort
     * is important */
    my_ticket = tickets_out++;
    printf("My ticket is %d\n",my_ticket);
  }

  return 0;
}
