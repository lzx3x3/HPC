
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define MPI_CHECK(err) if (err != MPI_SUCCESS) return err

int main (int argc, char **argv)
{
  char message[] = "I'll go first.";
  char *sendbuff;
  int  len, packsize, n;
  char buffer[20] = {'\0'};
  int  err;
  int  myrank;

  err = MPI_Init (&argc, &argv);
  MPI_CHECK(err);
  err = MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
  MPI_CHECK(err);
  len = strlen(message);
  err = MPI_Pack_size(len,MPI_CHAR,MPI_COMM_WORLD,&packsize);
  MPI_CHECK(err);
  n = packsize + MPI_BSEND_OVERHEAD;
  sendbuff = (char *) malloc(n);
  if (!sendbuff) return 1;
  err = MPI_Buffer_attach(sendbuff,n);
  MPI_CHECK(err);
  err = MPI_Bsend (message, strlen(message), MPI_CHAR, 1 - myrank, 0, MPI_COMM_WORLD);
  MPI_CHECK(err);
  err = MPI_Recv (buffer, 19, MPI_CHAR, 1 - myrank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_CHECK(err);
  err = MPI_Buffer_detach(&sendbuff,&n);
  MPI_CHECK(err);
  free(sendbuff);

  err = MPI_Finalize();
  return err;
}
