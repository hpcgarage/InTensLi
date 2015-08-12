#!/bin/bash

# Set variables
R=10;
STRATEGY=1; #Need to change

for ((d=3;d<=5;d++))
do
  DIM=${d};
# Set the maximum sizes according to MEMSIZE
  if [ ${DIM} -eq 3 ]
  then
    tmpmin=500;
    tmpmax=1500;
    tmpgap=100;
  fi
  if [ ${DIM} -eq 4 ]
  then
    tmpmin=100;
    tmpmax=240;
    tmpgap=20;
  fi
  if [ ${DIM} -eq 5 ]
  then
    tmpmin=30;
    tmpmax=80;
    tmpgap=10;
  fi
  for ((i=${tmpmin};i<=${tmpmax};i+=${tmpgap}))
  do
    MAXDIMSIZE=${i}
    MINDIMSIZE=${i}
    for ((j=1;j<=${DIM};j++))
    do
      MODE=${j};
      echo ${MODE}

# Set enviorment variables
      for omp in 1 2 4 6 8
      do
        # Define nested loops parallelism
        OMPNT=${omp};
        for mkl in 1 2 4 6 8 
        do  
          MKLNT=${mkl};
#mkdir logs/omp${OMPNT}_mkl${MKLNT}
	  if [ ${MODE} -eq ${DIM} ]
	  then
          for (( deg=1;deg<${MODE};deg++ )) #Need to change when strategy != 1
          do
            DEGREE=${deg};

		# Run gen_ttm and ttm
		echo "make clean";
		make clean; 
		echo "make gen";
		make gen;
		echo "./gen_ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE}\
		-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -OMPNT=${OMPNT} -MKLNT=${MKLNT} -mode=${MODE}";
		./gen_ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} -R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -OMPNT=${OMPNT} -MKLNT=${MKLNT} -mode=${MODE};
		echo "make ttm";
		make ttm;
		echo "./ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} \
		-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -mode=${MODE}\
		>	logs/omp${OMPNT}_mkl${MKLNT}/log_d${DIM}_${MAXDIMSIZE}_${MINDIMSIZE}_m${MODE}_deg${DEGREE}.txt";
		./ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE}	-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -mode=${MODE} >	logs/omp${OMPNT}_mkl${MKLNT}/log_d${DIM}_${MAXDIMSIZE}_${MINDIMSIZE}_m${MODE}_deg${DEGREE}.txt;

          done  #End of DEGREE
	  else
            DEGREE=`expr ${DIM} - ${MODE}`;

		# Run gen_ttm and ttm
		echo "make clean";
		make clean; 
		echo "make gen";
		make gen;
		echo "./gen_ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE}\
		-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -OMPNT=${OMPNT} -MKLNT=${MKLNT} -mode=${MODE}";
		./gen_ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} -R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -OMPNT=${OMPNT} -MKLNT=${MKLNT} -mode=${MODE};
		echo "make ttm";
		make ttm;
		echo "./ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE} \
		-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -mode=${MODE}\
		>	logs/omp${OMPNT}_mkl${MKLNT}/log_d${DIM}_${MAXDIMSIZE}_${MINDIMSIZE}_m${MODE}_deg${DEGREE}.txt";
		./ttm -dim=${DIM} -maxdimsize=${MAXDIMSIZE} -mindimsize=${MINDIMSIZE}	-R=${R} -strategy=${STRATEGY} -degree=${DEGREE} -mode=${MODE} >	logs/omp${OMPNT}_mkl${MKLNT}/log_d${DIM}_${MAXDIMSIZE}_${MINDIMSIZE}_m${MODE}_deg${DEGREE}.txt;
	fi

        done	#End of MKLNT 
      done	#End of OMPNT 
    done	#End of MODE
  done	#End of MAXDIMSIZE and MINDIMSIZE
done	#End of DIM

