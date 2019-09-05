/* randomized quicksort */
#include <stdio.h>
#include <stdlib.h>
#include <tictoc.h>

int *
create_random_list(int N)
{
  int *list = (int *) malloc(N * sizeof(int));
  int i;

  for (i = 0; i < N; i++) {
    list[i] = (int) rand();
  }

  return list;
}

int
list_is_sorted(int N, const int *list)
{
  int i;

  for (i = 0; i < N - 1; i++) {
    if (list[i] > list[i + 1]) {
      return 0;
    }
  }
  return 1;
}

static inline void
list_swap(int *list, int a, int b)
{
  int temp = list[a];

  list[a] = list[b];
  list[b] = temp;
}

void
sort_int_list(int N, int *list)
{
  int idx, i, last, key;

  if (N <= 1) return;

  idx = (int) rand() % N;
  list_swap(list,0,idx);

  key = list[0];
  last = N;
  for (i = 1; i < last;) {
    int test = list[i];

    if (test >= key) {
      list_swap(list,i,--last);
    } else {
      i++;
    }
  }
  list_swap(list,0,i-1);
  sort_int_list(i-1,list);
  sort_int_list(N - i,&list[i]);
}

int
main(int argc, char **argv)
{
  int         N = 10000000;
  int         *list, sorted;
  double      time;
  TicTocTimer timer;

  if (argc > 1) {
    N = atoi(argv[1]);
  }

  printf("Creating a list of %d random entries: ",N);
  timer = tic();
  list = create_random_list(N);
  time = toc(&timer);
  printf("(%e secs)\n",time);
  printf("  done.\n");

  printf("Checking list: ");
  timer = tic();
  sorted = list_is_sorted(N, list);
  time = toc(&timer);
  printf("(%e secs)\n",time);
  if (sorted) {
    printf("  sorted, done.\n");
    free(list);
    return 0;
  }
  printf("  not sorted, let's get to work.\n");

  printf("Sorting list: ");
  timer = tic();
  sort_int_list(N, list);
  time = toc(&timer);
  printf("(%e secs)\n",time);
  printf("  done.\n");

  printf("Checking list: ");
  timer = tic();
  sorted = list_is_sorted(N, list);
  time = toc(&timer);
  printf("(%e secs)\n",time);
  if (sorted) {
    printf("  sorted, hooray!\n");
    free(list);
    return 0;
  }
  printf("  not sorted, uh oh.\n");

  return -1;
}
