#!/bin/bash

# Set variables

# Compile
echo "icc -mkl dgemm.c -o dgemm"
icc -mkl dgemm.c -o dgemm

for mkl in 1 2 4 6 8 
do  
    export MKL_NUM_THREADS=${mkl}
    echo "MKL_NUM_THREADS=";printenv MKL_NUM_THREADS
    mkdir logs/mkl${MKL_NUM_THREADS}
	for (( m=2;m<=16384;m*=2))
	do
	  for (( n=2;n<=16384;n*=2))
	  do
	    for (( k=2;k<=16384;k*=2))
	    do
		  # Run dgemm
		  echo "./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt"
		  ./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt
	    done
	  done
	done
done

