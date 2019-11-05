#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MPI_CHK(err) if (err != MPI_SUCCESS) return err

#define MPI_LOG(rank,...) if (!rank) {printf(__VA_ARGS__);fflush(stdout);}

int splitCommunicator(MPI_Comm comm, int firstCommSize, MPI_Comm *subComm_p)
{
  /* TODO: split the communicator `comm` into one communicator for ranks
   * [0, ..., firstCommSize - 1] and one for [firstCommSize, ..., size - 1],
   * where `size` is the size of `comm`.
   *
   * Look at the way subcommunicators are created using a coloring in
   * `VecGetGlobalSize_tree_subcomm()` in the file `dmv_global_size.c` in the
   * `notes/mpi/dmv` example.
   */
  int err, rank, color;
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  color = (rank >= firstCommSize);
  err = MPI_Comm_split(comm, color, rank, subComm_p); MPI_CHK(err);
  return 0;
}

int destroyCommunicator(MPI_Comm *subComm_p)
{
  /* TODO: destroy the subcommunicator created in `splitCommunicator` */
  int err = MPI_Comm_free(subComm_p);
  MPI_CHK(err);
  return 0;
}

int startTime(double *tic_p)
{
  /* TODO: Record the MPI walltime in `tic_p` */
  *tic_p = MPI_Wtime();
  return 0;
}

int stopTime(double tic_in, double *toc_p)
{
  /* TODO: Get the elapsed MPI walltime since `tic_in`,
   * write the results in `toc_p` */
  double t = MPI_Wtime();
  *toc_p = t - tic_in;
  return 0;
}

