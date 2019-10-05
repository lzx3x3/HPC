
#include <string.h>
#include <mpi.h>

#define MPI_CHECK(err) if (err != MPI_SUCCESS) return err

int main (int argc, char **argv)
{
  char message[] = "I'll go first.";
  char buffer[20] = {'\0'};
  int  err;
  int  myrank;

  err = MPI_Init (&argc, &argv);
  MPI_CHECK(err);
  err = MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
  MPI_CHECK(err);
  err = MPI_Recv (buffer, 19, MPI_CHAR, 1 - myrank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_CHECK(err);
  err = MPI_Send (message, strlen(message)+1, MPI_CHAR, 1 - myrank, 0, MPI_COMM_WORLD);
  MPI_CHECK(err);

  err = MPI_Finalize();
  return err;
}
