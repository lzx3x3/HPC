#if !defined(DMVIMPL_H)
#define      DMVIMPL_H

struct _args
{
  MPI_Comm            comm;
  threefry4x64_ukey_t seed;
  threefry4x64_key_t  key;
  int                 global_size_strategy;
  int                 layout_strategy;
  int                 partition_strategy;
  int                 matvec_strategy;
  int                 scale;
  int                 verbosity;
  int                 debug;
};

enum
{
  TAG_TREE_SUBCOMM = 0,
  TAG_TREE_RECURSE,
  TAG_SSEND,
  TAG_ISSEND
};

int DenseMatVec_RowPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int nLocal, const double *vecRightLocal, int mLocal, double *vecLeftLocal);
int DenseMatVec_ColPartition(Args args, int mStart, int mEnd, int nStart, int nEnd, const double *matrixEntries, int nLocal, const double *vecRightLocal, int mLocal, double *vecLeftLocal);

#endif
