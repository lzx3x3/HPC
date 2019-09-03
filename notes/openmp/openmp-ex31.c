/* Of course, rand_r() is thread safe */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void safe_one (unsigned int *seed)
{
  int random = rand_r(seed);

  printf("This function does something thread safe, like calculating %d from rand_r.\n",random);
}

void safe_two (unsigned int *seed)
{
  int random = rand_r(seed);

  safe_one(seed);
  printf("This function calculates another random number %d\n",random);
}

int main(void)
{
  #pragma omp parallel
  {
    unsigned int seed = omp_get_thread_num();

    safe_two(&seed);
  }

  return 0;
}
