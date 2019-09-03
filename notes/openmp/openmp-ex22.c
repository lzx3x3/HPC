#include <stdio.h>
#include <unistd.h>

int main(void)
{
  #pragma omp parallel
  {
    int i;
    char wait[BUFSIZ] = {'\0'};
    /* Again, there is implicit synchronization at the end of a for directive */
    #pragma omp for
    for (i = 0; i < 4; i++) {
      int j;
      for (j = 0; j < 4 * i; j++) wait[j] = ' ';
      sleep(i);
      printf("%srow row row your boat...\n",wait);
    }
    #pragma omp for
    for (i = 0; i < 4; i++) {
      sleep(i);
      printf("%s...gently down the stream...\n",wait);
    }
  }
  printf("Better.\n");
  return 0;
}
