#if !defined(CSE6230RAND_H)
#define      CSE6230RAND_H

/* Wrappers to Random123 */

#include <stdint.h>
#include <math.h>
#include <Random123/include/Random123/threefry.h>

#if defined(M_PI)
#define CSE6230_PI M_PI
#else
#define CSE6230_PI 3.1415926535897932384626433832795L
#endif

typedef struct _cse6230rand
{
  threefry4x64_ctr_t c;
  threefry4x64_key_t k;
  threefry4x64_ctr_t r;
  size_t count;
  size_t tag;
}
cse6230rand_t;

static inline void cse6230rand_seed(int seed, cse6230rand_t *restrict rand)
{
  threefry4x64_key_t uk = {{0}};

  uk.v[0] = seed;
  rand->k = threefry4x64keyinit(uk);
  rand->c.v[0] = 0;
  rand->c.v[1] = 1;
  rand->c.v[2] = 2;
  rand->c.v[3] = 3;
  rand->r = threefry4x64(rand->c,rand->k);
  rand->count = 0;
  rand->tag = 0;
}

static inline double cse6230rand(cse6230rand_t *restrict rand)
{
  const double scale = 1. / (UINT64_MAX + 1.);
  const double shift = scale / 2.;
  size_t       mod   = (rand->count++) % 4;
  double       ret;

  ret = rand->r.v[mod] * scale + shift;

  if (mod == 3) {
    for (int i = 0; i < 4; i++) {rand->c.v[i] += 4;}
    rand->r = threefry4x64(rand->c,rand->k);
  }

  return ret;
}

static inline size_t cse6230rand_get_tag(cse6230rand_t *restrict rand)
{
  return rand->tag++;
}

/* gives four random numbers for four indices */
static inline void cse6230rand_hash(cse6230rand_t *restrict rand,
                                    size_t tag,
                                    size_t index1,
                                    size_t index2,
                                    size_t index3,
                                    double rand_out[])
{
  const double scale = 1. / (UINT64_MAX + 1.);
  const double shift = scale / 2.;
  threefry4x64_ctr_t c;
  threefry4x64_ctr_t r;
  int i;

  c.v[0] = tag;
  c.v[1] = index1;
  c.v[2] = index2;
  c.v[3] = index3;

  r = threefry4x64(c, rand->k);
  for (i = 0; i < 4; i++) {
    rand_out[i] = r.v[i] * scale + shift;
  }
}

/* gives three normally distributed random numbers from four indices */
static inline void cse6230rand_normal_hash(cse6230rand_t *restrict rand,
                                           size_t tag,
                                           size_t index1,
                                           size_t index2,
                                           size_t index3,
                                           double rand_out[])
{
  const double scale = 1. / (UINT64_MAX + 1.);
  const double shift = scale / 2.;
  threefry4x64_ctr_t c;
  threefry4x64_ctr_t r;

  c.v[0] = tag;
  c.v[1] = index1;
  c.v[2] = index2;
  c.v[3] = index3;

  r = threefry4x64(c, rand->k);
  for (int i = 0; i < 4; i += 2) {
    double u1 = r.v[i] * scale + shift;
    double u2 = r.v[i+1] * scale + shift;
    double z1 = sqrt(-2. * log(u1)) * cos(CSE6230_PI * 2. * u2);
    double z2 = sqrt(-2. * log(u1)) * sin(CSE6230_PI * 2. * u2);
    rand_out[i] = z1;
    rand_out[i+1] = z2;
  }
}

typedef struct _cse6230nrand
{
  cse6230rand_t urand;
  double        z[4];
  size_t        count;
}
cse6230nrand_t;

static inline void _cse6230nrand_batch(cse6230nrand_t *restrict nrand)
{
  const double scale = 1. / (UINT64_MAX + 1.);
  const double shift = scale / 2.;

  for (int i = 0; i < 4; i++) {nrand->urand.c.v[i] += 4;}
  nrand->urand.r = threefry4x64(nrand->urand.c,nrand->urand.k);
  for (int i = 0; i < 4; i++) {nrand->z[i] = nrand->urand.r.v[i] * scale + shift;}
  nrand->urand.count += 4;

  for (int i = 0; i < 4; i += 2) {
    double u1 = nrand->z[i];
    double u2 = nrand->z[i+1];
    double z1 = sqrt(-2. * log(u1)) * cos(CSE6230_PI * 2. * u2);
    double z2 = sqrt(-2. * log(u1)) * sin(CSE6230_PI * 2. * u2);
    nrand->z[i]   = z1;
    nrand->z[i+1] = z2;
  }
}

static inline void cse6230nrand_seed(int seed, cse6230nrand_t *restrict nrand)
{
  cse6230rand_seed(seed,&(nrand->urand));
  _cse6230nrand_batch(nrand);
  nrand->count = 0;
}

static inline double cse6230nrand(cse6230nrand_t *restrict nrand)
{
  size_t       mod   = (nrand->count++) % 4;
  double       ret;

  ret = nrand->z[mod];

  if (mod == 3) {
    _cse6230nrand_batch(nrand);
  }

  return ret;
}

#endif
