/*******************************************************************************
*   Copyright(C) 2012-2014 Intel Corporation. All Rights Reserved.
*   
*   The source code, information  and  material ("Material") contained herein is
*   owned  by Intel Corporation or its suppliers or licensors, and title to such
*   Material remains  with Intel Corporation  or its suppliers or licensors. The
*   Material  contains proprietary information  of  Intel or  its  suppliers and
*   licensors. The  Material is protected by worldwide copyright laws and treaty
*   provisions. No  part  of  the  Material  may  be  used,  copied, reproduced,
*   modified, published, uploaded, posted, transmitted, distributed or disclosed
*   in any way  without Intel's  prior  express written  permission. No  license
*   under  any patent, copyright  or  other intellectual property rights  in the
*   Material  is  granted  to  or  conferred  upon  you,  either  expressly,  by
*   implication, inducement,  estoppel or  otherwise.  Any  license  under  such
*   intellectual  property  rights must  be express  and  approved  by  Intel in
*   writing.
*   
*   *Third Party trademarks are the property of their respective owners.
*   
*   Unless otherwise  agreed  by Intel  in writing, you may not remove  or alter
*   this  notice or  any other notice embedded  in Materials by Intel or Intel's
*   suppliers or licensors in any way.
*
********************************************************************************/

/*******************************************************************************
*   This example demonstrates threading impact on computing real matrix product 
*   C=alpha*A*B+beta*C using Intel(R) MKL subroutine DGEMM, where A, B, and C 
*   are matrices and alpha and beta are double precision scalars. 
*
*   In this simple example, practices such as memory management, data alignment, 
*   and I/O that are necessary for good programming style and high MKL 
*   performance are omitted to improve readability.
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "mkl.h"

/* Consider adjusting LOOP_COUNT based on the performance of your computer */
/* to make sure that total run time is at least 1 second */
#define LOOP_COUNT 212

int main(int argc, char **argv)
{
    double *A, *B, *C;
    int m, n, p, i, j, r, max_threads;
    double alpha, beta;
    double s_initial, s_elapsed;
    long int flops = 0;
    double perf = 0;
    double time_sum = 0;
    int count = 0;

    //printf ("Freq: %f\n", mkl_get_max_cpu_frequency());
    m = atoi(argv[1]);
    p = atoi(argv[2]);
    n = atoi(argv[3]);
    printf("m: %d, p: %d, n: %d\n", m, p, n);
    flops = 2*(long int)m*(long int)n*(long int)p;

    alpha = 1.0; beta = 0.0;
    
    A = (double *)mkl_malloc( m*p*sizeof( double ), 64 );
    B = (double *)mkl_malloc( p*n*sizeof( double ), 64 );
    C = (double *)mkl_malloc( m*n*sizeof( double ), 64 );
    if (A == NULL || B == NULL || C == NULL) {
        printf( "\n ERROR: Can't allocate memory for matrices. Aborting... \n\n");
        mkl_free(A);
        mkl_free(B);
        mkl_free(C);
        return 1;
    }

    for (i = 0; i < (m*p); i++) {
        A[i] = (double)(i+1);
    }

    for (i = 0; i < (p*n); i++) {
        B[i] = (double)(-i-1);
    }

    for (i = 0; i < (m*n); i++) {
        C[i] = 0.0;
    }

        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                    m, n, p, alpha, A, p, B, n, beta, C, n);
        
#if 0
        s_initial = dsecnd();
        for (r = 0; r < LOOP_COUNT; r++) {
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                        m, n, p, alpha, A, p, B, n, beta, C, n);
	}
	s_elapsed = (dsecnd() - s_initial) / LOOP_COUNT;
	perf = flops/(s_elapsed*1.0e9);
        printf("LOOP_COUNT: %d\n", LOOP_COUNT);
	printf("time: %f sec, perf: %f\n", s_elapsed, perf);
#endif
	while ( time_sum < 1.0 ) {
	    s_initial = dsecnd();
            cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                        m, n, p, alpha, A, p, B, n, beta, C, n);
	    s_elapsed = (dsecnd() - s_initial);
	    time_sum += s_elapsed;
	    count ++;
        }
	time_sum /= count;
	perf = flops/(time_sum*1.0e9);
	printf("count: %d\n", count);
	printf("time: %.3e sec, perf: %.2f\n", time_sum, perf);
	time_sum = 0;
	count = 0;
    
    mkl_free(A);
    mkl_free(B);
    mkl_free(C);
    
    return 0;
}
