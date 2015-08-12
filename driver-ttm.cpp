#include "driver.h"

// Function definitions
int usage ( int argc, char** argv);

int main (int argc, char** argv)
{
  if (get_arg(argc, argv, "help") != NULL) {
    usage(argc, argv);
    return 0;
  }

  // Temporary variables
  char* input_str = NULL;

  // OMP
#if 0
  int nthreads, tid;
#pragma omp parallel
  {
    tid = omp_get_thread_num();
    if (tid == 0)
    {
      nthreads = omp_get_num_threads();
      printf("nthreads: %d\n", nthreads);
    }
  }
#endif

  // Variables with meanings
  double MemSize = MEMSIZE;
  char *XFile = NULL;
  FILE *fp_in;
  int Dim = 3;
  long int XMaxDimSize = 100, XMinDimSize = 100;
  long int XDimSizeGap = 0;
  long int *XDimSize;  //Tensor X's dimension sizes
  long int XSize = 1;  //Tensor X's total size
  double XMemRatio = 0; //Memory ratio X occupies
  double *X;  //Input tensor
  int Moden = 2;  //Mode-n is in [1,Dim]
  int ModeComn = 0; //Moden-1, in [0,Dim-1]
  int *ModeCom;  //The component modes list (ptr), different from BMLAS_2D
  int ModeComLen = 0;  //The number of component modes
  int KTag = 0; //KTag=1,Use U*X kernel; KTag=2, Use X*U' kernel.
  char tflag = 'n'; //If input matrix U needs to be transposed
  int strategy = 1; //Decide component modes, forward strategy (1) or backward strategy (2)
  int degree = 0; //Maximum number of modes considered as component modes
  int R = 10; //Default: 10
  //int R = 2;  //For debug
  long int UDimSize[2];  //Matrix U's dimension sizes
  long int USize = 1;  //Matrix U's total size
  double UMemRatio = 0; //Memory ratio U occupies  
  double *U;  //Input matrix
  long int *YDimSize;  //Tensor Y's dimension sizes
  long int YSize = 1;  //Tensor Y's total size
  double YMemRatio = 0; //Memory ratio Y occupies
  double *Y;  //Output result tensor Y
  long int TotalSize = 0; //Counting X, U, and Y
  double TotalMemRatio = 0; //Memory ratio of total size
  int res = 0;
  FILE *fp_out = fopen("out.txt","w");  //For debug

  //Timing
  double ttm_STime, ttm_time;
  double gflops = 0;

  input_str = (char*)malloc(sizeof(char)*100);
  XFile = (char*)malloc(sizeof(char)*100);

  input_str = get_arg (argc, argv, "mode");
  if (input_str != NULL )
    Moden = atoi(input_str);
  input_str = get_arg (argc, argv, "tflag");
  if (input_str != NULL )
    tflag = input_str[0];
  input_str = get_arg (argc, argv, "R");
  if (input_str != NULL )
    R = atoi(input_str);
  input_str = get_arg (argc, argv, "strategy");
  if (input_str != NULL )
    strategy = atoi(input_str);
  input_str = get_arg (argc, argv, "degree");
  if (input_str != NULL )
    degree = atoi(input_str);
  XFile = get_arg (argc, argv, "file");
  if (XFile == NULL )
  {
    //Generate tensor X 
    input_str = get_arg ( argc, argv, "dim");
    if (input_str != NULL )
      Dim = atoi(input_str);
    input_str = get_arg (argc, argv, "mindimsize");
    if (input_str != NULL )
      XMinDimSize = atoi(input_str);
    input_str = get_arg (argc, argv, "maxdimsize");
    if (input_str != NULL )
      XMaxDimSize = atoi(input_str);

    // Set every dimemsion size: XDimSize
    XDimSize = (long int*)malloc(sizeof(long int)*Dim);
    if (XMaxDimSize==XMinDimSize)
    {
      for (int i=0;i<Dim;i++)
        XDimSize[i] = XMaxDimSize;
    }
    else
    {
      XDimSizeGap = (XMaxDimSize-XMinDimSize)/(Dim-1);
      XDimSize[0] = XMaxDimSize;
      XDimSize[Dim-1] = XMinDimSize;
      for (int i=1;i<Dim-1;i++)
        XDimSize[i] = XDimSize[i-1]-XDimSizeGap;
    }
  } //End of generating tensor X
  else
  {
    //TODO: Read XFile content to X, decide X dimensions
    fp_in = fopen(XFile,"r");
    XDimSize = (long int*)malloc(sizeof(long int)*Dim);
  } //End of reading X from a file

  ModeComn = Moden-1; //Adjust to 0-based C pattern

  // Check
  assert (ModeComn < Dim);
  fprintf(stdout, "Dim: %d\n", Dim);
  fprintf(stdout, "Moden: %d\n", Moden);
  fprintf(stdout, "tflag: %c\n", tflag);

  for (int i=0;i<Dim;i++)
    XSize *= XDimSize[i];
  XMemRatio = (double)XSize *sizeof(double) / MemSize;
  print_array(XDimSize, Dim, "XDimSize", stdout);
#ifdef DEBUG
  fprintf(stdout, "XSize: %ld.\n", XSize);
  fprintf(stdout, "XMemRatio: %.2f\n", XMemRatio);
#endif

  //srand(time(NULL));
  X = (double*)malloc(sizeof(double)*XSize);
  for (long int i=0; i<XSize; i++)
    X[i] = (int)rand()%10+1; //i+1;
#ifdef PRINT
    print_array (X, XSize, "Tensor X", fp_out);
#endif

  if ( tflag=='t' )
  {
    //tflag='t', U: XDimSize[ModeComn]*R
    UDimSize[0] = XDimSize[ModeComn];
    UDimSize[1] = R;
  }
  else
  {
    //tflag='n', U: R*XDimSize[ModeComn]
    UDimSize[0] = R;
    UDimSize[1] = XDimSize[ModeComn];
  }
  USize = UDimSize[0]*UDimSize[1];
  UMemRatio = (double)USize *sizeof(double) / MemSize;
  print_array(UDimSize, 2, "UDimSize", stdout);
#ifdef DEBUG
  fprintf(stdout, "USize: %ld.\n", USize);
  fprintf(stdout, "UMemRatio: %.2f\n", UMemRatio);
#endif  

  //srand(time(NULL));
  U = (double*)malloc(sizeof(double)*USize);
  for (long int i=0;i<USize;i++)
    U[i] = (int)rand()%10+1; //1;
#ifdef PRINT
    print_array (U, USize, "Matrix U", fp_out);
#endif

  YDimSize = (long int*)malloc(sizeof(long int)*Dim);
  for (int i=0;i<Dim; i++)
    YDimSize[i] = XDimSize[i];
  YDimSize[ModeComn] = R;
  for (int i=0;i<Dim;i++)
    YSize *= YDimSize[i];
  YMemRatio = (double)YSize *sizeof(double) / MemSize;
  print_array(YDimSize, Dim, "YDimSize", stdout);
#ifdef DEBUG
  fprintf(stdout, "YSize: %ld.\n", YSize);
  fprintf(stdout, "YMemRatio: %.2f\n", YMemRatio);
#endif

  Y = (double*)malloc(sizeof(double)*YSize);
  for (int i=0; i<YSize; i++)
    Y[i] = 0;

  TotalSize = XSize+USize+YSize;
  TotalMemRatio = (double)TotalSize / MemSize;
  fprintf(stdout, "Total input size: %.0f, is %.1f%% of memory (%.1f GB).\n", TotalSize, TotalMemRatio*100, MemSize/10e9);
  assert (TotalMemRatio<=1);

  /* Decide degree value */
  int out_degree = 0;
  int out_omp_nt, out_mkl_nt;
  res = decide_paras(Dim, XDimSize, R, ModeComn, strategy, degree, &out_degree, &out_omp_nt, &out_mkl_nt);
  if (res != 0)
    fprintf(stdout, "Error in decide_degree.\n");
  fprintf(stdout, "MSTH: %f, MLTH: %f\n", MSTH, MLTH);
  fprintf(stdout, "Strategy: %d, degree: %d\n", strategy, out_degree);
  fprintf(stdout, "omp_nt: %d, mkl_nt: %d\n", out_omp_nt, out_mkl_nt);

  ModeCom = (int*)malloc(sizeof(int)*Dim);
  res = decide_component_mode (Dim, XDimSize, ModeComn, strategy, out_degree, ModeCom, &ModeComLen, &KTag);
  if (res != 0)
    fprintf(stdout, "Error in decide_component_mode.\n");
  fprintf(stdout, "ModeComn: %d\n", ModeComn);
  fprintf(stdout, "ModeCom (Component list):\n");
  for (int i=0;i<ModeComLen;i++)
    fprintf(stdout, "%d, ", ModeCom[i]);
  fprintf(stdout, "\n");

  // Warm-up
  Y = ttm (X,Dim,XDimSize,U,UDimSize,tflag,Y,YDimSize,ModeComn, ModeCom, ModeComLen);
  memset(Y,0,YSize*sizeof(double));

  //ttm_STime = bli_clock();
  timer ttm_time_timer;
  for ( int i=0; i<MAXLOOPS; i++)
  {
#ifdef PRINT
    memset(Y,0,YSize*sizeof(double));
#endif
    Y = ttm (X,Dim,XDimSize,U,UDimSize,tflag,Y,YDimSize,ModeComn, ModeCom, ModeComLen);
  }
  ttm_time = ttm_time_timer.get_ms()/1e3/MAXLOOPS;
  //ttm_time = (bli_clock() - ttm_STime)/MAXLOOPS;
  gflops = (2.0*YSize*XDimSize[ModeComn]) / (ttm_time*1.0e9);
  fprintf(stdout, "Time: %f sec\n",ttm_time);
  fprintf(stdout, "Perf: %.2f GFLOPS\n", gflops);

#ifdef PRINT
    print_array (Y, YSize, "Tensor Y", fp_out);
#endif

  free(X);
  free(U);
  free(Y);
  free(XDimSize);
  free(YDimSize);
  free(ModeCom);
  if (XFile != NULL )
    fclose(fp_in);
  fclose(fp_out);
  return 0;
}

