#!/bin/bash

# Set variables
R=10;
STRATEGY=1; #Need to change

# Set enviorment variables
for omp in 1 2 4 6 8
do
	# Define nested loops parallelism
#export OMP_NUM_THREADS=${omp}
#	echo "OMP_NUM_THREADS=";printenv OMP_NUM_THREADS
  NT=${omp};
	for mkl in 1 2 4 6 8 
	do  
	    export MKL_NUM_THREADS=${mkl}
	    echo "MKL_NUM_THREADS=";printenv MKL_NUM_THREADS
	    mkdir logs/omp${NT}_mkl${MKL_NUM_THREADS}

	done	#End of MKL_NUM_THREADS
done	#End of NT 

