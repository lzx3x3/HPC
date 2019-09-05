/*-----------------------------------------------------------------------*/
/* Program: STREAM                                                       */
/* Revision: $Id: stream.c,v 5.10 2013/01/17 16:01:06 mccalpin Exp mccalpin $ */
/* Original code developed by John D. McCalpin                           */
/* Programmers: John D. McCalpin                                         */
/*              Joe R. Zagar                                             */
/*                                                                       */
/* This program measures memory transfer rates in MB/s for simple        */
/* computational kernels coded in C.                                     */
/*-----------------------------------------------------------------------*/
/* Copyright 1991-2013: John D. McCalpin                                 */
/*-----------------------------------------------------------------------*/
/* License:                                                              */
/*  1. You are free to use this program and/or to redistribute           */
/*     this program.                                                     */
/*  2. You are free to modify this program for your own use,             */
/*     including commercial use, subject to the publication              */
/*     restrictions in item 3.                                           */
/*  3. You are free to publish results obtained from running this        */
/*     program, or from works that you derive from this program,         */
/*     with the following limitations:                                   */
/*     3a. In order to be referred to as "STREAM benchmark results",     */
/*         published results must be in conformance to the STREAM        */
/*         Run Rules, (briefly reviewed below) published at              */
/*         http://www.cs.virginia.edu/stream/ref.html                    */
/*         and incorporated herein by reference.                         */
/*         As the copyright holder, John McCalpin retains the            */
/*         right to determine conformity with the Run Rules.             */
/*     3b. Results based on modified source code or on runs not in       */
/*         accordance with the STREAM Run Rules must be clearly          */
/*         labelled whenever they are published.  Examples of            */
/*         proper labelling include:                                     */
/*           "tuned STREAM benchmark results"                            */
/*           "based on a variant of the STREAM benchmark code"           */
/*         Other comparable, clear, and reasonable labelling is          */
/*         acceptable.                                                   */
/*     3c. Submission of results to the STREAM benchmark web site        */
/*         is encouraged, but not required.                              */
/*  4. Use of this program or creation of derived works based on this    */
/*     program constitutes acceptance of these licensing restrictions.   */
/*  5. Absolutely no warranty is expressed or implied.                   */
/*-----------------------------------------------------------------------*/

/* I followed
 * https://github.com/nattoheaven/cuda_stream_benchmark/blob/master/stream.cu
 * while making my own modifications. -- Toby Isaac */
# include <stdio.h>
# include <unistd.h>
# include <math.h>
# include <float.h>
# include <limits.h>
# include <sys/time.h>

/*-----------------------------------------------------------------------
 * INSTRUCTIONS:
 *
 *	1) STREAM requires different amounts of memory to run on different
 *           systems, depending on both the system cache size(s) and the
 *           granularity of the system timer.
 *     You should adjust the value of 'STREAM_ARRAY_SIZE' (below)
 *           to meet *both* of the following criteria:
 *       (a) Each array must be at least 4 times the size of the
 *           available cache memory. I don't worry about the difference
 *           between 10^6 and 2^20, so in practice the minimum array size
 *           is about 3.8 times the cache size.
 *           Example 1: One Xeon E3 with 8 MB L3 cache
 *               STREAM_ARRAY_SIZE should be >= 4 million, giving
 *               an array size of 30.5 MB and a total memory requirement
 *               of 91.5 MB.  
 *           Example 2: Two Xeon E5's with 20 MB L3 cache each (using OpenMP)
 *               STREAM_ARRAY_SIZE should be >= 20 million, giving
 *               an array size of 153 MB and a total memory requirement
 *               of 458 MB.  
 *       (b) The size should be large enough so that the 'timing calibration'
 *           output by the program is at least 20 clock-ticks.  
 *           Example: most versions of Windows have a 10 millisecond timer
 *               granularity.  20 "ticks" at 10 ms/tic is 200 milliseconds.
 *               If the chip is capable of 10 GB/s, it moves 2 GB in 200 msec.
 *               This means the each array must be at least 1 GB, or 128M elements.
 *
 *      Version 5.10 increases the default array size from 2 million
 *          elements to 10 million elements in response to the increasing
 *          size of L3 caches.  The new default size is large enough for caches
 *          up to 20 MB. 
 *      Version 5.10 changes the loop index variables from "register int"
 *          to "ssize_t", which allows array indices >2^32 (4 billion)
 *          on properly configured 64-bit systems.  Additional compiler options
 *          (such as "-mcmodel=medium") may be required for large memory runs.
 *
 *      Array size can be set at compile time without modifying the source
 *          code for the (many) compilers that support preprocessor definitions
 *          on the compile line.  E.g.,
 *                gcc -O -DSTREAM_ARRAY_SIZE=100000000 stream.c -o stream.100M
 *          will override the default size of 10M with a new size of 100M elements
 *          per array.
 */
