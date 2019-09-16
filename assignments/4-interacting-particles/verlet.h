#if !defined(VERLET_H)
#define      VERLET_H

#include <cse6230rand.h>

void
verlet_step (int Np, int Nt, double dt, double k, double d, cse6230rand_t *rand, double *restrict X[3], double *restrict V[3]);

void
verlet_step_stream_and_noise (int Np, double dt_stream, double dt_noise, cse6230rand_t *rand, double *restrict X[3], const double *restrict V[3]);

void
verlet_step_accelerate (int Np, double dt, const double *restrict X[3], double *restrict V[3]);

#endif
