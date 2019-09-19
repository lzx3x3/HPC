#if !defined(INTERACTIONS_H)
#define      INTERACTIONS_H

#include "vector.h"

typedef struct _ix *IX;

typedef struct _ix_pair
{
  int p[2];
}
ix_pair;

int IXCreate(double L, int boxdim, int maxNx, IX *ix);
int IXDestroy(IX *ix);

/* get a list of interacting pairs that contains at least all pairs of
 * particles less than r distance apart from each other */
int IXGetPairs(IX ix, Vector X, double r, int *Npairs, ix_pair **pairs);
int IXRestorePairs(IX ix, Vector X, double r, int *Npairs, ix_pair **pairs);

/* Verify directly that a list of particles contains all pairs
 * less than r distance apart from each other */
int interactions_check(IX ix, Vector X, double r, int Npairs, ix_pair *pairs, int *total);

#endif
