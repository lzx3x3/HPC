#define BENCHMARK "OSU MPI%s Latency Test"
/*
 * Copyright (C) 2002-2019 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University. 
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */
#include <osu_util_mpi.h>

int
main (int argc, char *argv[])
{
    int myid, numprocs, i;
    int sendid = 0, recvid = 1;
    int size;
    MPI_Status reqstat;
    char *s_buf, *r_buf;
    double t_start = 0.0, t_end = 0.0;
    int po_ret = 0;
    options.bench = PT2PT;
    options.subtype = LAT;
    MPI_Comm comm = MPI_COMM_WORLD;

    set_header(HEADER);
    set_benchmark_name("osu_latency");

    if (argc > 2) {
      recvid = atoi(argv[--argc]);
      sendid = atoi(argv[--argc]);
    }

    po_ret = process_options(argc, argv);

    if (PO_OKAY == po_ret && NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            exit(EXIT_FAILURE);
        }
    }

    MPI_CHECK(MPI_Init(&argc, &argv));
    MPI_CHECK(MPI_Comm_size(MPI_COMM_WORLD, &numprocs));
    MPI_CHECK(MPI_Comm_rank(MPI_COMM_WORLD, &myid));

    if (0 == myid) {
        switch (po_ret) {
            case PO_CUDA_NOT_AVAIL:
                fprintf(stderr, "CUDA support not enabled.  Please recompile "
                        "benchmark with CUDA support.\n");
                break;
            case PO_OPENACC_NOT_AVAIL:
                fprintf(stderr, "OPENACC support not enabled.  Please "
                        "recompile benchmark with OPENACC support.\n");
                break;
            case PO_BAD_USAGE:
                print_bad_usage_message(myid);
                break;
            case PO_HELP_MESSAGE:
                print_help_message(myid);
                break;
            case PO_VERSION_MESSAGE:
                print_version_message(myid);
                MPI_CHECK(MPI_Finalize());
                exit(EXIT_SUCCESS);
            case PO_OKAY:
                break;
        }
    }

    switch (po_ret) {
        case PO_CUDA_NOT_AVAIL:
        case PO_OPENACC_NOT_AVAIL:
        case PO_BAD_USAGE:
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_FAILURE);
        case PO_HELP_MESSAGE:
        case PO_VERSION_MESSAGE:
            MPI_CHECK(MPI_Finalize());
            exit(EXIT_SUCCESS);
        case PO_OKAY:
            break;
    }

    if (!myid) {
      char library[MPI_MAX_LIBRARY_VERSION_STRING] = {0};
      int version, subversion, liblen;
      double time, timeprec;

      MPI_CHECK(MPI_Get_version(&version, &subversion));
      MPI_CHECK(MPI_Get_library_version(library, &liblen));

      printf("MPI Version: %d.%d\n", version, subversion);
      printf("%s\n", library);
      printf("MPI # Procs: %d\n", numprocs);

      time = MPI_Wtime();
      timeprec = MPI_Wtick();

      printf("MPI Wtime %g, precision %g\n", time, timeprec);
      if (MPI_WTIME_IS_GLOBAL) {
        printf("MPI Wtime is global\n");
      } else {
        printf("MPI Wtime is not global\n");
      }
    }
    for (i = 0; i < numprocs; i++) {
      char procname[MPI_MAX_PROCESSOR_NAME] = {0};
      if (i == myid) {
        int namelen;

        MPI_CHECK(MPI_Get_processor_name(procname, &namelen));
        if (i) {
          MPI_CHECK(MPI_Send(procname, namelen, MPI_CHAR, 0, 0, comm));
        }
      }
      if (!myid) {
        if (i) {
          MPI_CHECK(MPI_Recv(procname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, 0, comm, MPI_STATUS_IGNORE));
        }
        printf("MPI proc %d host: %s\n", i, procname);
      }
    }


    if(numprocs < 2) {
        if(myid == 0) {
            fprintf(stderr, "This test requires at least two processes\n");
        }

        MPI_CHECK(MPI_Finalize());
        exit(EXIT_FAILURE);
    }

    if (allocate_memory_pt2pt_sr(&s_buf, &r_buf, myid, sendid, recvid)) {
        /* Error allocating memory */
        MPI_CHECK(MPI_Finalize());
        exit(EXIT_FAILURE);
    }

    print_header(myid, LAT);

    
    /* Latency test */
    for(size = options.min_message_size; size <= options.max_message_size; size = (size ? size * 2 : 1)) {
        set_buffer_pt2pt_sr(s_buf, myid, options.accel, 'a', size, sendid, recvid);
        set_buffer_pt2pt_sr(r_buf, myid, options.accel, 'b', size, sendid, recvid);

        if(size > LARGE_MESSAGE_SIZE) {
            options.iterations = options.iterations_large;
            options.skip = options.skip_large;
        }

        MPI_CHECK(MPI_Barrier(MPI_COMM_WORLD));

        if(myid == sendid) {
            for(i = 0; i < options.iterations + options.skip; i++) {
                if(i == options.skip) {
                    t_start = MPI_Wtime();
                }

                MPI_CHECK(MPI_Send(s_buf, size, MPI_CHAR, recvid, 1, MPI_COMM_WORLD));
                MPI_CHECK(MPI_Recv(r_buf, size, MPI_CHAR, recvid, 1, MPI_COMM_WORLD, &reqstat));
            }

            t_end = MPI_Wtime();
        }

        else if(myid == recvid) {
            for(i = 0; i < options.iterations + options.skip; i++) {
                MPI_CHECK(MPI_Recv(r_buf, size, MPI_CHAR, sendid, 1, MPI_COMM_WORLD, &reqstat));
                MPI_CHECK(MPI_Send(s_buf, size, MPI_CHAR, sendid, 1, MPI_COMM_WORLD));
            }
        }

        if(myid == sendid) {
            double latency = (t_end - t_start) * 1e6 / (2.0 * options.iterations);

            fprintf(stdout, "%-*d%*.*f\n", 10, size, FIELD_WIDTH,
                    FLOAT_PRECISION, latency);
            fflush(stdout);
        }
    }

    free_memory_sr(s_buf, r_buf, myid, sendid, recvid);
    MPI_CHECK(MPI_Finalize());

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}

