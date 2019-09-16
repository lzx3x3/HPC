
#include <tictoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cse6230rand.h>
#include "cloud.h"
#include "verlet.h"

int safe_malloc (size_t count, void *out, const char * file, const char * fn, int line)
{
  if (!count) {
    *((void **) out) = NULL;
    return 0;
  }
  *((void **) out) = malloc (count);
  if (!(*((void **) out))) {
    fprintf (stderr, "%s, %s (%d): failed to malloc %zu bytes\n", file, fn, line, count);
    return 1;
  }
  return 0;
}

#define safeMALLOC(count,out) safe_malloc (count, out, __FILE__, __func__, __LINE__)
#define CHK(Q) if (Q) return Q

static int
write_step (int Np, double T, double H, const double *X[3])
{
  printf ("{\"num_points\":%d,", Np);
  printf ("\"time\":%g,", T);
  printf ("\"hamiltonian\":%g,", H);
  printf("\"X\":[");
  for (int d=0; d < 3; d++) {
    printf ("[");
    for (int l = 0; l < Np - 1; l++) {
      printf ("%9.2e,", X[d][l]);
    }
    if (Np) {printf ("%9.2e]", X[d][Np - 1]);}
    if (d < 2) {
      printf (",");
    }
  }
  printf("]}\n");
  return 0;
}

int
process_options (int argc, char **argv, int *Np, int *Nt, int *Nint, double *dt, double *k, double *d, const char **gifname)
{
  if (argc < 6 || argc > 8) {
    printf ("Usage: %s NUM_POINTS NUM_STEPS DT K D [CHUNK_SIZE GIFNAME]\n", argv[0]);
    return 1;
  }
  *Np = atoi (argv[1]);
  *Nt = atoi (argv[2]);
  *dt = (double) atof (argv[3]);
  *k  = (double) atof (argv[4]);
  *d  = (double) atof (argv[5]);
  if (argc > 6) {
    *Nint = atoi (argv[6]);
    if (argc == 8) {
      *gifname = argv[7];
    }
    else {
      *gifname = NULL;
    }
  }
  else {
    *Nint = *Nt;
  }
  return 0;
}


int
main (int argc, char **argv)
{
  int Np, Nt, err;
  int Nint;
  double dt;
  double k;
  double d;
  double *X0[3];
  double *X[3], *U[3];
  double Hin, Hout;
  int seed = 6230;
  const char *gifname = NULL;
  cse6230rand_t rand;
  TicTocTimer loop_timer;
  double loop_time;

  err = process_options (argc, argv, &Np, &Nt, &Nint, &dt, &k, &d, &gifname);CHK(err);

  for (int d = 0; d < 3; d++) {
    err = safeMALLOC (Np * sizeof (double), &X0[d]);CHK(err);
    err = safeMALLOC (Np * sizeof (double), &X[d]);CHK(err);
    err = safeMALLOC (Np * sizeof (double), &U[d]);CHK(err);
  }

  cse6230rand_seed (seed, &rand);

  initialize_variables (Np, k, &rand, X0, X, U);

  Hin = compute_hamiltonian (Np, k, (const double **)X, (const double **)U);
  if (!gifname) {
    printf ("[%s] NUM_POINTS=%d, NUM_STEPS=%d, CHUNK_SIZE=%d, DT=%g, K=%g, D=%g\n", argv[0], Np, Nt, Nint, dt, k, d);
    printf ("[%s] Hamiltonian, T = 0: %g\n", argv[0], Hin);
  }
  else {
    printf ("{ \"num_points\": %d, \"k\": %e, \"d\": %e, \"dt\": %e, \"num_steps\": %d, \"step_chunk\": %d, \"hamiltonian_0\": %e, \"gifname\":\"%s\" }\n",
            Np, k, d, dt, Nt, Nint, Hin, gifname);
  }


  loop_timer = tic();
  for (int t = 0; t < Nt; t += Nint) {
    if (gifname) {
      Hout = compute_hamiltonian (Np, k, (const double **)X, (const double **)U);
      write_step (Np, t * dt, Hout, (const double **)X);
    }
    /* execute the loop */
    verlet_step (Np, Nint, dt, k, d, &rand, X, U);
  }
  loop_time = toc(&loop_timer);
  Hout = compute_hamiltonian (Np, k, (const double **)X, (const double **)U);
  if (gifname) {write_step (Np, Nt * dt, Hout, (const double **)X);}

  {
    double avgDist = 0.;

    for (int i = 0; i < Np; i++) {
      double this_dist;

      this_dist = 0.;
      for (int d = 0; d < 3; d++) {
        this_dist += (X[d][i] - X0[d][i]) * (X[d][i] - X0[d][i]);
      }
      this_dist = sqrt (this_dist);
      avgDist += this_dist;
    }
    avgDist /= Np;

    if (!gifname) {
      printf ("[%s] Simulation walltime: %g\n", argv[0], loop_time);
      printf ("[%s] Hamiltonian, T = %g: %g, Relative Error: %g\n", argv[0], Nt * dt, Hout, Hin ? fabs(Hout - Hin) / Hin: 0.);
      printf ("[%s] Average Distance Traveled: %g\n", argv[0], avgDist);
    } else {
      printf ("{ \"avg_dist\": %e, \"hamiltonian_T\": %e, \"hamiltionian_relerr\": %e, \"sim_time\": %e }\n",
              avgDist, Hout, Hin ? fabs(Hout - Hin) / Hin : 0., loop_time);
      fflush(stdin);
    }
  }

  for (int d = 0; d < 3; d++) {
    free (U[d]);
    free (X[d]);
    free (X0[d]);
  }
  return 0;
}
