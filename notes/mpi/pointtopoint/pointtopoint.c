/* From Section 3.1 of the MPI-3.1 Manual */
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>

#define MPI_CHECK(err) if (err != MPI_SUCCESS) return err

int main(int argc, char **argv)
{
  char       message[20];
  int        err;
  int        myrank;

  err = MPI_Init( &argc, &argv );
  MPI_CHECK(err);
  err = MPI_Comm_rank( MPI_COMM_WORLD, &myrank );
  MPI_CHECK(err);
  if (myrank == 0)
    /* code for process zero */
  {
    strcpy(message,"Hello, there");
    err = MPI_Send(message, strlen(message)+1, MPI_CHAR, 1, 99, MPI_COMM_WORLD);
    MPI_CHECK(err);
    memset(message,0,sizeof(message));
  }
  else if (myrank == 1) /* code for process one */
  {
    int count;
    MPI_Status status;

    sleep(1);
    err = MPI_Recv(message, 20, MPI_CHAR, 0, 99, MPI_COMM_WORLD, &status);
    MPI_CHECK(err);
    err = MPI_Get_count(&status,MPI_CHAR,&count);
    MPI_CHECK(err);
    printf("received :%s:\n", message);
    printf("status: from %d, tag %d, error %d\n", status.MPI_SOURCE, status.MPI_TAG, status.MPI_ERROR);
    printf("message length: %d\n", count);
  }
  err = MPI_Finalize();
  return err;
}
