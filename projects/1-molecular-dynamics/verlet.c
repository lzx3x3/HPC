#include <math.h>
#include "cloud_util.h"
#include "verlet.h"

struct _verlet_t
{
  double         d;
  Accel          accel;
  cse6230rand_t *rand;
};

int
VerletCreate(Verlet *verlet)
{
  int err;
  Verlet v;

  err = safeMALLOC(sizeof(*v),&v); CHK(err);
  v->d    = 0.;
  v->rand = NULL;
  v->accel = NULL;
  *verlet = v;
  return 0;
}

int
VerletSetNoise(Verlet v, cse6230rand_t *rand, double d)
{
  v->d    = d;
  v->rand = rand;
  return 0;
}

int
VerletSetAccel(Verlet v, Accel accel)
{
  v->accel = accel;
  return 0;
}

int
VerletDestroy(Verlet *verlet)
{
  free (*verlet);
  *verlet = 0;
  return 0;
}

static void
stream_and_noise (Verlet Vr, double dt_stream, double dt_noise,
                  Vector X, Vector U)
{
  int Np = X->Np;
  cse6230rand_t *rand = Vr->rand;
  size_t tag;

  tag = cse6230rand_get_tag (rand);
  #pragma omp parallel for schedule(static)
  for (int i = 0; i < Np; i+= 4) {
    double rval[3][4];

    for (int d = 0; d < 3; d++) {
      cse6230rand_normal_hash (rand, tag, i, d, 0, &rval[d][0]);
    }

    if (i + 4 <= Np) {
      for (int d = 0; d < 3; d++) {
        for (int j = 0; j < 4; j++) {
          IDX(X,d,i + j) += dt_stream * IDX(U,d,i + j) + dt_noise * rval[d][j];
        }
      }
    }
    else {
      for (int d = 0; d < 3; d++) {
        for (int j = 0; j < Np - i; j++) {
          IDX(X,d,i + j) += dt_stream * IDX(U,d,i + j) + dt_noise * rval[d][j];
        }
      }
    }
  }
}

void
verlet_step (Verlet Vr, int Nt, double dt, Vector X, Vector U)
{
  int t;
  double d = Vr->d;

  double dt_noise = sqrt(2. * d * dt);

  for (t = 0; t < Nt; t++) {
    accelerate (Vr->accel, X, U);
    stream_and_noise (Vr, dt, dt_noise, X, U);
  }
}

