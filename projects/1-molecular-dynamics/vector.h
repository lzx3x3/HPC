#if !defined(VECTOR_H)
#define      VECTOR_H

typedef struct _vector *Vector;

int VectorCreate(int Np, Vector *vec);
int VectorDestroy(Vector *vec);


#if defined(VECTOR_AOS)
/* implement a vector as an array of structures */

#define IDX(a,i,j) (a->v[j][i])

struct _vector
{
  int Np;
  double (*v)[3];
};

#else
/* implement a vector as a structure of arrays */

#define IDX(a,i,j) (a->v[i][j])

struct _vector
{
  int Np;
  double *v[3];
};

#endif

#endif
