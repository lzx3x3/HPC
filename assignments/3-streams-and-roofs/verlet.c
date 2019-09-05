#include <math.h>
#include "cloud.h"
#include "verlet.h"


#if !defined(NT)
#define NT Nt
#endif

void
verlet_step (int Np, int Nt, real_t dt,
             real_t *restrict x, real_t *restrict y, real_t *restrict z,
             real_t *restrict u, real_t *restrict v, real_t *restrict w)
{
  real_t hdt = 0.5 * dt;

  for (int i = 0; i < Np; i++) {
    real_t x_t = x[i];
    real_t y_t = y[i];
    real_t z_t = z[i];
    real_t u_t = u[i];
    real_t v_t = v[i];
    real_t w_t = w[i];

    for (int j = 0; j < NT; j++) {
      real_t r2_t, r_t, ir3_t, ax_t_dt, ay_t_dt, az_t_dt;

      /* half update position using existing velocity and acceleration */
      x_t += hdt * u_t;
      y_t += hdt * v_t;
      z_t += hdt * w_t;

      /* compute midpoint acceleration */
      r2_t = x_t * x_t + y_t * y_t + z_t * z_t;
      r_t = sqrt (r2_t);
      ir3_t = 1. / (r2_t * r_t);
      ax_t_dt = -x_t * ir3_t;
      ay_t_dt = -y_t * ir3_t;
      az_t_dt = -z_t * ir3_t;

      /* update velocity using midpoint acceleration */
      u_t += dt * ax_t_dt;
      v_t += dt * ay_t_dt;
      w_t += dt * az_t_dt;

      /* half update position using existing velocity and acceleration */
      x_t += hdt * u_t;
      y_t += hdt * v_t;
      z_t += hdt * w_t;
    }

    x[i] = x_t;
    y[i] = y_t;
    z[i] = z_t;
    u[i] = u_t;
    v[i] = v_t;
    w[i] = w_t;
  }
}

