
CC=mpicc
CFLAGS=-g -Wall
COPTFLAGS=-O3
OMPFLAGS=

%.o: %.c
	$(CC) $(OMPFLAGS) $(CFLAGS) $(COPTFLAGS) -I./ -c $<

osu_latency: osu_util.o osu_util_mpi.o osu_latency.o
	$(CC) $(OMPFLAGS) $^ -o $@

clean:
	rm -f *.o osu_latency

.PHONY: clean
