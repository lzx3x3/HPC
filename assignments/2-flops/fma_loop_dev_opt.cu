
#include "fma_dev.h"

__global__
void
fma_loop_dev (int N, int T, float *a, float b, float c)
{
  int my_thread = threadIdx.x + blockIdx.x * blockDim.x;
  int num_threads = gridDim.x * blockDim.x;

  for (int i = my_thread; i < N; i+= num_threads) {
    for (int j = 0; j < T; j++) {
      a[i] = a[i] * b + c;
    }
  }
}
