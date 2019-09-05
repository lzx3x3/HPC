
CC = icc
COPTFLAGS = -O -qopt-report=3
OMPFLAGS = -qopenmp
CFLAGS = -g -Wall -fPIC
CPPFLAGS = -I$(CUDAROOT)/include
DEFINES =
CULIBS = -L$(CUDAROOT)/lib64 -Wl,-rpath,$(CUDAROOT)/lib64 -lcudart
LIBS = $(CULIBS)
NVCC = nvcc -ccbin=icpc
NVCCFLAGS = -lineinfo -Xcompiler '-fPIC'
CUOPTFLAGS = -O
RM = rm -f
LINKER = icpc $(OMPFLAGS)

all: stream cloud

%.o: %.c
	$(CC) -std=c99 $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $(DEFINES) $(OMPFLAGS) -c -o $@ $<

%.link.o: %.o
	$(NVCC) -dlink -o $@ $< -lcudadevrt

%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(CUOPTFLAGS) -dc -o $@ $<

verlet.o: verlet.c verlet.h cloud.h

cloud.o: cloud.c verlet.h cloud.h

cloud: verlet.o cloud.o
	$(LINKER) -o $@ $^ -Wl,-rpath,.

PERF =
STREAM_N = 1000000
runstream:
	$(CC) $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $(OMPFLAGS) -o stream stream.c -DSTREAM_ARRAY_SIZE=$(STREAM_N)
	$(PERF) ./stream

runstream2:
	$(CC) $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $(OMPFLAGS) -o stream2 stream2.c -DSTREAM_ARRAY_SIZE=$(STREAM_N)
	$(PERF) ./stream2

runstreamcu:
	$(NVCC) $(NVCCFLAGS) $(CUOPTFLAGS) -o streamcu stream.cu -DSTREAM_ARRAY_SIZE=$(STREAM_N)
	$(PERF) ./streamcu

runstreamcu2:
	$(NVCC) $(NVCCFLAGS) $(CUOPTFLAGS) -o streamcu2 stream2.cu -DSTREAM_ARRAY_SIZE=$(STREAM_N)
	$(PERF) ./streamcu2

NPOINT=64
DT=0.01
NT=1000
NCHUNK=1

runcloud:
	$(RM) cloud cloud.o verlet.o
	$(MAKE) verlet.o DEFINES="-DNT=$(NCHUNK)"
	$(MAKE) cloud
	$(PERF) ./cloud $(NPOINT) $(NT) $(DT) $(NCHUNK)

clean:
	$(RM) *.o cloud stream stream2 streamcu streamcu2

.PHONY: clean runstream runstream2 runstreamcu runcloud
