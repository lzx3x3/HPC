
#include "cloud.h"
#include "verlet.h"

void
verlet_step_stream_and_noise (int Np, double dt_stream, double dt_noise,
                              cse6230rand_t *rand,
                              double *restrict X[3], const double *restrict U[3])
{
  if (dt_noise) {
    size_t tag;

    tag = cse6230rand_get_tag (rand);
    #pragma omp parallel for
    for (int i = 0; i < Np; i++) {
      double rval[4];

      cse6230rand_hash (rand, tag, i, 0, 0, rval);

      for (int d = 0; d < 3; d++) {
        X[d][i] += dt_stream * U[d][i] + dt_noise * (2. * rval[d] - 1.);

      }
    }
  }
  else {
    #pragma omp parallel for
    for (int i = 0; i < Np; i++) {
      for (int d = 0; d < 3; d++) {
        X[d][i] += dt_stream * U[d][i];

      }
    }
  }
}
