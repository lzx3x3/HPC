
#include "cloud_util.h"
#include "vector.h"

int
VectorCreate(int Np, Vector *vec)
{
  Vector v;
  int err;

  err = safeMALLOC(sizeof(*v),&v);CHK(err);
  v->Np = Np;
#if defined(VECTOR_AOS)
  {
    double *vals;
    err = safeMALLOC(Np * 3 * sizeof(double), &vals);CHK(err);
    v->v = (double (*) [3]) vals;
  }
#else
  for (int d = 0; d < 3; d++) {
    err = safeMALLOC(Np * sizeof(double), &(v->v[d]));CHK(err);
  }
#endif
  *vec = v;
  return 0;
}

int
VectorDestroy(Vector *vec)
{
#if defined(VECTOR_AOS)
  free((*vec)->v);
#else
  for (int d = 0; d < 3; d++) {
    free((*vec)->v[d]);
  }
#endif
  free(*vec);
  *vec = NULL;
  return 0;
}