int maxTime(MPI_Comm comm, double myTime, double *maxTime_p)
{
  /* TODO: take the times from all processes and compute the maximum,
   * storing the result on process 0 */
  int err = MPI_Reduce(&myTime, maxTime_p, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
  MPI_CHK(err);
  return 0;
}

int main(int argc, char **argv)
{
  MPI_Comm comm;
  int      err;
  int      size, rank;
  int      maxSize = 1 << 22;
  int      maxCollectiveSize;
  int      numTests = 1000;
  int      numSkip = 100;
  char     *buffer;
  char     *buffer2;

  err = MPI_Init(&argc, &argv); MPI_CHK(err);

  comm = MPI_COMM_WORLD;

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);

  maxCollectiveSize = maxSize / size;

  /* Print out some info about the environment */
  if (!rank) {
    char library[MPI_MAX_LIBRARY_VERSION_STRING] = {0};
    int version, subversion, liblen;
    double time, timeprec;

    err = MPI_Get_version(&version, &subversion); MPI_CHK(err);
    err = MPI_Get_library_version(library, &liblen); MPI_CHK(err);

    printf("MPI Version: %d.%d\n", version, subversion);
    printf("%s\n", library);
    printf("MPI # Procs: %d\n", size);

    time = MPI_Wtime();
    timeprec = MPI_Wtick();

    printf("MPI Wtime %g, precision %g\n", time, timeprec);
    if (MPI_WTIME_IS_GLOBAL) {
      printf("MPI Wtime is global\n");
    } else {
      printf("MPI Wtime is not global\n");
    }
  }
  for (int i = 0; i < size; i++) {
    char procname[MPI_MAX_PROCESSOR_NAME] = {0};
    if (i == rank) {
      int namelen;

      err = MPI_Get_processor_name(procname, &namelen); MPI_CHK(err);
      if (i) {
        err = MPI_Send(procname, namelen, MPI_CHAR, 0, 0, comm); MPI_CHK(err);
      }
    }
    if (!rank) {
      if (i) {
        err = MPI_Recv(procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, 0, comm, MPI_STATUS_IGNORE); MPI_CHK(err);
      }
      printf("MPI proc %d host: %s\n", i, procname);
    }
  }

  buffer = (char *) calloc(maxSize, sizeof(char));
  if (!buffer) return 1;
  buffer2 = (char *) calloc(maxCollectiveSize, sizeof(char));
  if (!buffer2) return 1;

  /* === POINT-TO-POINT PING-PONG TIMINGS === */
  MPI_LOG(rank, "MPI Point-to-point ping-pong test:\n"
          " # Processes  | Message Size | Total Size   | Time         | B/s\n");
  for (int numComm = 14; numComm <= size; numComm *= 2) {
    MPI_Comm subComm = MPI_COMM_NULL;
    err = splitCommunicator(comm, numComm, &subComm); 
    MPI_CHK(err);
    
    for (int numBytes = 8; numBytes <= maxSize; numBytes *= 8) 
    {
      double tic = -1;
<<<<<<< HEAD
      double timeAvg = 0.;
      long long int totalNumBytes = numBytes * numComm; 
      //double tic = -1;//
      for (int t = 0; t < numTests + numSkip; t++) 
      {
        //double tic = -1.;//
        if (t == numSkip) 
        {
          err = startTime(&tic); 
          MPI_CHK(err);
=======

      for (int t = 0; t < numTests + numSkip; t++) {
        if (t == numSkip) {
          err = startTime(&tic); MPI_CHK(err);
>>>>>>> 0ea20d4143a4070cedb7f6b029a3aaeab2b37542
        }
        if (rank < numComm) {
          if (rank < numComm / 2) {
            err = MPI_Send(buffer, numBytes, MPI_BYTE, rank + numComm / 2, 0, subComm); MPI_CHK(err);
            err = MPI_Recv(buffer, numBytes, MPI_BYTE, rank + numComm / 2, 0, subComm, MPI_STATUS_IGNORE); 
              MPI_CHK(err);
          } else {
            err = MPI_Recv (buffer, numBytes, MPI_BYTE, rank - numComm / 2, 0, subComm, MPI_STATUS_IGNORE); MPI_CHK(err);
            err = MPI_Send (buffer, numBytes, MPI_BYTE, rank - numComm / 2, 0, subComm); MPI_CHK(err);
          }
        }
      }
      err = stopTime(tic, &timeAvg); MPI_CHK(err);
      timeAvg /= numTests;
      err = maxTime(subComm, timeAvg, &timeAvg); MPI_CHK(err);
      MPI_LOG(rank, " %12d   %12d   %12lld   %+12.5e   %+12.5e\n", numComm, numBytes, totalNumBytes, timeAvg, totalNumBytes / timeAvg);
    }
  }

  /* NOTE: Whenever one of the following calls for a reduction operation, use
   * MPI_BXOR, the bitwise x-or operation */
  /* NOTE: In all of your tests, make sure you skip `numSkip` iterations
   * before you start timing, as in the point-to-point ping-pong example */

  /* === BROADCAST PING-PONG === */
  MPI_LOG(rank, "MPI Broadcast ping-pong test:\n"
          " # Processes  | Message Size | Total Size   | Time         | B/s\n");
  for (int numComm = 14; numComm <= size; numComm *= 2) {
    MPI_Comm subComm = MPI_COMM_NULL;

    err = splitCommunicator(MPI_COMM_WORLD, numComm, &subComm); MPI_CHK(err);
    for (int numBytes = 8; numBytes <= maxSize; numBytes *= 8) {
      double timeAvg = 0.;
      long long int totalNumBytes = numBytes * (numComm - 1) * 2;

      /* TODO: Set up a ping pong test for the broadcast collective.  When
       * you broadcast the 'ping' message, use the subComm communicator to
       * broadcast from rank 0 the first `numBytes` bytes of the `buffer` to
       * the other subComm processes. Store the results in the first
       * `numBytes` bytes of the `buffer` on the receiving processes.  Use
       * the collective with the reverse communication pattern of broadcast
       * for the 'pong' message.
       * (HINT: look up the proper usage of MPI_IN_PLACE) */
      double tic = -1;
      char* tmp_buffer;
      tmp_buffer = (char *)calloc(totalNumBytes, sizeof(char));  // ???
      if (!tmp_buffer)
        return 1;
      for (int t = 0; t < numTests + numSkip; t++)
      {
        if (t == numSkip)
        {
          err = startTime(&tic);
          MPI_CHK(err);
        }
        if (!rank)
        {
          err = MPI_Bcast(buffer, numBytes, MPI_BYTE, 0, subComm);
          MPI_CHK(err);
          err = MPI_Reduce(MPI_IN_PLACE, buffer, numBytes, MPI_BYTE, MPI_BXOR, 0, subComm);
          MPI_CHK(err);
        }
        else
        {
          err = MPI_Bcast(buffer, numBytes, MPI_BYTE, 0, subComm);
          MPI_CHK(err);
          err = MPI_Reduce(buffer, tmp_buffer, numBytes, MPI_BYTE, MPI_BXOR, 0, subComm);
          MPI_CHK(err);
        }
      }
      err = stopTime(tic, &timeAvg);
      MPI_CHK(err);
      timeAvg /= numTests;
      err = maxTime(subComm, timeAvg, &timeAvg);
      MPI_CHK(err);
      MPI_LOG(rank, " %12d   %12d   %12lld   %+12.5e   %+12.5e\n", numComm, numBytes, totalNumBytes, timeAvg, totalNumBytes / timeAvg);
    }
    err = destroyCommunicator(&subComm); MPI_CHK(err);
  }

  /* === SCATTER PING-PONG === */
  MPI_LOG(rank, "MPI Scatter ping-pong test:\n"
          " # Processes  | Message Size | Total Size   | Time         | B/s\n");
  for (int numComm = 14; numComm <= size; numComm *= 2) {
    MPI_Comm subComm = MPI_COMM_NULL;

    err = splitCommunicator(MPI_COMM_WORLD, numComm, &subComm); MPI_CHK(err);
    for (int numBytes = 8; numBytes <= maxCollectiveSize; numBytes *= 8) {
      double        timeAvg = 0.;
      long long int totalNumBytes = numBytes * (numComm - 1) * 2;

      /* TODO: Set up a ping pong test for the scatter collective.  When
       * you broadcast the 'ping' message, use the subComm communicator to
       * scatter from rank 0 the first (`numBytes` x `numComm`) bytes of the `buffer` to
       * the other subComm processes. Store the results in the first
       * `numBytes` bytes of `buffer2` on the receiving processes.  Use
       * the collective with the reverse communication pattern of scatter
       * for the 'pong' message. */
      double tic = -1;
      for (int t = 0; t < numTests + numSkip; t++)
      {
        if (t == numSkip)
        {
          err = startTime(&tic);
          MPI_CHK(err);
        }

        err = MPI_Scatter(buffer, numBytes, MPI_BYTE, buffer2, numBytes, MPI_BYTE, 0, subComm);
        MPI_CHK(err);
        err = MPI_Gather(buffer2, numBytes, MPI_BYTE, buffer, numBytes, MPI_BYTE, 0, subComm);
        MPI_CHK(err);
      }
      err = stopTime(tic, &timeAvg);
      MPI_CHK(err);
      timeAvg /= numTests;
      err = maxTime(subComm, timeAvg, &timeAvg);
      MPI_CHK(err);
      MPI_LOG(rank, " %12d   %12d   %12lld   %+12.5e   %+12.5e\n", numComm, numBytes, totalNumBytes, timeAvg, totalNumBytes / timeAvg);
    }
    err = destroyCommunicator(&subComm); MPI_CHK(err);
  }

  /* === ALL REDUCE: BXOR messages of varying lengths === */
  MPI_LOG(rank, "MPI All-reduce test:\n"
          " # Processes  | Message Size | Total Size   | Time         | B/s\n");
  for (int numComm = 14; numComm <= size; numComm *= 2) {
    MPI_Comm subComm = MPI_COMM_NULL;

    err = splitCommunicator(MPI_COMM_WORLD, numComm, &subComm); MPI_CHK(err);
    for (int numBytes = 8; numBytes <= maxSize; numBytes *= 8) {
      double        timeAvg = 0.;
      long long int totalNumBytes = numBytes * (numComm - 1) * 2;

      /* TODO: Set up a timing loop for the following:
       *
       * Use the subComm communicator to BXOR the first `numBytes`
       * chars of `buffer` from every process and store the results in
       * `buffer` on all processes (HINT: MPI_IN_PLACE again).
       */
      double tic = -1;
      for (int t = 0; t < numTests + numSkip; t++)
      {
        if (t == numSkip)
        {
          err = startTime(&tic);
          MPI_CHK(err);
        }

        err = MPI_Allreduce(MPI_IN_PLACE, buffer, numBytes, MPI_BYTE, MPI_BXOR, subComm);
        MPI_CHK(err);
      }
      err = stopTime(tic, &timeAvg);
      MPI_CHK(err);
      timeAvg /= numTests;
      err = maxTime(subComm, timeAvg, &timeAvg);
      MPI_CHK(err);

      MPI_LOG(rank, " %12d   %12d   %12lld   %+12.5e   %+12.5e\n", numComm, numBytes, totalNumBytes, timeAvg, totalNumBytes / timeAvg);
    }
    err = destroyCommunicator(&subComm); MPI_CHK(err);
  }

  /* === ALL GATHER === */
  MPI_LOG(rank, "MPI All-gather test:\n"
          " # Processes  | Message Size | Total Size   | Time         | B/s\n");
  for (int numComm = 14; numComm <= size; numComm *= 2) {
    MPI_Comm subComm = MPI_COMM_NULL;

    err = splitCommunicator(MPI_COMM_WORLD, numComm, &subComm); MPI_CHK(err);
    for (int numBytes = 8; numBytes <= maxCollectiveSize; numBytes *= 8) {
      double        timeAvg = 0.;
      long long int totalNumBytes = numBytes * (numComm - 1) * numComm;

      /* TODO: Set up a timing loop for the following:
       *
       * Use the subComm communicator to gather the first `numBytes`
       * bytes of `buffer2` from every process and store the results in
       * `buffer`.
       */
      double tic = -1;
      for (int t = 0; t < numTests + numSkip; t++)
      {
        if (t == numSkip)
        {
          err = startTime(&tic);
          MPI_CHK(err);
        }

        err = MPI_Allgather(buffer2, numBytes, MPI_BYTE, buffer, numBytes, MPI_BYTE, subComm);
        MPI_CHK(err);
      }
      err = stopTime(tic, &timeAvg);
      MPI_CHK(err);
      timeAvg /= numTests;
      err = maxTime(subComm, timeAvg, &timeAvg);
      MPI_CHK(err);
      MPI_LOG(rank, " %12d   %12d   %12lld   %+12.5e   %+12.5e\n", numComm, numBytes, totalNumBytes, timeAvg, totalNumBytes / timeAvg);
    }
    err = destroyCommunicator(&subComm); MPI_CHK(err);
  }

  /* === ALL TO ALL === */
  MPI_LOG(rank, "MPI All-to-all test:\n"
          " # Processes  | Message Size | Total Size   | Time         | B/s\n");
  for (int numComm = 14; numComm <= size; numComm *= 2) {
    MPI_Comm subComm = MPI_COMM_NULL;

    err = splitCommunicator(MPI_COMM_WORLD, numComm, &subComm); MPI_CHK(err);
    for (int numBytes = 8; numBytes <= maxCollectiveSize; numBytes *= 8) {
      double        timeAvg = 0.;
      long long int totalNumBytes = numBytes * ((numComm - 1) * (numComm - 1));

      /* TODO: Set up a timing loop for the following:
       *
       * Use the subComm communicator to transpose the first
       * `numComm` * `numBytes` bytes of `buffer` from every process and
       * store the results in `buffer`.  This is another place where
       * MPI_IN_PLACE is relevant.
       */
      double tic = -1;
      for (int t = 0; t < numTests + numSkip; t++)
      {
        if (t == numSkip)
        {
          err = startTime(&tic);
          MPI_CHK(err);
        }

        err = MPI_Alltoall(MPI_IN_PLACE, numBytes, MPI_BYTE, buffer, numBytes, MPI_BYTE, subComm);
        MPI_CHK(err);
      }
      err = stopTime(tic, &timeAvg);
      MPI_CHK(err);
      timeAvg /= numTests;
      err = maxTime(subComm, timeAvg, &timeAvg);
      MPI_CHK(err);
      MPI_LOG(rank, " %12d   %12d   %12lld   %+12.5e   %+12.5e\n", numComm, numBytes, totalNumBytes, timeAvg, totalNumBytes / timeAvg);
    }
    err = destroyCommunicator(&subComm); MPI_CHK(err);
  }

  free (buffer2);
  free (buffer);
  err = MPI_Finalize();
  return err;
}
