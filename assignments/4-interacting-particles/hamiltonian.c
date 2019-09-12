
#include "cloud.h"

double
compute_hamiltonian (int Np, double k, const double *X[3], const double *U[3])
{
  double h = 0.;

  for (int i = 0; i < Np; i++) { /* for every particle */
    h += 0.5 * (U[0][i]*U[0][i] + U[1][i]*U[1][i] + U[2][i]*U[2][i]); /* kinetic energy */
    if (k) {
      for (int j = i + 1; j < Np; j++) { /* for every other particle */
        double hj = potential (k, X[0][i], X[1][i], X[2][i], X[0][j], X[1][j], X[2][j]);

        h += hj;
      }
    }
  }
  return h;
}

