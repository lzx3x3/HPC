#include <math.h>
#include "cloud.h"
#include "verlet.h"

void
verlet_step (int Np, int Nt, double dt, double k, double d,
             cse6230rand_t *rand,
             double *restrict X[3],
             double *restrict U[3])
{
  int t;
  double dt_accel = k * dt;
  double dt_half  = 0.5 * dt;
  double dt_fullnoise = sqrt(2. * d * dt);
  double dt_halfnoise = sqrt(d * dt);

  if (Nt > 0) {
    verlet_step_stream_and_noise (Np, dt_half, dt_halfnoise, rand, X, (const double *restrict *) U);
  }
  for (t = 0; t < Nt - 1; t++) {
    if (dt_accel) {verlet_step_accelerate (Np, dt_accel, (const double *restrict *) X, U);}
    verlet_step_stream_and_noise (Np, dt, dt_fullnoise, rand, X, (const double *restrict *) U);
  }
  for (; t < Nt; t++) {
    if (dt_accel) {verlet_step_accelerate (Np, dt_accel, (const double *restrict *) X, U);}
    verlet_step_stream_and_noise (Np, dt_half, dt_halfnoise, rand, X, (const double *restrict *) U);
  }
}