#ifndef STREAM_ARRAY_SIZE
#   define STREAM_ARRAY_SIZE	31000000
#endif

/*  2) STREAM runs each kernel "NTIMES" times and reports the *best* result
 *         for any iteration after the first, therefore the minimum value
 *         for NTIMES is 2.
 *      There are no rules on maximum allowable values for NTIMES, but
 *         values larger than the default are unlikely to noticeably
 *         increase the reported performance.
 *      NTIMES can also be set on the compile line without changing the source
 *         code using, for example, "-DNTIMES=7".
 */
#ifdef NTIMES
#if NTIMES<=1
#   define NTIMES	10
#endif
#endif
#ifndef NTIMES
#   define NTIMES	10
#endif

/*  Users are allowed to modify the "OFFSET" variable, which *may* change the
 *         relative alignment of the arrays (though compilers may change the 
 *         effective offset by making the arrays non-contiguous on some systems). 
 *      Use of non-zero values for OFFSET can be especially helpful if the
 *         STREAM_ARRAY_SIZE is set to a value close to a large power of 2.
 *      OFFSET can also be set on the compile line without changing the source
 *         code using, for example, "-DOFFSET=56".
 */
#ifndef OFFSET
#   define OFFSET	0
#endif

/*
 *	3) Compile the code with optimization.  Many compilers generate
 *       unreasonably bad code before the optimizer tightens things up.  
 *     If the results are unreasonably good, on the other hand, the
 *       optimizer might be too smart for me!
 *
 *     For a simple single-core version, try compiling with:
 *            cc -O stream.c -o stream
 *     This is known to work on many, many systems....
 *
 *     To use multiple cores, you need to tell the compiler to obey the OpenMP
 *       directives in the code.  This varies by compiler, but a common example is
 *            gcc -O -fopenmp stream.c -o stream_omp
 *       The environment variable OMP_NUM_THREADS allows runtime control of the 
 *         number of threads/cores used when the resulting "stream_omp" program
 *         is executed.
 *
 *     To run with single-precision variables and arithmetic, simply add
 *         -DSTREAM_TYPE=float
 *     to the compile line.
 *     Note that this changes the minimum array sizes required --- see (1) above.
 *
 *     The preprocessor directive "TUNED" does not do much -- it simply causes the 
 *       code to call separate functions to execute each kernel.  Trivial versions
 *       of these functions are provided, but they are *not* tuned -- they just 
 *       provide predefined interfaces to be replaced with tuned code.
 *
 *
 *	4) Optional: Mail the results to mccalpin@cs.virginia.edu
 *	   Be sure to include info that will help me understand:
 *		a) the computer hardware configuration (e.g., processor model, memory type)
 *		b) the compiler name/version and compilation flags
 *      c) any run-time information (such as OMP_NUM_THREADS)
 *		d) all of the output from the test case.
 *
 * Thanks!
 *
 *-----------------------------------------------------------------------*/

# define HLINE "-------------------------------------------------------------\n"

# ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
# endif
# ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
# endif

#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

#define CUDA_CHK(e) do {if ((e) != cudaSuccess) {printf("line %d: CUDA error: %s\n", __LINE__, cudaGetErrorString(e)); return 1;}} while(0)

/* We are going to work on device memory */
static __device__ STREAM_TYPE   a[STREAM_ARRAY_SIZE+OFFSET],
                                b[STREAM_ARRAY_SIZE+OFFSET],
                                c[STREAM_ARRAY_SIZE+OFFSET];

/* notice this is a d_ symbol, indicating that there will probably be a host and a device version */
static __device__ STREAM_TYPE d_sum[3];

static double	avgtime[4] = {0}, maxtime[4] = {0},
		mintime[4] = {FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX};

static const char	*label[4] = {"Copy:      ", "Scale:     ",
    "Add:       ", "Triad:     "};

static double	bytes[4] = {
    2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    2 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    3 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE
    };

