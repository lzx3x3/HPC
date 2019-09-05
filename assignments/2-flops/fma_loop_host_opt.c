
#include "fma_host.h"

/* fma_loop: Fused Multiply Add loop
 *           -     -        -
 *
 * a[:] = a[:] * b + c, T times
 *
 * Inputs:
 * N : the size of the array
 * T : the number of loops
 * b : the multiplier
 * c : the shift
 *
 * Input-Outputs:
 * a : the array
 */
void
fma_loop_host (int N, int T, float *a, float b, float c)
{
  for  (int i = 0; i < N; i++){
    for (int j = 0; j < T; j+=12) {
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;
      a[i] = a[i] * b + c;




    }
  }
}

