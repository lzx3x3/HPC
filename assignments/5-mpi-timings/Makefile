
OMPFLAGS = -fopenmp
MPICC    = mpicc $(OMPFLAGS)
CFLAGS   = -g -Wall -O3 -std=c99
CPPFLAGS = -I../../utils
LIBS     =
NODEFILEFLAG = -f ${PBS_NODEFILE}

MPIRUN    = mpirun
N         = ${PBS_NP}
MPIRUNCMD = $(MPIRUN)

RM       = rm -f

all: benchmarks

%.o: %.c
	$(MPICC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

benchmarks: benchmarks.o
	$(MPICC) -o $@ $^ $(LIBS)

runbenchmarks: benchmarks
	$(MPIRUNCMD) $(NODEFILEFLAG) -n $(N) ./benchmarks  
    # xvfb-run python figgen.py
    
clean:
	$(RM) *.o benchmarks

.PHONY: clean runbenchmarks
