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
  long int *YDimSize;  //Tensor Y's dimension sizes
  long int YSize = 1;  //Tensor Y's total size
  double YMemRatio = 0; //Memory ratio Y occupies
  long int TotalSize = 0; //Counting X, U, and Y
  double TotalMemRatio = 0; //Memory ratio of total size
  int res = 0;
  int omp_nt = 1;
  int mkl_nt = 1;

  //Timing
  double gen_STime, gen_time;

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
  input_str = get_arg (argc, argv, "OMPNT");
  if (input_str != NULL )
    omp_nt = atoi(input_str);
  input_str = get_arg (argc, argv, "MKLNT");
  if (input_str != NULL )
    mkl_nt = atoi(input_str);
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

  TotalSize = XSize+USize+YSize;
  TotalMemRatio = (double)TotalSize / MemSize;
  fprintf(stdout, "Total input size: %ld, is %.1f%% of memory (%.1f GB).\n", TotalSize, TotalMemRatio, MemSize/10e9);
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

  gen_STime = bli_clock();
  res = gen_ttm (Dim,XDimSize,UDimSize,tflag,YDimSize,ModeComn,strategy,ModeCom,ModeComLen,KTag, out_omp_nt, out_mkl_nt);
  gen_time = bli_clock() - gen_STime;
  fprintf(stdout, "Gen Time: %.2f sec\n", gen_time);
  if (res != 0)
    fprintf(stdout, "Error in gen_ttm.\n");

  free(XDimSize);
  free(YDimSize);
  free(ModeCom);
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
