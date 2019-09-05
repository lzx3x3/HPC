#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main(void)
{
  #pragma omp parallel
  {
    int i;
    #pragma omp parallel for
    for (i = 0; i < 4; i++) {
      int  j;
      char wait[BUFSIZ] = {'\0'};

      for (j = 0; j < 4 * i; j++) wait[j] = ' ';
      sleep(j);
      printf("%srow row row your boat...\n",wait);
      /* This won't even compile */
      #pragma omp barrier
      sleep(j);
      printf("%s...gently down the stream...\n",wait);
    }
  }
  printf("The waiting is the hardest part...\n");
  return 0;
}
