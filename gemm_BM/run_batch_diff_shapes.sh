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
	for (( m=16;m<=1048576;m*=4))
	do
	  for (( n=16;n<=1024;n*=4))
	  do
	    for (( k=16;k<=1024;k*=4))
	    do
		  # Run dgemm
		  echo "./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt"
		  ./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt
	    done
	  done
	done

	for (( m=16;m<=1024;m*=4))
	do
	  for (( n=16;n<=1048576;n*=4))
	  do
	    for (( k=16;k<=1024;k*=4))
	    do
		  # Run dgemm
		  echo "./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt"
		  ./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt
	    done
	  done
	done

	for (( m=16;m<=1024;m*=4))
	do
	  for (( n=16;n<=1024;n*=4))
	  do
	    for (( k=16;k<=1048576;k*=4))
	    do
		  # Run dgemm
		  echo "./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt"
		  ./dgemm ${m} ${k} ${n} >> logs/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt
	    done
	  done
	done
done