extern double mysecond();
extern void checkSTREAMresults(dim3 grid, dim3 block);

/* This is a launchable kernel: it has the __global__ attribute */
static __global__ void STREAM_Init_1D()
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    a[j] = 1.0;
    b[j] = 2.0;
    c[j] = 0.0;
  }
}

static __global__ void STREAM_Test_1D()
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    a[j] = 2.0 * a[j];
  }
}

static __global__ void STREAM_Copy_1D()
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    c[j] = a[j];
  }
}

static __global__ void STREAM_Scale_1D(double scalar)
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    b[j] = scalar * c[j];
  }
}

static __global__ void STREAM_Add_1D()
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    c[j] = a[j] + b[j];
  }
}

static __global__ void STREAM_Triad_1D(double scalar)
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    a[j] = b[j] + scalar * c[j];
  }
}

/* Notice that this is a __device__ function */
static __device__ void STREAM_Sum_sub(double *sum, const double *d,
                                      double *shared)
{
  /* 1D decomposition */
  int tid   = blockIdx.x * blockDim.x + threadIdx.x;
  int gSize = gridDim.x * blockDim.x;
  double  x = 0.;

  /* grid stride, should work for any number of thread blocks */
  for (int j = tid; j < STREAM_ARRAY_SIZE; j += gSize) {
    x += d[j];
  }
  /* Now I have the sum for just my values: write it in the shared array */
  shared[threadIdx.x] = x;
  /* Do a reduction within the thread block */
  for (int w = blockDim.x; w != 1;) {
    int lastw = w;

    /* divide w in half or, if w is odd, ceiling of half */
    w = (w + 1) / 2;
    __syncthreads();
    if (threadIdx.x + w < lastw) {
      x += shared[threadIdx.x + w];
      shared[threadIdx.x] = x;
    }
  }

  /* one thread per block */
  /* This strange behavior is for devices with compute capabilities < 6.x,
     otherwise we can use atomicAdd() with doubles */
  if (threadIdx.x == 0) {
    unsigned long long int *address = (unsigned long long int *)sum;
    unsigned long long int old = *address, assumed;
    do {
      assumed = old;
      old = atomicCAS(address, assumed,
	  __double_as_longlong(x + __longlong_as_double(assumed)));
    } while (assumed != old);
  }
}

static __global__ void STREAM_Sum()
{
  /* get access to the shared memory allocated at the start of the call */
  extern __shared__ double shared[];
  STREAM_Sum_sub(&d_sum[0], a, shared);
  STREAM_Sum_sub(&d_sum[1], b, shared);
  STREAM_Sum_sub(&d_sum[2], c, shared);
}

