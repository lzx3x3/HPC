#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include "proj2.h"
#include "proj2sorter.h"
#include <Random123/threefry.h>

static int Proj2IsSorted(MPI_Comm comm, size_t numKeysLocal, const uint64_t *keys, int *isSorted_p)
{
  int isSortedLocal = 1;
  int isSorted = 1;
  uint64_t prev_max, next_min;
  int rank, size;
  MPI_Request recv_max = MPI_REQUEST_NULL, recv_min = MPI_REQUEST_NULL;
  int err;

  for (size_t i = 0; i < numKeysLocal - 1; i++) {
    if (keys[i] > keys[i+1]) {
      isSortedLocal = 0;
      break;
    }
  }
  err = MPI_Allreduce(&isSortedLocal, &isSorted, 1, MPI_INT, MPI_LAND, comm); PROJ2CHK(err);
  if (!isSorted) {
    *isSorted_p = 0;
    return 0;
  }
  err = MPI_Comm_size(comm, &size); PROJ2CHK(err);
  err = MPI_Comm_rank(comm, &rank); PROJ2CHK(err);
  if (rank) {
    err = MPI_Irecv(&prev_max, 1, MPI_UINT64_T, rank - 1, 0, comm, &recv_max); PROJ2CHK(err);
  }
  if (rank < size - 1) {
    err = MPI_Irecv(&next_min, 1, MPI_UINT64_T, rank + 1, 0, comm, &recv_min); PROJ2CHK(err);
  }
  if (rank) {
    err = MPI_Send(&keys[0], 1, MPI_UINT64_T, rank - 1, 0, comm); PROJ2CHK(err);
  }
  if (rank < size - 1) {
    err = MPI_Send(&keys[numKeysLocal-1], 1, MPI_UINT64_T, rank + 1, 0, comm); PROJ2CHK(err);
  }
  if (rank) {
    err = MPI_Wait(&recv_max,MPI_STATUS_IGNORE); PROJ2CHK(err);
    if (keys[0] < prev_max) isSortedLocal = 0;
  }
  if (rank < size - 1) {
    err = MPI_Wait(&recv_min,MPI_STATUS_IGNORE); PROJ2CHK(err);
    if (keys[numKeysLocal-1] > next_min) isSortedLocal = 0;
  }
  err = MPI_Allreduce(&isSortedLocal, &isSorted, 1, MPI_INT, MPI_LAND, comm); PROJ2CHK(err);
  *isSorted_p = isSorted;

  return 0;
}

static int testSorter(MPI_Comm comm, Proj2Sorter sorter, int numTests, size_t numKeysLocal, threefry4x64_key_t *tfkey, threefry4x64_ctr_t *ctr, int uniform_size,
                      int uniform_keys, int partially_sorted, double *avgBW_p)
{
  uint64_t *keys, *keysCopy;
  uint64_t offset = 0;
  uint64_t numKeysLocal_l = numKeysLocal;
  uint64_t numKeysGlobal = 0;
  size_t   totalB;
  double   avgBW = 0.;
  int      err, rank;
  uint64_t my_offset = 0;
  uint64_t my_range = 1;

  err = MPI_Exscan(&numKeysLocal_l, &offset, 1, MPI_UINT64_T, MPI_SUM, comm); PROJ2CHK(err);
  err = MPI_Allreduce(&numKeysLocal_l, &numKeysGlobal, 1, MPI_UINT64_T, MPI_SUM, comm); PROJ2CHK(err);
  err = MPI_Comm_rank(comm, &rank); PROJ2CHK(err);

  totalB = numKeysGlobal * sizeof(*keys);

  PROJ2LOG(comm, "Testing numKeysLocal %zu, numKeysGlobal %lu, total bytes %zu\n", numKeysLocal, numKeysGlobal, totalB);

  err = Proj2SorterGetWorkArray(sorter, numKeysLocal, sizeof(*keys), &keys); PROJ2CHK(err);
  err = Proj2SorterGetWorkArray(sorter, numKeysLocal, sizeof(*keys), &keysCopy); PROJ2CHK(err);
  ctr->v[0] = numKeysGlobal;
  /* create a random array */
  ctr->v[1] = 0;
  if (!uniform_keys) {
    threefry4x64_ctr_t my_ctr = {{0}};
    threefry4x64_ctr_t rand;
    static int calls = 0;

    my_ctr.v[0] = numKeysLocal;
    my_ctr.v[1] = rank;
    my_ctr.v[2] = ++calls;
    rand = threefry4x64(*tfkey,my_ctr);
    my_offset = rand.v[0] / 2;
    my_range = rand.v[1] / 2;
#if defined(DEBUG)
    PROJ2LOG(MPI_COMM_SELF, "My range: [%zu, %zu)\n", my_offset, my_offset + my_range);
#endif
  }
  if (!partially_sorted) {
    for (uint64_t idx = (offset / 4) * 4; idx < offset + numKeysLocal; idx += 4) {
      threefry4x64_ctr_t rand;

      ctr->v[2] = idx;
      rand = threefry4x64(*tfkey,*ctr);
      for (int j = 0; j < 4; j++) {
        size_t k = idx + j;

        if (k >= offset && k < offset + numKeysLocal) {
          keysCopy[k - offset] = uniform_keys ? rand.v[j] : ((rand.v[j] % my_range) + my_offset);
        }
      }
    }
  } else {
    threefry4x64_ctr_t rand;
    uint64_t last;
    static int calls = 0;

    ctr->v[2] = ++calls;
    rand = threefry4x64(*tfkey,*ctr);
    last = rand.v[0];
    for (uint64_t idx = 0; idx < numKeysGlobal; idx += 4) {
      threefry4x64_ctr_t rand;

      ctr->v[2] = idx;
      rand = threefry4x64(*tfkey,*ctr);
      for (int j = 0; j < 4; j++) {
        size_t k = idx + j;
        int step = (int) (rand.v[j] % 4);
        size_t val;

        step--;
        if (partially_sorted < 0) step = -step;
        if (step < 0) {
          val = last + (UINT64_MAX + step);
        } else {
          val = last + step;
        }

        if (k >= offset && k < offset + numKeysLocal) {
          keysCopy[k - offset] = uniform_keys ? val : ((val % my_range) + my_offset);
#if defined(DEBUG)
          PROJ2LOG(comm, "[%zu] %zu\n", k - offset, keysCopy[k - offset]);
#endif
        }
        last = val;
      }
    }
  }
  for (int i = 0; i < numTests; i++) {
    int    isSorted = 1;
    double tic, toc;

    memcpy(keys,keysCopy,numKeysLocal*sizeof(*keys));
    err = MPI_Barrier(comm); PROJ2CHK(err);
    tic = MPI_Wtime();
    err = Proj2SorterSort(sorter, numKeysLocal, uniform_size, keys); PROJ2CHK(err);
    toc = MPI_Wtime();
    err = Proj2IsSorted(comm, numKeysLocal, keys, &isSorted); PROJ2CHK(err);
    if (!isSorted) PROJ2ERR(comm, 4, "Array not sorted: numKeysLocal %zu, testNum %d\n", numKeysLocal, i);
    if (i) {
      avgBW += totalB / (toc - tic);
    }
  }
  avgBW /= (numTests - 1);
  PROJ2LOG(comm, "Tested numKeysLocal %zu, numKeysGlobal %lu, total bytes %zu: average bandwidth %e\n", numKeysLocal, numKeysGlobal, totalB, avgBW);
  *avgBW_p = avgBW;
  err = Proj2SorterRestoreWorkArray(sorter, numKeysLocal, sizeof(*keys), &keysCopy); PROJ2CHK(err);
  err = Proj2SorterRestoreWorkArray(sorter, numKeysLocal, sizeof(*keys), &keys); PROJ2CHK(err);
  return 0;
}

