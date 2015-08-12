#!/bin/sh

cd gemm_BM;
echo `pwd`;
./run_batch.sh
echo "Run gemm perf log: ./run_batch.sh";
./gen_THs.pl;
echo "./gen_THs.pl";
cd ..;
echo `pwd`;

make clean; make gen;
echo "Compile: make clean; make gen";
./gen_ttm;
echo "Running ./gen_ttm";
make ttm;
echo "Compile: make ttm";
./ttm;
echo "Running ./ttm";
