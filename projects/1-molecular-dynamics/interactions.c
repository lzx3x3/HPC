#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interactions.h"
#include "cloud_util.h"
#include "steric.h"

typedef struct _box
{
    int head;
}
box;

struct _ix
{
  double L;
  double r;
  int boxdim;
  box ***boxes;
  int maxNx;
  int curNx;
  ix_pair *pairs;
};

// it is possible to use smaller boxes and more complex neighbor patterns
#define NUM_BOX_NEIGHBORS 13
int box_neighbors[NUM_BOX_NEIGHBORS][3] =
{
    {-1,-1,-1},
    {-1,-1, 0},
    {-1,-1,+1},
    {-1, 0,-1},
    {-1, 0, 0},
    {-1, 0,+1},
    {-1,+1,-1},
    {-1,+1, 0},
    {-1,+1,+1},
    { 0,-1,-1},
    { 0,-1, 0},
    { 0,-1,+1},
    { 0, 0,-1}
};

/* maxNx: _predicted_ maximum number of interactions */
int
IXCreate (double L, int boxdim, int maxNx, IX *ix_p)
{
  IX ix;
  int err;
  boxdim = 8;//I added this line
  if (boxdim < 4) /* need at least four boxes in each direction */
  {
    boxdim = 4;
  }
  err = safeMALLOC(sizeof (*ix), &ix);CHK(err);

  ix->L = L;
  ix->boxdim = boxdim;
  ix->curNx = 0;
  ix->maxNx = maxNx;
  err = safeMALLOC(boxdim * sizeof (box **), &(ix->boxes));CHK(err);
  for (int i = 0; i < boxdim; i++) {
    err = safeMALLOC(boxdim * sizeof (box *), &(ix->boxes[i]));CHK(err);
    for (int j = 0; j < boxdim; j++) {
      err = safeMALLOC(boxdim * sizeof (box), &(ix->boxes[i][j]));CHK(err);
    }
  }
  err = safeMALLOC(maxNx * sizeof (ix_pair), &(ix->pairs));CHK(err);
  *ix_p = ix;
  return (0);
}

int
IXDestroy (IX *ix_p)
{
  int boxdim = (*ix_p)->boxdim;

  free ((*ix_p)->pairs);
  #pragma omp parallel for //I changed this line
  for (int i = 0; i < boxdim; i++) {
    for (int j = 0; j < boxdim; j++) {
      free((*ix_p)->boxes[i][j]);
    }
    free((*ix_p)->boxes[i]);
  }
  free((*ix_p)->boxes);
  free(*ix_p);
  *ix_p = NULL;
  return 0;
}

static void
IXClearPairs(IX ix)
{
  ix->curNx = 0;
}

static void
IXPushPair(IX ix, int p1, int p2)
{
  ix_pair *pair;
  if (ix->curNx == ix->maxNx) {
    int maxNx = ix->maxNx * 2;
    ix_pair *newpairs;

    safeMALLOC(maxNx * sizeof(ix_pair), &newpairs);
    memcpy(newpairs, ix->pairs, ix->curNx * sizeof (ix_pair));
    free(ix->pairs);
    ix->pairs = newpairs;
    ix->maxNx = maxNx;
  }
  pair = &(ix->pairs[ix->curNx++]);
  pair->p[0] = p1;
  pair->p[1] = p2;
}

int
interactions_check(IX ix, Vector X, double r, int Npairs, ix_pair *pairs, int *total)
{
  double L = ix->L;
  double r2 = r * r;
  int Np = X->Np;
  int intcount = 0;

  for (int i = 0; i < Np; i++) {
    for (int j = i + 1; j < Np; j++) {
      double dx, dy, dz;

      double dist2 = dist_and_disp (IDX(X,0,i),IDX(X,1,i),IDX(X,2,i),
                                    IDX(X,0,j),IDX(X,1,j),IDX(X,2,j), L,
                                    &dx, &dy, &dz);
     
      if (dist2 < r2) {
        intcount++;
        int k;
        for (k = 0; k < Npairs; k++) {
          if ((pairs[k].p[0] == i && pairs[k].p[1] == j) ||
              (pairs[k].p[0] == j && pairs[k].p[1] == i)) {
            break;
          }
        }
        if (k == Npairs) {
          fprintf(stderr,"Pair %d %d not in list\n", i, j);
          return 1;
        }
      }
    }
  }
  *total = intcount;
  return 0;

}