int main(int argc, char **argv)
{
  Proj2Sorter sorter;
  int                 minKeys, maxKeys, mult, seed;
  threefry4x64_ukey_t ukey = {{0}};
  threefry4x64_key_t  tfkey;
  threefry4x64_ctr_t  ctr = {{0}};
  int                 err;
  int                 numTests = 0;
  double              harmAvg = 0.;
  int                 numReps = 2;
  int                 uniform_size = 1;
  int                 uniform_keys = 1;
  int                 partially_sorted = 0;
  int                 rank;

  err = MPI_Init(&argc,&argv); if (err) return err;
  if (argc != 9) {
    fprintf(stderr, "Usage: %s MIN_KEYS_PER_PROCESS MAX_KEYS_PER_PROCESS MULTIPLIER SEED NUM_REPS UNIFORM_SIZE UNIFORM_KEYS PARTIALLY_SORTED\n", argv[0]);
    return 1;
  }
  minKeys          = atoi(argv[1]);
  maxKeys          = atoi(argv[2]);
  mult             = atoi(argv[3]);
  seed             = atoi(argv[4]);
  numReps          = atoi(argv[5]);
  uniform_size     = atoi(argv[6]);
  uniform_keys     = atoi(argv[7]);
  partially_sorted = atoi(argv[8]);

  if (mult < 2) mult = 2;
  if (numReps < 2) numReps = 2;

  PROJ2LOG(MPI_COMM_WORLD, "%s minKeys %d maxKeys %d mult %d seed %d uniform size %d uniform distribution %d partially sorted %d\n", argv[0], minKeys, maxKeys, mult, seed, uniform_size, uniform_keys, partially_sorted);

  /* seed the random number generator */
  ukey.v[0] = seed;
  tfkey = threefry4x64keyinit(ukey);
  err = Proj2SorterCreate(MPI_COMM_WORLD, &sorter); PROJ2CHK(err);
  err = MPI_Comm_rank(MPI_COMM_WORLD, &rank); PROJ2CHK(err);
  for (size_t numKeysLocal = minKeys; numKeysLocal <= maxKeys; numKeysLocal *= mult, numTests++) {

    double avgBW = 0.;
    size_t num_added_keys = 0;

    if (!uniform_size) {
      threefry4x64_ctr_t rand;

      ctr.v[0] = numKeysLocal;
      ctr.v[1] = rank;
      rand = threefry4x64(tfkey,ctr);
      num_added_keys = rand.v[0] % numKeysLocal;
    }

    err = testSorter(MPI_COMM_WORLD, sorter, numReps, numKeysLocal + num_added_keys, &tfkey, &ctr, uniform_size, uniform_keys, partially_sorted, &avgBW); PROJ2CHK(err);
    harmAvg += 1. / avgBW;
  }
  harmAvg = numTests / harmAvg;
  PROJ2LOG(MPI_COMM_WORLD, "Harmonic average bandwidth: %e\n", harmAvg);
  err = Proj2SorterDestroy(&sorter); PROJ2CHK(err);
  err = MPI_Finalize();
  return err;
}