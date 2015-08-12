#include "driver.h"

double* ttm (double *X, const int Dim, long int *XDimSize,
            double *U, long int *UDimSize, char tflag,
            double *Y, long int *YDimSize,
            int ModeComn, int *ModeCom, int ModeComLen)
{
	//Decide transa and transb according to tflag and ModeComn and ModeCom's relationship.
	CBLAS_TRANSPOSE transa = CblasNoTrans;
	CBLAS_TRANSPOSE transb = CblasNoTrans;
	long int m,n,k;
	double alpha, beta;
	long int rsu,csu,rsx,csx,rsy,csy;
	long int XLoopStride = 0, YLoopStride = 0;
	alpha = 1.0;
	beta = 1.0;

	/****Generated TM algo according to different modes and dimensions ***/
	m = 10;
	n = 1000;
	k = 1000;
	rsu = 1000;
	csu = 1;
	rsx = 1000;
	csx = 1;
	rsy = 1000;
	csy = 1;

#pragma omp parallel for default(shared) schedule(static) num_threads(1)
	for (long int i0=0;i0<1000;i0++)
	{
		mkl_set_dynamic(0);
		mkl_set_num_threads_local(8);
		XLoopStride = i0*1000000;
		YLoopStride = i0*10000;
		cblas_dgemm(CblasRowMajor, transa, transb, 
              			m, n, k, 
              			alpha, 
              			U, rsu, 
              			X+XLoopStride, rsx, 
              			beta, 
              			Y+YLoopStride, rsy);
	}
	return Y;
}
