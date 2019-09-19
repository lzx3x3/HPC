
MAKERULES=intel
include Makefile.$(MAKERULES)
CPPFLAGS = -I../../utils -I../../utils/tictoc
DEFINES =
RM = rm -f
LIBS = -lm -lrt
LINKER = $(CC) $(OMPFLAGS)

all: cloud

%.o: %.c
	$(CC) -std=c99 $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $(DEFINES) $(OMPFLAGS) -c -o $@ $<

%.link.o: %.o
	$(NVCC) -dlink -o $@ $< -lcudadevrt

%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(CUOPTFLAGS) -dc -o $@ $<

cloud: verlet.o accelerate.o cloud.o initialize.o vector.o cloud_util.o interactions.o
	$(LINKER) -o $@ $^ $(LIBS)

PERF =
STREAM_N = 1000000

NP=256
DT=1.e-4
K=100.
D=1.
L=20.
R=1.
NT=1000
CHUNK=1000
VIZNAME=cloud

runcloud:
	$(MAKE) --silent clean
	$(MAKE) --silent cloud
	$(PERF) ./cloud $(NP) $(NT) $(DT) $(K) $(D) $(L) $(R)

vizcloud:
	$(MAKE) --silent clean
	$(MAKE) --silent cloud
	./cloud $(NP) $(NT) $(DT) $(K) $(D) $(L) $(R) $(CHUNK) $(VIZNAME) | xvfb-run python3 viz.py

checkcloud:
	$(MAKE) --silent clean
	$(MAKE) --silent cloud
	./cloud $(NP) $(NT) $(DT) $(K) $(D) $(L) $(R) $(CHUNK) check | python3 check.py

clean:
	$(RM) *.o cloud

.PHONY: clean runcloud vizcloud checkcloud
