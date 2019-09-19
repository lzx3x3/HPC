
MAKERULES=intel
include Makefile.$(MAKERULES)
CPPFLAGS = -I../../utils -I../../utils/tictoc
DEFINES =
RM = rm -f
LIBS = -lm -lrt
LINKER = $(CC)
LINKER = icpc $(OMPFLAGS)
OMPFLAGS = -qopenmp
all: cloud cloud2

%.o: %.c
	$(CC) -std=c99 $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $(DEFINES) $(OMPFLAGS) -c -o $@ $<

%.link.o: %.o
	$(NVCC) -dlink -o $@ $< -lcudadevrt

%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(CUOPTFLAGS) -dc -o $@ $<

verlet.o: verlet.c verlet.h cloud.h

verlet_stream_and_noise.o: verlet_stream_and_noise.c verlet.h cloud.h

verlet_accelerate.o: verlet_accelerate.c verlet.h cloud.h

verlet_accelerate2.o: verlet_accelerate2.c verlet.h cloud.h

cloud.o: cloud.c verlet.h cloud.h

cloud: verlet.o verlet_stream_and_noise.o verlet_accelerate.o cloud.o initialize.o hamiltonian.o
	$(LINKER) -o $@ $^ $(LIBS)

cloud2: verlet.o verlet_stream_and_noise.o verlet_accelerate2.o cloud.o initialize.o hamiltonian.o
	$(LINKER) -o $@ $^ $(LIBS)

PERF =
STREAM_N = 1000000

NP=64
DT=1.e-3
K=-1.0
D=0.1
NT=1000
CHUNK=-1
VIZNAME=cloud

runcloud:
	$(MAKE) --silent clean
	$(MAKE) --silent cloud
	$(PERF) ./cloud $(NP) $(NT) $(DT) $(K) $(D)

runcloud2:
	$(MAKE) --silent clean
	$(MAKE) --silent cloud2
	$(PERF) ./cloud2 $(NP) $(NT) $(DT) $(K) $(D)

vizcloud:
	$(MAKE) --silent clean
	$(MAKE) --silent cloud
	./cloud $(NP) $(NT) $(DT) $(K) $(D) $(CHUNK) $(VIZNAME) | xvfb-run python3 cloud.py

clean:
	$(RM) *.o cloud cloud2

.PHONY: clean runcloud runcloud2 vizcloud
