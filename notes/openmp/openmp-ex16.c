/* This example for computing pi is adapted from Hager & Wellein, Listing 6.2.
 *
 * We compute $\pi = \int_0^1 \frac{4}{1 + x^2} dx$.
 *
 * In this example we use two-point Gaussian quadrature rule.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <tictoc.h>

int main(int argc, char **argv)
{
  int         i, j, M = 10, N = 10;
  double      h, phi[2], relerr, pi = M_PI;
  double      mintime = 0., maxtime = 0., avgtime = 0.;
  TicTocTimer timer;

  /* if this program is run with an argument, take the first argument to be N */
  if (argc > 1) {
    N = atoi(argv[1]);
  }

  /* h is the width of each interval */
  h = 1. / N;
  /* compute the quadrature points and weights */
  phi[0] = (-sqrt(1./3.) + 1.) / 2.;
  phi[1] = ( sqrt(1./3.) + 1.) / 2.;

  for (j = 0; j < M; j++) {
    double time;

    pi    = 0.;
    timer = tic();
#pragma omp parallel for reduction(+:pi)
    for (i = 0; i < N; i++) {
      int k;

      for (k = 0; k < 2; k++) {
        double x = h * (i + phi[k]);

        /* let's pretend this is a lengthier calculation */
        usleep(1);

        pi += h * 0.5 * 4. / (1. + x*x);
      }
    }
    time = toc(&timer);
    if (j == 1) {
      mintime = maxtime = avgtime = time;
    } else if (j > 1) {
      mintime = time < mintime ? time : mintime;
      maxtime = time > maxtime ? time : maxtime;
      avgtime += time;
    }
  }
  avgtime /= (M - 1);

  relerr = fabs(M_PI - pi) / M_PI;

  printf("Computed pi %g, relative error %g\n", pi, relerr);
  printf("Calculation time %g [%g, %g]\n", avgtime, mintime, maxtime);

  return 0;
}
