#if !defined(VERLET_H)
#define      VERLET_H

#include <cse6230rand.h>
#include "accelerate.h"

typedef struct _verlet_t *Verlet;

int VerletCreate(Verlet *v);
int VerletSetNoise(Verlet Vr, cse6230rand_t *rand, double d);
int VerletSetAccel(Verlet Vr, Accel Ac);
int VerletDestroy(Verlet *Vr);

void
verlet_step (Verlet Vr, int Nt, double dt, Vector X, Vector V);

#endif
