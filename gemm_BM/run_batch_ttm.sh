#!/bin/bash

# Compile
echo "icc -mkl dgemm.c -o dgemm"
icc -mkl dgemm.c -o dgemm

# Set variables
R=10;

for mkl in 1 2 4 6 8 
do  
    export MKL_NUM_THREADS=${mkl}
    echo "MKL_NUM_THREADS=";printenv MKL_NUM_THREADS
    mkdir logs_ttm/mkl${MKL_NUM_THREADS}
	for ((d=3;d<=5;d++))
	do
	  DIM=${d};
	  if [ "${DIM}" -eq "3" ]
	  then
	    tmpmin=500;
	    tmpmax=1400;
	    tmpgap=100;
	  fi
	  if [ "${DIM}" -eq "4" ]
	  then
	    tmpmin=100;
	    tmpmax=240;
	    tmpgap=20;
	  fi
	  if [ "${DIM}" -eq "5" ]
	  then
	    tmpmin=30;
	    tmpmax=80;
	    tmpgap=10;
	  fi
	  for ((i=${tmpmin};i<=${tmpmax};i+=${tmpgap}))
	  do
	        SIZE=${i}
		echo "dim: ${DIM}"
		  m=${R};
		  k=${SIZE};
		  n=$(( ${SIZE}**(${DIM}-1) ));
		  # Run dgemm
		  echo "./dgemm ${m} ${k} ${n} >> logs_ttm/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt"
		  ./dgemm ${m} ${k} ${n} >> logs_ttm/mkl${MKL_NUM_THREADS}/perf_${m}_${k}_${n}.txt
	  done
	done
done

