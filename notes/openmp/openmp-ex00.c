/* Hello threads: adapted from Edmond Chow's OpenMP notes */
#include <stdio.h>

int main(void)
{
  printf ("You're all individuals!\n");
  /* create a team of threads for the following structured block */
#pragma omp parallel
  {
    printf("Yes, we're all individuals!\n");
  }
  /* team of threads join master thread after the structured block */

  return 0;
}
