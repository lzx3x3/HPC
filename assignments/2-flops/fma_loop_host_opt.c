
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
  for  (int i = 0; i < N; i+=24){
    for (int j = 0; j < T; j++) {
      a[i] = a[i] * b + c;
      a[i+1] = a[i+1] * b + c;
      a[i+2] = a[i+2] * b + c;
      a[i+3] = a[i+3] * b + c;
      a[i+4] = a[i+4] * b + c;
      a[i+5] = a[i+5] * b + c;     
      a[i+6] = a[i+6] * b + c;
      a[i+7] = a[i+7] * b + c;
      a[i+8] = a[i+8] * b + c;     
      a[i+9] = a[i+9] * b + c;
      a[i+10] = a[i+10] * b + c;
      a[i+11] = a[i+11] * b + c;
      a[i+12] = a[i+12] * b + c;

      a[i+13] = a[i+13] * b + c;
      a[i+14] = a[i+14] * b + c;
      a[i+15] = a[i+15] * b + c;     
      a[i+16] = a[i+16] * b + c;
      a[i+17] = a[i+17] * b + c;
      a[i+18] = a[i+18] * b + c;     
      a[i+19] = a[i+19] * b + c;
      a[i+20] = a[i+20] * b + c;
      a[i+21] = a[i+21] * b + c;
      a[i+22] = a[i+22] * b + c;
      a[i+23] = a[i+23] * b + c;




    }
  }
}

