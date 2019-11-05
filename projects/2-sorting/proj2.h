#if !defined(PROJ2_H)
#define      PROJ2_H

#include <mpi.h>
#include <stdio.h>

#define PROJ2LOG(comm,...)                      \
  do {                                          \
    int _rank, _globalRank;                     \
    MPI_Comm_rank(comm,&_rank);                 \
    MPI_Comm_rank(MPI_COMM_WORLD,&_globalRank); \
    if (!_rank) {                               \
      printf("[%d] ",_globalRank);              \
      printf(__VA_ARGS__);                      \
      fflush(stdout);                           \
    }                                           \
  } while(0)

#define PROJ2ERR(comm,err,...)                                            \
  do {                                                                    \
    int _rank, _globalRank;                                               \
    MPI_Comm_rank(comm,&_rank);                                           \
    MPI_Comm_rank(MPI_COMM_WORLD,&_globalRank);                           \
    if (!_rank) {                                                         \
      printf("[%d, %s, %s, %d] ",_globalRank,__FILE__,__func__,__LINE__); \
      printf(__VA_ARGS__);                                                \
      fflush(stdout);                                                     \
    }                                                                     \
    return (err);                                                         \
  } while(0)

#define PROJ2CHK(err) if (err) PROJ2ERR(MPI_COMM_WORLD,err,"Function returned error\n")

#define MPI_CHK(err) if (err) PROJ2ERR(MPI_COMM_WORLD,err,"MPI error")

int Proj2Malloc(size_t size, void **array);
int Proj2Free(void **array);

#define PROJ2MALLOC(count,a) Proj2Malloc((count)*sizeof(**(a)),(void **)(a))
#define PROJ2FREE(a) Proj2Free((void **)(a))

#endif

