#include <string.h>
#include <getopt.h>
#include "dmv.h"
#include "dmv_impl.h"

static int OptionsGetEnum(const dmv_option *options, int numOptions, const char *optstring, int *optionChoice)
{
  int i;

  for (i = 0; i < numOptions; i++) {
    if (!strcmp(options[i].optname, optstring)) {
      break;
    }
  }
  *optionChoice = i;

  return 0;
}

int ArgsCreate(MPI_Comm comm, int argc, char **argv, Args *args_p)
{
  Args     args = NULL;
  int      err, opt, size, rank;

  args = (Args) calloc(1,sizeof(*args));
  if (!args) MPI_CHK(1);

  args->comm = comm;
  args->scale = 20;
  err = MPI_Comm_rank(comm, &rank); MPI_CHK(err);
  while ((opt = getopt(argc, argv, "s:e:g:l:v:d:p:m:h")) != -1) {
    switch (opt) {
    case 's':
      args->scale = atoi(optarg);
      break;
    case 'e':
      args->seed.v[0] = atoi(optarg);
      break;
    case 'g':
      err = OptionsGetEnum(GlobalSizeTypes, GLOBAL_SIZE_NUM_TYPES, optarg, &args->global_size_strategy); MPI_CHK(err);
      break;
    case 'l':
      err = OptionsGetEnum(LayoutTypes, LAYOUT_NUM_TYPES, optarg, &args->layout_strategy); MPI_CHK(err);
      break;
    case 'v':
      args->verbosity = atoi(optarg);
      break;
    case 'd':
      args->debug = atoi(optarg);
      break;
    case 'p':
      err = OptionsGetEnum(PartitionTypes, PARTITION_NUM_TYPES, optarg, &args->partition_strategy); MPI_CHK(err);
      break;
    case 'm':
      err = OptionsGetEnum(MatVecTypes, MATVEC_NUM_TYPES, optarg, &args->matvec_strategy); MPI_CHK(err);
      break;
    case 'h':
      if (!rank) {
        fprintf(stderr, "Usage: %s [-s scale] [-e seed] [-v verbosity] [-d debug]\n", argv[0]);
        fprintf(stderr, "          [-g global_size_strategy] [-l layout_strategy]\n");
        fprintf(stderr, "          [-p matrix_partition_strategy] [-m matvec_strategy]\n\n");
        fprintf(stderr, "global_size_strategy:");
        for (int i = 0; i < GLOBAL_SIZE_NUM_TYPES; i++) {
          if (i) {fprintf(stderr, ", ");}
          fprintf(stderr, "%s", GlobalSizeTypes[i].optname);
        }
        fprintf(stderr, "\nlayout_strategy: ");
        for (int i = 0; i < LAYOUT_NUM_TYPES; i++) {
          if (i) {fprintf(stderr, ", ");}
          fprintf(stderr, "%s", LayoutTypes[i].optname);
        }
        fprintf(stderr, "\nmatrix_partition_strategy: ");
        for (int i = 0; i < PARTITION_NUM_TYPES; i++) {
          if (i) {fprintf(stderr, ", ");}
          fprintf(stderr, "%s", PartitionTypes[i].optname);
        }
        fprintf(stderr, "\nmatvec_strategy: ");
        for (int i = 0; i < MATVEC_NUM_TYPES; i++) {
          if (i) {fprintf(stderr, ", ");}
          fprintf(stderr, "%s", MatVecTypes[i].optname);
        }
      }
      break;
    default:
      break;
    }
  }

  err = MPI_Comm_size(comm, &size); MPI_CHK(err);

  if (args->verbosity) {
    MPI_LOG(comm,
            "Distributed dense matrix-vector product example:\n"
            "    Procs: %d\n    Scale: %d\n    Seed: %lu\n"
            "    Global size: %s\n    Layout: %s\n    Partition: %s\n    Matvec: %s\n",
            size, args->scale, args->seed.v[0],
            GlobalSizeTypes[args->global_size_strategy].optname,
            LayoutTypes[args->layout_strategy].optname,
            PartitionTypes[args->partition_strategy].optname,
            MatVecTypes[args->matvec_strategy].optname);
  }

  args->key = threefry4x64keyinit(args->seed);

  *args_p = args;

  return 0;
}

int ArgsDestroy(Args *args_p)
{
  free(*args_p);
  *args_p = NULL;
  return 0;
}

int ArgsGetMatVecType(Args args, int *type_p)
{
  *type_p = args->matvec_strategy;
  return 0;
}

int ArgsGetPartitionType(Args args, int *type_p)
{
  *type_p = args->partition_strategy;
  return 0;
}
