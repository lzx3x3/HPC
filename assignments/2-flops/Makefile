
all: fma_prof fma_prof_opt

CC = icc
COPTFLAGS = -O3 -xHost -qopt-report=5
OMPFLAGS = -qopenmp
CFLAGS = -g -Wall -std=c99 -fPIC
CPPFLAGS = -I$(CUDAROOT)/include
CULIBS = -L$(CUDAROOT)/lib64 -Wl,-rpath,$(CUDAROOT)/lib64 -lcudart
LIBS = $(CULIBS)
NVCC = nvcc -ccbin=icpc
NVCCFLAGS = -Xcompiler '-fPIC'
CUOPTFLAGS = 
RM = rm -f
LINKER = icpc $(OMPFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(COPTFLAGS) $(CPPFLAGS) $(OMPFLAGS) -c -o $@ $<

%.link.o: %.o
	$(NVCC) -dlink -o $@ $< -lcudadevrt

%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(CUOPTFLAGS) -dc -o $@ $<

fma_cuda_link.o: fma_cuda.o fma_loop_dev.o
	$(NVCC) $(NVCCFLAGS) -dlink  $^ -o $@

fma_cuda_link_opt.o: fma_cuda.o fma_loop_dev_opt.o
	$(NVCC) $(NVCCFLAGS) -dlink  $^ -o $@

libfma_cuda.so: fma_cuda_link.o fma_cuda.o fma_loop_dev.o
	$(LINKER) -shared -Wl,-soname,libfma_cuda.so -o $@ $^ $(LIBS)

libfma_cuda_opt.so: fma_cuda_link_opt.o fma_cuda.o fma_loop_dev_opt.o
	$(LINKER) -shared -Wl,-soname,libfma_cuda_opt.so -o $@ $^ $(LIBS)

fma_prof: fma_prof.o fma_omp.o fma_loop_host.o libfma_cuda.so
	$(LINKER) -o $@ $^ -Wl,-rpath,.

fma_prof_opt: fma_prof.o fma_omp.o fma_loop_host_opt.o libfma_cuda_opt.so
	$(LINKER) -o $@ $^ -Wl,-rpath,.

Nh = $(shell echo $$(( $(PBS_NP) * 256 )))
Nd = 256
T = 1048576
b = 0.5
c = 3.0
Bs = -1
Gs = -1

PERF =
OMPENV = OMP_PROC_BIND=spread OMP_NUM_THREADS=$(PBS_NP)

run_fma_prof: fma_prof
	$(OMPENV) $(PERF) ./fma_prof $(Nh) $(Nd) $(Bs) $(Gs) $(T) $(b) $(c)

run_fma_prof_opt: fma_prof fma_prof_opt
	$(OMPENV) $(PERF) ./fma_prof_opt $(Nh) $(Nd) $(Bs) $(Gs) $(T) $(b) $(c)

clean:
	$(RM) *.o *.optrpt *.so fma_prof fma_prof_opt

.PHONY: clean run_fma_prof
