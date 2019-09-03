/* Nesting unnamed critical regions doesn't work */
#include <stdio.h>
#include <stdlib.h>

void non_safe_one (void)
{
  int random = rand();

  printf("This function does something not thread safe, like calculating %d from rand.\n",random);
}

void non_safe_two (void)
{
  int random = rand();

  #pragma omp critical
  {
    non_safe_one();
    printf("This function calculates another random number %d\n",random);
  }
}

int main(void)
{
  #pragma omp parallel
  {
    #pragma omp critical
    {
      non_safe_two();
    }
  }

  return 0;
}
