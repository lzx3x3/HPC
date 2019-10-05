/* From Section 3.1 of the MPI-3.1 Manual */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MPI_CHECK(err) if (err != MPI_SUCCESS) return err

int main(int argc, char **argv)
{
  int        err;
  int        myrank;

  err = MPI_Init( &argc, &argv );
  MPI_CHECK(err);
  err = MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_CHECK(err);
  if (myrank == 0)
    /* code for process zero */
  {
    char       message[20];
    strcpy(message,"Hello, there");
    err = MPI_Send(message, strlen(message)+1, MPI_CHAR, 1, 99, MPI_COMM_WORLD);
    MPI_CHECK(err);
  }
  else if (myrank == 1) /* code for process one */
  {
    char *message;
    int count;
    MPI_Status pstatus, status;

    err = MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &pstatus);
    MPI_CHECK(err);
    err = MPI_Get_count(&pstatus,MPI_CHAR,&count);
    MPI_CHECK(err);
    message = (char *) malloc(count * sizeof (*message));
    err = MPI_Recv(message, count, MPI_CHAR, pstatus.MPI_SOURCE, pstatus.MPI_TAG, MPI_COMM_WORLD, &status);
    MPI_CHECK(err);
    err = MPI_Get_count(&status,MPI_CHAR,&count);
    MPI_CHECK(err);
    printf("received :%s:\n", message);
    printf("status: from %d, tag %d, error %d\n", status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);
    printf("message length: %d\n", count);
    free(message);
  }
  err = MPI_Finalize();
  return err;
}