int
IXGetPairs(IX ix, Vector X, double r, int *Npairs, ix_pair **pairs)
{
  int    boxdim = ix->boxdim;
  double L = ix->L;
  double boxwidth = L / boxdim;
  double cutoff2 = r * r;
  box    ***b = ix->boxes;
  int Np = X->Np;
  int err;

  if (r > boxwidth)
  {
    printf("interactions: radius %g is greater than box width %g\n", r, boxwidth);
    return 1;
  }

  // box indices
  int idx, idy, idz;
  int neigh_idx, neigh_idy, neigh_idz;
  int *next;
  box *bp, *neigh_bp;
  #pragma omp parallel for// I added this line
  for (int i = 0; i < boxdim; i++) {
    for (int j = 0; j < boxdim; j++) {
      for (int k = 0; k < boxdim; k++) {
        b[i][j][k].head = -1;
      }
    }
  }

  err = safeMALLOC(Np * sizeof(int), &next);CHK(err);

  // traverse all particles and assign to boxes
  #pragma omp parallel for default(shared) private (idx,idy,idz,bp)//I added this line, private variables idx,idy,idz
  for (int i=0; i<Np; i++)
  {
    double pos_p[3];
    // initialize entry of implied linked list
    next[i] = -1;

    // get the periodic coordinates in [0,L)
    for (int j = 0; j < 3; j++) {
      //printf("%g\n",IDX(X,j,i));
      pos_p[j] = remainder(IDX(X,j,i),L) + L/2.;
    }
    // which box does the particle belong to?
    idx = (int)(pos_p[0]/L*boxdim);
    idy = (int)(pos_p[1]/L*boxdim);
    idz = (int)(pos_p[2]/L*boxdim);

    // add to beginning of implied linked list
    bp = &b[idx][idy][idz];
    #pragma omp critical // I added this line, to protect for updating
   {
    next[i] = bp->head;
    bp->head = i;}
  }

  int p1, p2;
  double d2, dx, dy, dz;
  
  IXClearPairs(ix);
  #pragma omp parallel for collapse(3) default(shared) private (p1,p2,d2,dx,dy,dz,idx,idy,idz,bp,neigh_idx,neigh_idy,neigh_idz,neigh_bp)
  for (idx=0; idx<boxdim; idx++)
  {
    for (idy=0; idy<boxdim; idy++)
    {
      for (idz=0; idz<boxdim; idz++)
      {
        bp = &b[idx][idy][idz];
     
  
        // within box interactions
        
        p1 = bp->head;
        while (p1 != -1)
        {
         
          p2 = next[p1];
          while (p2 != -1)
          {
            
            d2 = dist_and_disp(IDX(X,0,p1),IDX(X,1,p1),IDX(X,2,p1),
                               IDX(X,0,p2),IDX(X,1,p2),IDX(X,2,p2), L,
                               &dx, &dy, &dz);

            if (d2 < cutoff2)
            {
              #pragma omp critical //I added this line to protect updating
              IXPushPair(ix,p1,p2);
            }

            p2 = next[p2];
          }
          p1 = next[p1];
        }
    
        // interactions with other boxes
        for (int j=0; j<NUM_BOX_NEIGHBORS; j++)
        {
          neigh_idx = (idx + box_neighbors[j][0] + boxdim) % boxdim;
          neigh_idy = (idy + box_neighbors[j][1] + boxdim) % boxdim;
          neigh_idz = (idz + box_neighbors[j][2] + boxdim) % boxdim;

          neigh_bp = &b[neigh_idx][neigh_idy][neigh_idz];
          
          p1 = neigh_bp->head;
          while (p1 != -1)
          {
            
            p2 = bp->head;
            while (p2 != -1)
            {
              d2 = dist_and_disp(IDX(X,0,p1),IDX(X,1,p1),IDX(X,2,p1),
                                 IDX(X,0,p2),IDX(X,1,p2),IDX(X,2,p2), L,
                                 &dx, &dy, &dz);

              if (d2 < cutoff2)
              {
                #pragma omp critical //I added this line to protect updating
                IXPushPair(ix,p1,p2);
              }

              p2 = next[p2];
            }
            p1 = next[p1];
          }
        }
      }
    }
  }

  free(next);
 
  *Npairs = ix->curNx;
  *pairs = ix->pairs;

#if DEBUG
  {
    int NpairsCheck;

    err = interactions_check(ix,X,r,*Npairs,*pairs,&NpairsCheck);CHK(err);

    printf("Npairs %d, NpairsCheck %d\n", *Npairs, NpairsCheck);

    if (*Npairs != NpairsCheck) {
      fprintf (stderr, "Interaction count mismatch, %d != %d\n", *Npairs, NpairsCheck);
    }
  }
#endif

  return 0;
}

int
IXRestorePairs(IX ix, Vector X, double r, int *Npairs, ix_pair **pairs)
{
  *Npairs = 0;
  *pairs = NULL;
  return 0;
}
