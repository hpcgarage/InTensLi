#include "driver.h"

double* ttm (double *X, const int Dim, int *XDimSize,
            double *U, int *UDimSize, char tflag,
            double *Y, int *YDimSize,
            int ModeComn, int *ModeCom, int ModeComLen)
{
	int nthreads, tid;
#pragma omp parallel
{
	tid = omp_get_thread_num();
	if (tid == 0)
	{
		nthreads = omp_get_num_threads();
	}
}

	//Decide transa and transb according to tflag and ModeComn and ModeCom's relationship.
	CBLAS_TRANSPOSE transa = CblasNoTrans;
	CBLAS_TRANSPOSE transb = CblasNoTrans;
	int m,n,k;
	double alpha, beta;
	int rsu,csu,rsx,csx,rsy,csy;
	int XLoopStride = 0, YLoopStride = 0;
	alpha = 1.0;
	beta = 1.0;

  double gemm_time, tmp_time, tmp_STime;
  double gflops;

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

  gemm_time = 0;
#pragma omp parallel for default(shared)
	for (int i0=0;i0<1000;i0++)
	{
		XLoopStride = i0*1000000;
		YLoopStride = i0*10000;
    tmp_STime = bli_clock();
		cblas_dgemm(CblasRowMajor, transa, transb, 
              			m, n, k, 
              			alpha, 
              			U, rsu, 
              			X+XLoopStride, rsx, 
              			beta, 
              			Y+YLoopStride, rsy);
    tmp_time = bli_clock()-tmp_STime;
    gemm_time += tmp_time;
	}
  gemm_time /= 1000;
  gflops = (2*m*n*k)/(gemm_time*1.0e9);
  fprintf(stdout, "Gemm time : %f sec, Perf: %.2f GFLOPS\n", gemm_time, gflops);
	return Y;
}