int
main()
    {
    int     nDevice;
    int			quantum, checktick();
    int			BytesPerWord;
    int			k;
    ssize_t		j;
    STREAM_TYPE		scalar;
    double		t, times[4][NTIMES];
    dim3                grid, block;
    cudaError_t         err;

    /* --- SETUP --- determine precision and check timing --- */

    printf(HLINE);
    printf("CSE6230 CUDA STREAM based on version $Revision: 5.10 $\n");
    printf(HLINE);
    BytesPerWord = sizeof(STREAM_TYPE);
    printf("This system uses %d bytes per array element.\n",
	BytesPerWord);

    printf(HLINE);
#ifdef N
    printf("*****  WARNING: ******\n");
    printf("      It appears that you set the preprocessor variable N when compiling this code.\n");
    printf("      This version of the code uses the preprocesor variable STREAM_ARRAY_SIZE to control the array size\n");
    printf("      Reverting to default value of STREAM_ARRAY_SIZE=%llu\n",(unsigned long long) STREAM_ARRAY_SIZE);
    printf("*****  WARNING: ******\n");
#endif

    printf("Array size = %llu (elements), Offset = %d (elements)\n" , (unsigned long long) STREAM_ARRAY_SIZE, OFFSET);
    printf("Memory per array = %.1f MiB (= %.1f GiB).\n", 
	BytesPerWord * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.0),
	BytesPerWord * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.0/1024.0));
    printf("Total memory required = %.1f MiB (= %.1f GiB).\n",
	(3.0 * BytesPerWord) * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024.),
	(3.0 * BytesPerWord) * ( (double) STREAM_ARRAY_SIZE / 1024.0/1024./1024.));
    printf("Each kernel will be executed %d times.\n", NTIMES);
    printf(" The *best* time for each kernel (excluding the first iteration)\n"); 
    printf(" will be used to compute the reported bandwidth.\n");

    /* Get initial value for system clock. */
    {
      int k;
      int x;
      struct cudaDeviceProp prop;
      err = cudaGetDeviceCount(&nDevice); CUDA_CHK(err);
      for (k = 0; k < nDevice; k++) {
        err = cudaGetDeviceProperties(&prop, k); CUDA_CHK(err);
        printf("Device Number: %d\n", k);
        printf("  Device name: %s\n", prop.name);
        printf("  Memory Clock Rate (KHz): %d\n", prop.memoryClockRate);
        printf("  Memory Bus Width (bits): %d\n", prop.memoryBusWidth);
        printf("  Peak Memory Bandwidth (GB/s): %f\n\n", 2.0*prop.memoryClockRate*(prop.memoryBusWidth/8)/1.0e6);
      }
      err = cudaGetDevice(&k); CUDA_CHK(err);
      printf ("Ordinal of GPUs requested = %i\n",k);
      err = cudaGetDeviceProperties(&prop, k); CUDA_CHK(err);
      block = dim3(prop.maxThreadsPerBlock);
      x = (STREAM_ARRAY_SIZE + block.x - 1) / block.x;
      grid = dim3(x);
    }

    printf(HLINE);

    /* Get initial value for system clock. */
    STREAM_Init_1D<<<grid, block>>>();

    printf(HLINE);

    if  ( (quantum = checktick()) >= 1) 
	printf("Your clock granularity/precision appears to be "
	    "%d microseconds.\n", quantum);
    else {
	printf("Your clock granularity appears to be "
	    "less than one microsecond.\n");
	quantum = 1;
    }

    t = mysecond();
    STREAM_Test_1D<<<grid, block>>>();
    err = cudaDeviceSynchronize(); CUDA_CHK(err);
    t = 1.0E6 * (mysecond() - t);

    printf("Each test below will take on the order"
	" of %d microseconds.\n", (int) t  );
    printf("   (= %d clock ticks)\n", (int) (t/quantum) );
    printf("Increase the size of the arrays if this shows that\n");
    printf("you are not getting at least 20 clock ticks per test.\n");

    printf(HLINE);

    printf("WARNING -- The above is only a rough guideline.\n");
    printf("For best results, please be sure you know the\n");
    printf("precision of your system timer.\n");
    printf(HLINE);
    
    /*	--- MAIN LOOP --- repeat test cases NTIMES times --- */

    scalar = 3.0;
    for (k=0; k<NTIMES; k++)
	{
	times[0][k] = mysecond();
        STREAM_Copy_1D<<<grid, block>>>();
	err = cudaDeviceSynchronize(); CUDA_CHK(err);
	times[0][k] = mysecond() - times[0][k];
	
	times[1][k] = mysecond();
        STREAM_Scale_1D<<<grid, block>>>(scalar);
	err = cudaDeviceSynchronize(); CUDA_CHK(err);
	times[1][k] = mysecond() - times[1][k];
	
	times[2][k] = mysecond();
        STREAM_Add_1D<<<grid, block>>>();
	err = cudaDeviceSynchronize(); CUDA_CHK(err);
	times[2][k] = mysecond() - times[2][k];
	
	times[3][k] = mysecond();
        STREAM_Triad_1D<<<grid, block>>>(scalar);
	err = cudaDeviceSynchronize(); CUDA_CHK(err);
	times[3][k] = mysecond() - times[3][k];
	}

    /*	--- SUMMARY --- */

    for (k=1; k<NTIMES; k++) /* note -- skip first iteration */
	{
	for (j=0; j<4; j++)
	    {
	    avgtime[j] = avgtime[j] + times[j][k];
	    mintime[j] = MIN(mintime[j], times[j][k]);
	    maxtime[j] = MAX(maxtime[j], times[j][k]);
	    }
	}
    
    printf("Function    Best Rate MB/s  Avg time     Min time     Max time\n");
    for (j=0; j<4; j++) {
		avgtime[j] = avgtime[j]/(double)(NTIMES-1);

		printf("%s%12.1f  %11.6f  %11.6f  %11.6f\n", label[j],
	       1.0E-06 * bytes[j]/mintime[j],
	       avgtime[j],
	       mintime[j],
	       maxtime[j]);
    }
    printf(HLINE);

    /* --- Check Results --- */
    checkSTREAMresults(grid, block);
    printf(HLINE);

    return 0;
}

