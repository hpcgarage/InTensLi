BLIS_PREFIX = /home/jli/Software
BLIS_INC    = -I$(BLIS_PREFIX)/include/blis
BLIS_LIB    = $(BLIS_PREFIX)/lib/libblis.a
MKL_PREFIX = /opt/intel/composerxe
MKL_INC    = -I$(MKL_PREFIX)/mkl/include
MKL_LIB    = -L$(MKL_PREFIX)/lib -L$(MKL_PREFIX)/mkl/lib -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -lmkl_rt -liomp5

OTHER_LIBS  = -L/usr/lib -lm -openmp

#CXX          = g++
#CXXFLAGS      = -O2 -g -I$(BLIS_INC) -fopenmp
CXX          = icc
CXXFLAGS      = -O3 -I./  $(BLIS_INC) $(MKL_INC) -openmp #-g -opt-report
LINKER      = $(CXX)

OBJSGEN     = driver-gen.o gen_ttm.o decide_mc.o util.o timer.o
OBJSRUN     = driver-ttm.o ttm.o decide_mc.o util.o timer.o

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

gen: $(OBJSGEN)
	$(LINKER) $(OBJSGEN) $(BLIS_LIB) $(MKL_LIB) $(OTHER_LIBS) -o gen_ttm

ttm: $(OBJSRUN)
	$(LINKER) $(OBJSRUN) $(BLIS_LIB) $(MKL_LIB) $(OTHER_LIBS) -o ttm

clean:
	-rm *.o gen_ttm ttm
