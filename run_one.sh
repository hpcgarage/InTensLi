#!/bin/bash

# Set enviorment variables
# Define nested loops parallelism
OMPNT=1;             #number of threads for OMP parallel for
echo "OMPNT=${OMPNT}"
# Define MKL parallelism
MKLNT=4;
echo "MKLNT=${MKLNT}"
# Define BLIS parallelism
export BLIS_IR_NT=1
export BLIS_JR_NT=1
export BLIS_IC_NT=1
export BLIS_JC_NT=1

# Set variables
DIM=3;
MODE=2;
MAXDIMSIZE=1500;
MINDIMSIZE=1500;
R=10;
DEGREE=2;
STRATEGY=1;

# Run gen_ttm and ttm
#:<<\eof
echo "make clean";
make clean; 
echo "make gen";
make gen;
# debug
echo "./gen_ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE}\
-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -OMPNT=${OMPNT} -MKLNT=${MKLNT} -mode=${MODE}";
./gen_ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} -R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -OMPNT=${OMPNT} -MKLNT=${MKLNT} -mode=${MODE};
#eof

echo "make ttm";
make ttm;
echo "./ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} \
-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -mode=${MODE}";
./ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} -R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -mode=${MODE};

