
#include "cloud_util.h"
#include "accelerate.h"
#include "steric.h"
#include "interactions.h"

struct _accel_t
{
  int Np;
  double L;
  double k;
  double r;
  int use_ix;
  IX ix;
};

int
AccelCreate(int Np, double L, double k, double r, int use_ix, Accel *accel)
{
  int err;
  Accel a;

  err = safeMALLOC(sizeof(*a), &a);CHK(err);
  a->Np = Np;
  a->L  = L;
  a->k  = k;
  a->r  = r;
  a->use_ix = use_ix;
  if (use_ix) {
    int boxdim = 4; /* how could we choose boxdim ? */
    int maxNx = Np; /* how should we estimate the maximum number of interactions? */
    err = IXCreate(L, boxdim, maxNx, &(a->ix));CHK(err);
  }
  else {
    a->ix = NULL;
  }
  *accel = a;
  return 0;
}

int
AccelDestroy(Accel *accel)
{
  int err;

  if ((*accel)->ix) {
    err = IXDestroy(&((*accel)->ix));CHK(err);
  }
  free (*accel);
  *accel = NULL;
  return 0;
}

static void
accelerate_ix (Accel accel, Vector X, Vector U)
{
  IX ix = accel->ix;
  int Np = X->Np;
  int Npairs;
  ix_pair *pairs;
  double L = accel->L;
  double k = accel->k;
  double r = accel->r;
  #pragma omp parallel for // I added this 
  for (int i = 0; i < Np; i++) {
    for (int j = 0; j < 3; j++) {
      IDX(U,j,i) = 0.;
    }
  }

  IXGetPairs (ix, X, 2.*r, &Npairs, &pairs);
  #pragma omp parallel for // I added this 
  for (int p = 0; p < Npairs; p++) {
    int i = pairs[p].p[0];
    int j = pairs[p].p[1];
    double du[3];

    force (k, r, L, IDX(X,0,i), IDX(X,1,i), IDX(X,2,i), IDX(X,0,j), IDX(X,1,j), IDX(X,2,j), du);

    for (int d = 0; d < 3; d++) {
      IDX(U,d,i) += du[d];
      IDX(U,d,j) -= du[d];
    }
  }
  IXRestorePairs (ix, X, 2.*r, &Npairs, &pairs);
}

static void
accelerate_direct (Accel accel, Vector X, Vector U)
{
  int Np = accel->Np;
  double L = accel->L;
  double k = accel->k;
  double r = accel->r;

  #pragma omp parallel for schedule(static)
  for (int i = 0; i < Np; i++) {
    double u[3] = {0.};

    for (int j = 0; j < Np; j++) {
      if (j != i) {
        double du[3];

        force (k, r, L, IDX(X,0,i), IDX(X,1,i), IDX(X,2,i), IDX(X,0,j), IDX(X,1,j), IDX(X,2,j), du);

        for (int d = 0; d < 3; d++) {
          u[d] += du[d];
        }
      }
    }
    for (int d = 0; d < 3; d++) {
      /* Instead of adding to the velocity,
       * For this project the computed interactions give the complete velocity */
      IDX(U,d,i) = u[d];
    }
  }
}

void
accelerate (Accel accel, Vector X, Vector U)
{
  if (accel->use_ix) {
    accelerate_ix (accel, X, U);
  }
  else {
    accelerate_direct (accel, X, U);
  }
}


