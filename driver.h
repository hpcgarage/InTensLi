#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "omp.h"

// Different gemm libraries
#include "blis.h"
#include "mkl.h"

#include "timer.h"
#include "defines.h"

//ttm
double* ttm (double *X, const int Dim, long int *XDimSize,
            double *U, long int *UDimSize, char tflag,
            double *Y, long int *YDimSize,
            int ModeComn, int *ModeCom, int ModeComLen);

int decide_component_mode (int Dim, long int* XDimSize, int ModeComn, int strategy, int degree, int *ModeCom, int *ModeComLen, int *KTag);
int decide_paras(int Dim, long int* XDimSize, int R, int ModeComn, int strategy, int degree, int* out_degree, int *out_omp_nt, int *out_mkl_nt);

int gen_ttm (int Dim, long int *XDimSize,
    long int *UDimSize, char tflag,
    long int *YDimSize,
    int ModeComn, int strategy,
    int *ModeCom, int ModeComLen, int KTag,
    int omp_nt, int mkl_nt);

// util
char* get_arg(int argc, char** argv, const char* key);
int print_error ( const char* msg);
int print_array (int *array, int size, const char *msg, FILE *fp);
int print_array (double *array, int size, const char *msg, FILE *fp);
int print_array (char **array, int size, const char *msg, FILE *fp);
int print_array (long int *array, long int size, const char *msg, FILE *fp);
