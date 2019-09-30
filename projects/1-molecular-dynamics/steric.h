#if !defined(STERIC_H)
#define      STERIC_H

#include <math.h>


/* This kernel should be called if the distance between two particles is less
 * than twice the particle radius */
static inline void
force_in_range (double k, /* The interaction strength (may be scaled by the time step already) */
                double r, /* The radius of a particle.  Two particles interact if they intersect */
                double R, /* The distance between these two particles */
                double dx, double dy, double dz, /* The displacement from particle 2 to particle 1 */
                double f[3]) /* The output force exerted on particle 1 by particle 2 */
{
  /* The interaction strength starts at 0 when they are just touching,
   * becoming infinite as the distance becomes zero */
  double strength = (2. * r - R) / R;

  f[0] = k * strength * dx;
  f[1] = k * strength * dy;
  f[2] = k * strength * dz;
}

/* get the square distance and displacement between two particles under periodic
 * conditions */
static inline double
dist_and_disp (double x1, double y1, double z1, /* The center of the first particle */
               double x2, double y2, double z2, /* The center of the second particle */
               double L, /* The width of the periodic domain */
               double *Dx, double *Dy, double *Dz)
{
  double dx, dy, dz;
  *Dx = dx = remainder(x1 - x2, L);
  *Dy = dy = remainder(y1 - y2, L);
  *Dz = dz = remainder(z1 - z2, L);

  return dx*dx + dy*dy + dz*dz;
}

static inline void
force (double k, /* The interaction strength (may be scaled by the time step already) */
       double r, /* The radius of a particle.  Two particles interact if they intersect */
       double L, /* The width of the periodic domain */
       double x1, double y1, double z1, /* The center of the first particle */
       double x2, double y2, double z2, /* The center of the second particle */
       double f[3]) /* The output force exterted on particle 1 by particle 2 */
{
  double dx, dy, dz;
  double R2;
  double r2 = 4. * r * r;

  R2 = dist_and_disp (x1, y1, z1, x2, y2, z2, L, &dx, &dy, &dz);

  /* If the distance between the centers is less than twice the radius, they
   * interact */
  if (R2 < r2) {
    double R = sqrt(R2);

    //printf("(%g,%g,%g)(%g,%g,%g)\n",x1, y1, z1, x2, y2, z2);

    force_in_range (k, r, R, dx, dy, dz, f);
  }
  else {
    f[0] = f[1] = f[2] = 0.;
  }
}


#endif
