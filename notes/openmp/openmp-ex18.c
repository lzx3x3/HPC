#include <stdio.h>
#include <unistd.h>
#include <omp.h>

int main(void)
{
  /* There is an implicit barrier (due to the fork-join model) at the end of
   * all parallel regions*/
  #pragma omp parallel
  {
    int thread_num = omp_get_thread_num(), i;
    char wait[BUFSIZ] = {'\0'};
    for (i = 0; i < 4 * thread_num; i++) wait[i] = ' ';
    sleep(thread_num);
    printf("%srow row row your boat...\n",wait);
    sleep(thread_num);
    printf("%s...gently down the stream...\n",wait);
  }
  printf("Cut!\n");
  return 0;
}
