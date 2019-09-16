
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "fma_omp.h"
#include "fma_cuda.h"

#define CHK(err) do {if (err) {fprintf(stderr, "[%s, %d] Top level error\n", __FILE__, __LINE__); return err;}} while (0)

int
main (int argc, char **argv)
{
  int Nh, Nd, T;
  float *ah = NULL;
  float b, c;
  float **ad = NULL;
  int numDevices = 0;
  int err;
  int blocksize=-1;
  int gridsize=-1;
#if defined(_OPENMP)
  double time_start, time_end, time_diff;
#endif
  size_t num_flops;

  /* input processing */
  if (argc != 8) {
    printf ("Usage: %s HOST_ARRAY_SIZE DEV_ARRAY_SIZE DEV_BLOCK_SIZE DEV_GRID_SIZE LOOP_COUNT b c\n", argv[0]);
    return 1;
  }
  Nh = atoi (argv[1]);
  if (Nh < 0) {
    printf ("HOST_ARRAY_SIZE negative\n");
    return 1;
  }
  Nd = atoi (argv[2]);
  if (Nd < 0) {
    printf ("DEV_ARRAY_SIZE negative\n");
    return 1;
  }
  blocksize = atoi (argv[3]);
  gridsize = atoi (argv[4]);
  T = atoi (argv[5]);
  if (T < 0) {
    printf ("LOOP_COUNT negative\n");
    return 1;
  }
  b = atof (argv[6]);
  c = atof (argv[7]);

  if (blocksize > 0) {
    printf ("[%s] Nh = %d, Nd = %d, T = %d, block size = %d\n", argv[0], Nh, Nd, T, blocksize);
  }
  else {
    printf ("[%s] Nh = %d, Nd = %d, T = %d, default block size\n", argv[0], Nh, Nd, T);
  }


  /* initialize the array */
  err = fma_dev_initialize (Nd, T, &numDevices, &ad); CHK (err);
  err = fma_host_initialize (Nh, T, &ah); CHK (err);

#if defined (_OPENMP)
  time_start = omp_get_wtime();
#endif
  err = fma_dev_start (Nd, T, blocksize, gridsize, numDevices, ad, b, c); CHK (err);
  err = fma_host_start (Nh, T, ah, b, c); CHK (err);
  err = fma_dev_end (Nd, T, numDevices, ad, b, c); CHK (err);
  err = fma_host_end (Nh, T, ah, b, c); CHK (err);
#if defined (_OPENMP)
  time_end = omp_get_wtime();
  time_diff = time_end - time_start;
  printf ("[%s]: %e elapsed seconds\n", argv[0], time_diff);
#endif

  num_flops = (size_t) (Nh + (size_t) Nd * numDevices) * (size_t) T * 2;
  printf ("[%s]: %zu flops executed\n", argv[0], num_flops);
#if defined (_OPENMP)
  printf ("[%s]: %e flop/s\n", argv[0], (double) num_flops / time_diff);
#endif

  /* clean up */
  err = fma_host_free (Nh, T, &ah);  CHK (err);
  err = fma_dev_free (Nd, T, &numDevices, &ad);  CHK (err);
  return 0;
}
