
CC = icc
OMPFLAGS = -qopenmp
# I think for Intel compilers this is the flag you want
# OMPFLAGS = -openmp
CPPFLAGS = -I../../utils/tictoc
CFLAGS = -g -O -Wall
LIBS =
RM = rm -f
PANDOC = pandoc

allexamples := $(basename $(wildcard *.c))

all: $(allexamples)

%: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(OMPFLAGS) -o $@ $< $(LIBS)

clean:
	$(RM) $(allexamples)

NPMAX=8
N = 1000
runex15: openmp-ex15
	for nt in `seq 1 $(NPMAX)`; do \
		OMP_NUM_THREADS=$$nt ./openmp-ex15 $N; \
	done

runex16: openmp-ex16
	for nt in `seq 1 $(NPMAX)`; do \
		OMP_NUM_THREADS=$$nt ./openmp-ex16 $N; \
	done

.PHONY:
	clean runex15 runex16


