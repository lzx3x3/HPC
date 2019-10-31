
CSE6230UTILSDIR = ../../utils
MPICC      = mpicc
PICFLAG    = -fpic
OPTFLAGS   = -O3
CFLAGS     = -g -Wall -std=c99
CPPFLAGS   = -I$(CSE6230UTILSDIR)/Random123/include
SHAREDFLAG = -shared
LINKER     = mpicc
THISDIR    = $(dir Makefile)
RM         = rm -f
EXTRALIBS  = -lm

CSOURCES := proj2.c proj2sorter.c local.c bitonic.c quicksort.c
LIBOBJS  :=

all: test_proj2

# If you only have additional C files, you can create a Makefile.inc file
# of the form
#
#     CSOURCES += foo.c bar.c baz.c
#
# If you have C++ or other language files, you will have to add them to the
# list of objects that go into the library like
#
# 		CXXSOURCES := foo.cc bar.cc
# 		LIBOBJS += $(CXXSOURCES:%.cc=%.o)
#
# If you have source files in a language other than C, you will have to write
# your own rule for compiling object files.  Be sure to use the $(PICFLAG) for
# position independent code, because this will compile to a shared library.
-include Makefile.inc

LIBOBJS += $(CSOURCES:%.c=%.o)

# Again: if you want to have C++ (or other language) sources, you will have to
# put your own rule for compiling object files in Makefile.inc.  Be sure to use
# the $(PICFLAG) for postion independent code.
%.o: %.c
	$(MPICC) $(CPPFLAGS) $(CFLAGS) $(PICFLAG) $(OPTFLAGS) -c -o $@ $<

libproj2.so: $(LIBOBJS)
	$(LINKER) $(SHAREDFLAG) -o $@ $^ $(EXTRALIBS)

test_proj2.o: test_proj2.c

test_proj2: test_proj2.o libproj2.so
	$(LINKER) -L$(THISDIR) -Wl,-rpath,$(THISDIR) -o $@ $< -lproj2

clean:
	$(RM) libproj2.so $(LIBOBJS) test_proj2.o test_proj2

.PHONY: clean
