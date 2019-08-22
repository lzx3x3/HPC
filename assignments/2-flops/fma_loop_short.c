
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
fma_loop_short (int N, int T, float *a, float b, float c)
{
  for (int i = 0; i < N; i++) {
    a[i] = a[i] * b + c;
  }
}
