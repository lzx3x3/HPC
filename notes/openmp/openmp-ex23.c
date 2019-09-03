#include <stdio.h>
#include <unistd.h>

int main(void)
{
  #pragma omp parallel
  {
    int i;
    char wait[BUFSIZ] = {'\0'};
    /* Unless we tell threads not to wait when they're done */
    #pragma omp for nowait
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
  printf("Not again, cut!\n");
  return 0;
}