# define	M	20

int
checktick()
    {
    int		i, minDelta, Delta;
    double	t1, t2, timesfound[M];

/*  Collect a sequence of M unique time values from the system. */

    for (i = 0; i < M; i++) {
	t1 = mysecond();
	while( ((t2=mysecond()) - t1) < 1.0E-6 )
	    ;
	timesfound[i] = t1 = t2;
	}

/*
 * Determine the minimum difference between these M values.
 * This result will be our estimate (in microseconds) for the
 * clock granularity.
 */

    minDelta = 1000000;
    for (i = 1; i < M; i++) {
	Delta = (int)( 1.0E6 * (timesfound[i]-timesfound[i-1]));
	minDelta = MIN(minDelta, MAX(Delta,0));
	}

   return(minDelta);
    }



/* A gettimeofday routine to give access to the wall
   clock timer on most UNIX-like systems.  */

#include <sys/time.h>

double mysecond()
{
        struct timeval tp;
        struct timezone tzp;

        (void) gettimeofday(&tp,&tzp);
        return ( (double) tp.tv_sec + (double) tp.tv_usec * 1.e-6 );
}

#ifndef abs
#define abs(a) ((a) >= 0 ? (a) : -(a))
#endif
void checkSTREAMresults (dim3 grid, dim3 block)
{
	STREAM_TYPE aj,bj,cj,scalar;
	double h_sum[3];
	double epsilon;
	int	k,serr;

    /* reproduce initialization */
	aj = 1.0;
	bj = 2.0;
	cj = 0.0;
    /* a[] is modified during timing check */
	aj = 2.0E0 * aj;
    /* now execute timing loop */
	scalar = 3.0;
	for (k=0; k<NTIMES; k++)
        {
            cj = aj;
            bj = scalar*cj;
            cj = aj+bj;
            aj = bj+scalar*cj;
        }

	aj = aj * (double) (STREAM_ARRAY_SIZE);
	bj = bj * (double) (STREAM_ARRAY_SIZE);
	cj = cj * (double) (STREAM_ARRAY_SIZE);

	h_sum[0] = 0.0;
	h_sum[1] = 0.0;
	h_sum[2] = 0.0;
	cudaMemcpyToSymbol(d_sum, h_sum, 3 * sizeof(double));
	STREAM_Sum<<<grid, block, block.x * sizeof(double)>>>();
	cudaMemcpyFromSymbol(h_sum, d_sum, 3 * sizeof(double));

	if (sizeof(STREAM_TYPE) == 4) {
		epsilon = 1.e-6;
	}
	else if (sizeof(STREAM_TYPE) == 8) {
		epsilon = 1.e-12;
	}
	else {
		printf("WEIRD: sizeof(STREAM_TYPE) = %lu\n",sizeof(STREAM_TYPE));
		epsilon = 1.e-6;
	}
	serr = 0;
	if (abs(aj-h_sum[0])/h_sum[0] > epsilon) {
	  serr++;
	  printf ("Failed Validation on array a[]\n");
	  printf ("        Expected  : %f \n",aj);
	  printf ("        Observed  : %f \n",h_sum[0]);
	}
	if (abs(bj-h_sum[1])/h_sum[1] > epsilon) {
	  serr++;
	  printf ("Failed Validation on array b[]\n");
	  printf ("        Expected  : %f \n",bj);
	  printf ("        Observed  : %f \n",h_sum[1]);
	}
	if (abs(cj-h_sum[2])/h_sum[2] > epsilon) {
	  serr++;
	  printf ("Failed Validation on array c[]\n");
	  printf ("        Expected  : %f \n",cj);
	  printf ("        Observed  : %f \n",h_sum[2]);
	}

	if (serr == 0) {
		printf ("Solution Validates: avg error less than %e on all three arrays\n",epsilon);
	}
#ifdef VERBOSE
	printf ("Results Validation Verbose Results: \n");
	printf ("    Expected a(1), b(1), c(1): %f %f %f \n",aj,bj,cj);
	printf ("    Observed a(1), b(1), c(1): %f %f %f \n",a[1],b[1],c[1]);
	printf ("    Rel Errors on a, b, c:     %e %e %e \n",abs(aAvgErr/aj),abs(bAvgErr/bj),abs(cAvgErr/cj));
#endif
}
