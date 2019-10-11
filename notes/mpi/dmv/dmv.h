/* Distributed dense matrix-vector product examples */
#if !defined(DMV_H)
#define      DMV_H

#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include <Random123/threefry.h>

#define MPI_CHK(err) do {if (err) {fprintf(stderr, "Error: %s, %s, %d\n", __FILE__,__func__,__LINE__); return err;}} while (0)

#define MPI_LOG(comm,...)                        \
  do {                                           \
    int _rank;                                   \
    MPI_Comm_rank(comm,&_rank);                  \
    if (!_rank) {                                \
      int _worldrank;                            \
      MPI_Comm_rank(MPI_COMM_WORLD,&_worldrank); \
      printf("[%d] ",_worldrank);                \
      printf(__VA_ARGS__);                       \
      fflush(stdout);                            \
    }                                            \
  } while(0)

typedef struct _args *Args;

int ArgsCreate(MPI_Comm, int argc, char **argv, Args *args);
int ArgsDestroy(Args *args);
int ArgsGetMatVecType(Args args, int *type_p);
int ArgsGetPartitionType(Args args, int *type_p);

int VecGetOffset(Args args, int nLocal, int *nOffset);
int VecGetEntries(Args args, int rowStart, int rowEnd, double *entries);
int VecGetGlobalSize(Args args, int mLocal, int *mGlobal_p);
int VecGetLayout(Args args, int mLocal, int *localOffsets);

int MatrixGetLocalSize(Args args, int *mLocal_p, int *nLocal_p);
int MatrixGetEntries(Args args, int rowStart, int rowEnd, int colStart, int colEnd, double *entries);

int DenseMatVec(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int nLocal, const double *vecRightLocal, int mLocal, double *vecLeftLocal);

typedef struct _dmv_option {
  const char *optname;
  int optenum;
}
dmv_option;

enum {
  GLOBAL_SIZE_TREE_SUBCOMM = 0,
  GLOBAL_SIZE_TREE_RECURSE,
  GLOBAL_SIZE_REDUCE_BCAST,
  GLOBAL_SIZE_ALLREDUCE,
  GLOBAL_SIZE_NUM_TYPES
};

static const dmv_option GlobalSizeTypes[GLOBAL_SIZE_NUM_TYPES] =
{
  {"tree_subcomm", GLOBAL_SIZE_TREE_SUBCOMM},
  {"tree_recurse", GLOBAL_SIZE_TREE_RECURSE},
  {"reduce_bcast", GLOBAL_SIZE_REDUCE_BCAST},
  {"allreduce",    GLOBAL_SIZE_ALLREDUCE},
};

enum {
  LAYOUT_GATHER = 0,
  LAYOUT_ALLGATHER,
  LAYOUT_SCAN,
  LAYOUT_NUM_TYPES
};

static const dmv_option LayoutTypes[LAYOUT_NUM_TYPES] =
{
  {"gather",       LAYOUT_GATHER},
  {"allgather",    LAYOUT_ALLGATHER},
  {"scan",         LAYOUT_SCAN},
};

enum {
  PARTITION_ROWS = 0,
  PARTITION_COLS,
  PARTITION_NUM_TYPES
};

static const dmv_option PartitionTypes[PARTITION_NUM_TYPES] =
{
  {"rows", PARTITION_ROWS},
  {"cols", PARTITION_COLS},
};

enum {
  MATVEC_SSEND = 0,
  MATVEC_ISSEND,
  MATVEC_ALLGATHERV,
  MATVEC_NUM_TYPES
};

static const dmv_option MatVecTypes[MATVEC_NUM_TYPES] =
{
  {"ssend",      MATVEC_SSEND},
  {"issend",     MATVEC_ISSEND},
  {"allgatherv", MATVEC_ALLGATHERV},
};

#endif
