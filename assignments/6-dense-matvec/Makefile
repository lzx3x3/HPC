
MPICC    = mpicc
MPIRUN   = mpirun
N        = 4
CFLAGS   = -g -Wall -O3 -std=c99
CPPFLAGS = -I../../utils/Random123/include -I../../utils
LIBS     = -lm
RM       = rm -f
ARGS     =
VALGRIND =
RUNENV   =

all: dense_matrix_vector_product


dmv_source.c := $(wildcard dmv*.c)
$(wildcard *.c): dmv.h
$(dmv_source.c): dmv_impl.h

%.o: %.c
	$(MPICC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

dense_matrix_vector_product: dense_matrix_vector_product.o $(dmv_source.c:%.c=%.o)
	$(MPICC) -o $@ $^ $(LIBS)

clean:
	$(RM) *.o dense_matrix_vector_product

ifeq ($(VALGRIND),1)
	RUNENV += valgrind
endif

run: dense_matrix_vector_product
	$(MPIRUN) -np $(N) $(RUNENV) $(abspath $<) $(ARGS)

print:
	@echo $($(VAR))

.PHONY: clean $(examples.run)

