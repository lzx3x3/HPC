
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
  #pragma unroll(12)
  for (int i = 0; i < N; i+=8) {
      a[i] = a[i] * b + c;
      a[i+1] = a[i+1]*b + c;
      a[i+2] = a[i+2]*b + c;
      a[i+3] = a[i+3]*b + c;
      a[i+4] = a[i+4] * b + c;
      a[i+5] = a[i+5]*b + c;
      a[i+6] = a[i+6]*b + c;
      a[i+7] = a[i+7]*b + c;




    
  }
}
