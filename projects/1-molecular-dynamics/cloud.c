
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cse6230rand.h>
#include "cloud_util.h"
#include "initialize.h"
#include "verlet.h"

static int
write_step (Vector X, double L, double T)
{
  int Np = X->Np;
  printf ("{\"num_points\":%d,", Np);
  printf ("\"domain_width\":%g,", L);
  printf ("\"time\":%g,", T);
  printf("\"X\":[");
  for (int d=0; d < 3; d++) {
    printf ("[");
    for (int l = 0; l < Np - 1; l++) {
      printf ("%9.2e,", IDX(X,d,l));
    }
    if (Np) {printf ("%9.2e]", IDX(X,d,Np - 1));}
    if (d < 2) {
      printf (",");
    }
  }
  printf("]}\n");
  return 0;
}

int
process_options (int argc, char **argv, int *Np, int *Nt, int *Nint, double *dt, double *k, double *d, double *L, double *r, const char **gifname)
{
  if (argc < 8 || argc > 10) {
    printf ("Usage: %s NUM_POINTS NUM_STEPS DT K D L R [CHUNK_SIZE GIFNAME]\n", argv[0]);
    return 1;
  }
  *Np = atoi (argv[1]);
  *Nt = atoi (argv[2]);
  *dt = (double) atof (argv[3]);
  *k  = (double) atof (argv[4]);
  *d  = (double) atof (argv[5]);
  *L  = (double) atof (argv[6]);
  *r  = (double) atof (argv[7]);
  if (argc > 8) {
    *Nint = atoi (argv[8]);
    if (argc == 10) {
      *gifname = argv[9];
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
  double L;
  double r;
  Vector X0, X, U;
  Verlet Vr;
  Accel  Ac;
  int seed = 6230;
  const char *gifname = NULL;
  cse6230rand_t rand;

  err = process_options (argc, argv, &Np, &Nt, &Nint, &dt, &k, &d, &L, &r, &gifname);CHK(err);

  err = VectorCreate(Np, &X0);CHK(err);
  err = VectorCreate(Np, &X);CHK(err);
  err = VectorCreate(Np, &U);CHK(err);

  cse6230rand_seed (seed, &rand);

  initialize_positions (X0, X, L, &rand);

  if (!gifname) {
    printf ("[%s] NUM_POINTS=%d, NUM_STEPS=%d, CHUNK_SIZE=%d, DT=%g, K=%g, D=%g, L=%g, R=%g\n", argv[0], Np, Nt, Nint, dt, k, d, L, r);
    {
      double pr_vol = (4./3.) * CSE6230_PI * r*r*r;
      double domain_vol = L*L*L;
      double vol_frac = pr_vol * Np / domain_vol;
      double interact_vol = 8. * pr_vol;
      double exp_ints_per = Np * interact_vol / domain_vol;
      printf("With %d particles of radius 1 and a box width of %f, the volume fraction is %g.\n",Np,L,vol_frac);
      printf("The interaction volume is %g, so we expect %g interactions per particle, %g overall.\n",interact_vol,exp_ints_per,exp_ints_per * Np / 2.);
    }
  }
  else {
    printf ("{ \"num_points\": %d, \"k\": %e, \"d\": %e, \"r\": %e, \"L\": %e, \"dt\": %e, \"num_steps\": %d, \"step_chunk\": %d, \"gifname\":\"%s\" }\n",
            Np, k, d, r, L, dt, Nt, Nint, gifname);
    fflush(stdout);
  }

  err = VerletCreate(&Vr);CHK(err);
  err = VerletSetNoise(Vr, &rand, d);CHK(err);

  int use_ix = 1;
  err = AccelCreate(Np, L, k, r, use_ix, &Ac);CHK(err);

  err = VerletSetAccel(Vr, Ac);CHK(err);

  for (int t = 0; t < Nt; t += Nint) {
    if (gifname) {
      write_step (X, L, t * dt);
    }
    /* execute the loop */
    verlet_step (Vr, Nint, dt, X, U);
  }
  if (gifname) {write_step (X, L, Nt * dt);}

  err = AccelDestroy(&Ac);CHK(err);
  err = VerletDestroy(&Vr);CHK(err);
  err = VectorDestroy(&U);CHK(err);
  err = VectorDestroy(&X);CHK(err);
  err = VectorDestroy(&X0);CHK(err);
  return 0;
}