int usage ( int argc, char** argv)
{
  printf("Give a tensor file OR specify the input parameters to generate a tensor with random valus.\n");
  printf("NOTE: only support double-precision floating points now.\n");
  printf("Give tensor parameters:\n");
  printf("\t-dim=[integer], number of tensor dimensions. Default is 3.\n");
  printf("\t-maxdimsize=[integer], maximum dimension size. Default is 100.\n");
  printf("\t-mindimsize=[integer], minimum dimension size. Default is 100.\n");
  printf("\t\tOther dimension sizes are generated by evenly distributed between mindimsize and maxdimsize.\n");
  printf("Give input tensor file:\n");
  printf("\t-file=[string], input file name\n");
  printf("Common input parameters:\n");
  printf("\t-mode=[integer], mode to be multiplied. Default is 2.\n");
  printf("\t-R=[integer], R is the reduced rank for the multiplied dimension. Default is 10.\n");
  printf("\t-tflag=[char], specify if the input matrix need to be transposed. Default is 'n'.\n");
  printf("\t-strategy=[integer], decide component modes. Default is 1.\n");
//  printf("\t\tstrategy=0, use auto-tumer to automatically choose the best strategy. \n");
  printf("\t\tstrategy=1, Forward Strategy. Always choose ModeComn to Dim-1 as the component modes, except ModeComn=Dim-1.\n");
  printf("\t\tstrategy=2, Backward Strategy. Choose the larger part as the component modes, see if ModeComn >(<) Dim/2.\n");
  printf("\t-degree=[integer], Maximum number of dimensions considered as component modes. Default is Dim.\n");
  printf("\t-help, Print this message\n");

  return 0;
}
