#include "driver.h"

int gen_ttm (int Dim, long int *XDimSize,
    long int *UDimSize, char tflag,
    long int *YDimSize,
    int ModeComn, int strategy,
    int *ModeCom, int ModeComLen, int KTag,
    int omp_nt, int mkl_nt)
{
  for (int i=0; i<ModeComLen; i++)
    assert (ModeComn != ModeCom[i]);

  char *ofile = (char*)OFILE;
  FILE *ofp = fopen(ofile, "w");
  printf ("### Generate \"%s\" ... ###\n", ofile);

  fprintf(ofp, "#include \"driver.h\"\n\n");
  fprintf (ofp, "double* ttm (double *X, const int Dim, long int *XDimSize,\n\
            double *U, long int *UDimSize, char tflag,\n\
            double *Y, long int *YDimSize,\n\
            int ModeComn, int *ModeCom, int ModeComLen)\n");
  fprintf (ofp, "{\n");

  // According to different Input ModeConm to decide if use BLIS or MKL
  // KTag has no relation with using MKL or BLIS, only deciding which kernel to use.
  if (ModeComn < Dim/2 || ModeComn == Dim-1)  //TODO: modify this according to "decide_component_mode" function
  {
    USE_BLIS=0;
    USE_MKL=1;
    printf("Use MKL.\n");
  }    
  else
  {
    if (strategy == 1)
    {
      USE_BLIS=0;
      USE_MKL=1;
      printf("Use MKL.\n");
    } 
    else if (strategy == 2)
    {
      USE_BLIS=1;
      USE_MKL=0;
      printf("Use BLIS.\n");
    }
  }

if (USE_BLIS)
  fprintf (ofp, "\tbli_init();\n\n");

#if 0
  fprintf (ofp, "\tint nthreads, tid;\n");
  fprintf (ofp, "#pragma omp parallel\n");
  fprintf (ofp, "{\n");
  fprintf (ofp, "\ttid = omp_get_thread_num();\n");
  fprintf (ofp, "\tif (tid == 0)\n");
  fprintf (ofp, "\t{\n");
  fprintf (ofp, "\t\tnthreads = omp_get_num_threads();\n");
  //fprintf (ofp, "\t\tprintf(\"nthreads : %%d\\n\", nthreads);\n");
  fprintf (ofp, "\t}\n");
  fprintf (ofp, "}\n\n");
#endif

#if 0
  int nthreads, tid;
#pragma omp parallel
  {
    tid = omp_get_thread_num();
    if (tid == 0 )
    {
      nthreads = omp_get_num_threads();
      printf("nthreads (in gen_ttm) : %d\n", nthreads);
    }
  }
#endif

  /* Decide the traspose parameters for a and b. Note: tflag only influence matrix U. */
  fprintf(ofp, "\t//Decide transa and transb according to tflag and ModeComn and ModeCom's relationship.\n");
  // tflag procedure needs to be checked again.
  if (KTag == 1)
  {
    //a: U, b: X
    if (tflag=='t') //For U (a)
    {
    if (USE_BLIS)
      fprintf (ofp, "\ttrans_t transa = BLIS_TRANSPOSE;\n");
    if (USE_MKL)
      fprintf (ofp, "\tCBLAS_TRANSPOSE transa = CblasTrans;\n");
    }
    else
    {
    if (USE_BLIS)
      fprintf (ofp, "\ttrans_t transa = BLIS_NO_TRANSPOSE;\n");
    if (USE_MKL)
      fprintf (ofp, "\tCBLAS_TRANSPOSE transa = CblasNoTrans;\n");
    }
    if (USE_BLIS)
    fprintf (ofp, "\ttrans_t transb = BLIS_NO_TRANSPOSE;\n");
    if (USE_MKL)
    fprintf (ofp, "\tCBLAS_TRANSPOSE transb = CblasNoTrans;\n");
  }
  else
  {
    //a: X, b: U'
    if (USE_BLIS)
    fprintf (ofp, "\ttrans_t transa = BLIS_NO_TRANSPOSE;\n");
    if (USE_MKL)
    fprintf (ofp, "\tCBLAS_TRANSPOSE transa = CblasNoTrans;\n");
    if (tflag=='t')
    {
      if (USE_BLIS)
      fprintf (ofp, "\ttrans_t transb = BLIS_NO_TRANSPOSE;\n");
      if (USE_MKL)
      fprintf (ofp, "\tCBLAS_TRANSPOSE transb = CblasNoTrans;\n");
    }
    else
    {
      if (USE_BLIS)
      fprintf (ofp, "\ttrans_t transb = BLIS_TRANSPOSE;\n");
      if (USE_MKL)
      fprintf (ofp, "\tCBLAS_TRANSPOSE transb = CblasTrans;\n");
    }
  }

  if (USE_BLIS)
  {
    fprintf (ofp, "\tnum_t dt = BLIS_DOUBLE;\n");
    fprintf (ofp, "\tdim_t m,n,k;\n");
    fprintf (ofp, "\tobj_t alpha, beta;\n");
    fprintf (ofp, "\tobj_t a, b, c;\n\n");
  }
  if (USE_MKL)
  {
    fprintf (ofp, "\tlong int m,n,k;\n");
    fprintf (ofp, "\tdouble alpha, beta;\n");
  }

  fprintf (ofp, "\tlong int rsu,csu,rsx,csx,rsy,csy;\n");
  fprintf (ofp, "\tlong int XLoopStride = 0, YLoopStride = 0;\n");

  if (USE_BLIS)
  {
    fprintf (ofp, "\tbli_obj_create(dt, 1, 1, 0, 0, &alpha);\n");
    fprintf (ofp, "\tbli_obj_create(dt, 1, 1, 0, 0, &beta);\n");
    fprintf (ofp, "\tbli_setsc(1.0, 0.0, &alpha);\n");
    fprintf (ofp, "\tbli_setsc(1.0, 0.0, &beta);\n\n");
  }
  if (USE_MKL)
  {
    fprintf (ofp, "\talpha = 1.0;\n");
    fprintf (ofp, "\tbeta = 1.0;\n");
  }
#ifdef TIMING
fprintf(ofp, "double gemm_time = 0, tmp_time = 0, tmp_STime = 0;\n");
fprintf(ofp, "double gflops = 0;\n");
#endif

  fprintf (ofp, "\n\t/****Generated TM algo according to different modes and dimensions ***/\n");
  // Inner-most matrix size: row size: ModeComn*ModeCom; column size: ModeCom2
  // Note: data layout has nothing to do with tflag.
  // Decide the strides values.
  long int *XStrides;
  long int UStrides[2];
  long int *YStrides;
  UStrides[0] = UDimSize[1];
  UStrides[1] = 1;
  XStrides = (long int*)malloc(sizeof(long int)*Dim);
  YStrides = (long int*)malloc(sizeof(long int)*Dim);
  XStrides[Dim-1] = 1;  //Row-major
  YStrides[Dim-1] = 1;  //Row-major
  for (int i=Dim-2; i>=0; i--) {
    XStrides[i] = XStrides[i+1]*XDimSize[i+1];
    YStrides[i] = YStrides[i+1]*YDimSize[i+1];
  }
  print_array(XStrides,Dim, "XSrides", stdout);
  print_array(YStrides,Dim, "YSrides", stdout);

  long int m=1,n=1,k=1;
  long int rsu, csu, rsx, csx, rsy, csy;
  long int SubSize = 1;
  for (int i=0;i<ModeComLen;i++)
    SubSize *= XDimSize[ModeCom[i]];
  printf("SubSize: %d\n", SubSize);

  if (KTag==1)
  {
    //a: U, b: X
    m = YDimSize[ModeComn];
    n = SubSize;
    rsx = XStrides[ModeComn];
    csx = XStrides[ModeCom[ModeComLen-1]];
    rsy = YStrides[ModeComn];
    csy = YStrides[ModeCom[ModeComLen-1]];
  }
  else if (KTag==2)
  {
    //a: X, b: U'
    m = SubSize;
    n = YDimSize[ModeComn];
    rsx = XStrides[ModeCom[ModeComLen-1]];
    csx = XStrides[ModeComn];
    rsy = YStrides[ModeCom[ModeComLen-1]];
    csy = YStrides[ModeComn];
  }
  else
    printf("Invalid value for KTag.\n");
  k = XDimSize[ModeComn];
  rsu = UDimSize[1];
  csu = 1;

  fprintf(ofp, "\tm = %d;\n", (long int)m);
  fprintf(ofp, "\tn = %d;\n", (long int)n);
  fprintf(ofp, "\tk = %d;\n", (long int)k);
  fprintf(ofp, "\trsu = %d;\n", rsu);
  fprintf(ofp, "\tcsu = %d;\n", csu);
  fprintf(ofp, "\trsx = %d;\n", rsx);
  fprintf(ofp, "\tcsx = %d;\n", csx);
  fprintf(ofp, "\trsy = %d;\n", rsy);
  fprintf(ofp, "\tcsy = %d;\n\n", csy);
  printf("m: %d, n: %d, k:%d\n", (long int)m, (long int)n, (long int)k);
  printf("rsu: %d, csu: %d\n", rsu, csu);
  printf("rsx: %d, csx: %d\n", rsx, csx);
  printf("rsy: %d, csy: %d\n\n", rsy, csy);

  if (USE_BLIS)
  {
    fprintf(ofp, "\tbli_obj_create_without_buffer( dt, m, k, &a);\n");
    if (KTag==1)
      fprintf(ofp, "\tbli_obj_create_without_buffer( dt, k, n, &b);\n");
    else if (KTag==2)
      fprintf(ofp, "\tbli_obj_create_without_buffer( dt, n, k, &b);\n");
    fprintf(ofp, "\tbli_obj_create_without_buffer( dt, m, n, &c);\n");
    fprintf(ofp, "\tbli_obj_set_conjtrans( transa, a );\n");
    fprintf(ofp, "\tbli_obj_set_conjtrans( transb, b );\n\n");
  }

  //Generate nested loops
  int NumLoops = Dim-ModeComLen-1;
  printf("NumLoops: %d\n", NumLoops);
  int *Loops; //Loop indices
  char **LoopIndices; //Loop indices, e.g. i2.
  char *buffer; //buffering loop index number
  char *LoopStrideIndX, *LoopStrideIndY;  //Starting location of each loop
  int *AllocThreads;
  int tmpTag = 0;

  if ( NumLoops > 0) //Loops are needed
  {
    AllocThreads = (int*)malloc(sizeof(int)*NumLoops);
    memset (AllocThreads,1,sizeof(int)*NumLoops);
    //for (int i=0;i<NumLoops;i++)
      //AllocThreads[i] = 1;
    AllocThreads[NumLoops-1] = 4;

    Loops = (int*)malloc(sizeof(int)*NumLoops);
    for (int i=0,j=0;i<Dim;i++)
    {
      tmpTag = 0;
      for (int k=0;k<ModeComLen;k++)
      {
        if (i == ModeCom[k])
        {
          tmpTag = 1;
          break;
        }
      }
      if (tmpTag ==0 && i!=ModeComn)
      {
        Loops[j] = i;
        j ++;
      }
    }
    #if 0
    print_array(Loops, NumLoops, "Loops", stdout);
    #endif

    buffer = (char*)malloc(sizeof(char)*MAXDIMLEN);
    LoopIndices = (char**)malloc(sizeof(char*)*NumLoops);
    for (int i=0;i<NumLoops;i++)
    {
      LoopIndices[i] = (char*)calloc((MAXDIMLEN+1),sizeof(char)); //Support up to 100-dim
      LoopIndices[i][0] = 'i';
      sprintf(buffer, "%d", Loops[i]);
      strcat (LoopIndices[i], buffer);
    }
    print_array(LoopIndices, NumLoops, "LoopIndices", stdout);

    LoopStrideIndX = (char*)calloc(NumLoops*100,sizeof(char)); //Maximum string length of each loop, set to 100.
    LoopStrideIndY = (char*)calloc(NumLoops*100,sizeof(char));
    char buffer2[64];  //Maximum 10^64 number
    for (int i=0;i<NumLoops-1;i++)
    {
      strcat (LoopStrideIndX, LoopIndices[i]);
      strcat (LoopStrideIndX, "*");
      sprintf(buffer2, "%d", XStrides[Loops[i]]);
      strcat (LoopStrideIndX, buffer2);
      strcat (LoopStrideIndX, "+");

      strcat (LoopStrideIndY, LoopIndices[i]);
      strcat (LoopStrideIndY, "*");
      sprintf(buffer2, "%d", YStrides[Loops[i]]);
      strcat (LoopStrideIndY, buffer2);
      strcat (LoopStrideIndY, "+");
    }
    strcat (LoopStrideIndX, LoopIndices[NumLoops-1]);
    strcat (LoopStrideIndX, "*");
    sprintf(buffer2, "%d", XStrides[Loops[NumLoops-1]]);
    strcat (LoopStrideIndX, buffer2);

    strcat (LoopStrideIndY, LoopIndices[NumLoops-1]);
    strcat (LoopStrideIndY, "*");
    sprintf(buffer2, "%d", YStrides[Loops[NumLoops-1]]);
    strcat (LoopStrideIndY, buffer2);
  #ifdef DEBUG
    fprintf(stdout, "LoopStrideIndX : %s\n", LoopStrideIndX);
    fprintf(stdout, "LoopStrideIndY : %s\n", LoopStrideIndY);
  #endif

#ifdef TIMING
fprintf(ofp, "tmp_STime = bli_clock();\n");
//fprintf(ofp, "timer tmp_time_timer;\n");
#endif
    fprintf(ofp, "#pragma omp parallel for default(shared) schedule(static) num_threads(%d)\n", omp_nt);
    //fprintf(ofp, "#pragma omp parallel for default(shared) schedule(static) collapse(%d) num_threads(%d)\n", NumLoops, omp_nt);
    for (int i=0;i<NumLoops;i++)
    {
      //fprintf(ofp, "#pragma omp parallel for default(shared) schedule(static) num_threads(%d)\n", AllocThreads[i]);
      //if ( i == 0 )
        //fprintf(ofp, "#pragma omp parallel for default(shared) schedule(static) num_threads(2)\n");
      fprintf(ofp, "\tfor (long int %s=0;%s<%d;%s++)\n", LoopIndices[i],LoopIndices[i],XDimSize[Loops[i]],LoopIndices[i]);
    }
    fprintf(ofp, "\t{\n");
    fprintf(ofp, "\t\tmkl_set_dynamic(0);\n");
    fprintf(ofp, "\t\tmkl_set_num_threads_local(%d);\n", mkl_nt);
    //fprintf(ofp, "printf(\"mkl_get_max_threads: %%d\\n\", mkl_get_max_threads());\n");
    fprintf(ofp, "\t\tXLoopStride = %s;\n",LoopStrideIndX);
    fprintf(ofp, "\t\tYLoopStride = %s;\n",LoopStrideIndY);

    if (USE_BLIS)
    {
      if (KTag==1)
      {
        //a: U, b: X
        fprintf(ofp, "\t\tbli_obj_attach_buffer (U, rsu, csu, 1, &a);\n");
        fprintf(ofp, "\t\tbli_obj_attach_buffer (X+XLoopStride, rsx, csx, 1, &b);\n");
      }
      else if (KTag==2)
      {
        //a: X, b: U'
        fprintf(ofp, "\t\tbli_obj_attach_buffer (X+XLoopStride, rsx, csx, 1, &a);\n");
        fprintf(ofp, "\t\tbli_obj_attach_buffer (U, rsu, csu, 1, &b);\n");
      }
      fprintf(ofp, "\t\tbli_obj_attach_buffer (Y+YLoopStride, rsy, csy, 1, &c);\n\n");
      fprintf(ofp, "\t\tbli_gemm (&alpha, &a, &b, &beta, &c);\n");
    }
    if (USE_MKL)
    {
      assert(csx==1);
      assert(csy==1);
      if (KTag==1)
        fprintf(ofp, "\t\tcblas_dgemm(CblasRowMajor, transa, transb, \n\
              \t\t\tm, n, k, \n\
              \t\t\talpha, \n\
              \t\t\tU, rsu, \n\
              \t\t\tX+XLoopStride, rsx, \n\
              \t\t\tbeta, \n\
              \t\t\tY+YLoopStride, rsy);\n");
      else if (KTag==2)
        fprintf(ofp, "\t\tcblas_dgemm (CblasRowMajor, transa, transb, \n\
              \t\t\tm, n, k, \n\
              \t\t\talpha, \n\
              \t\t\tX+XLoopStride, rsx, \n\
              \t\t\tU, rsu, \n\
              \t\t\tbeta, \n\
              \t\t\tY+YLoopStride, rsy);\n");
      else
        printf("Invalid value for KTag.\n");
    }
    fprintf(ofp, "\t}\n");
#ifdef TIMING
fprintf(ofp, "tmp_time = bli_clock()-tmp_STime;\n");
//fprintf(ofp, "tmp_time = tmp_time_timer.get_ms()/1e3;\n");
fprintf(ofp, "gemm_time = tmp_time;\n");
#endif
  } //End of if ( NumLoops>0) 
  else  // if (NumLoops=0), No loops are needed.
  {
    fprintf(ofp, "\t\tmkl_set_dynamic(0);\n");
    fprintf(ofp, "\t\tmkl_set_num_threads_local(%d);\n", mkl_nt);
    if (USE_BLIS)
    {
      if (KTag==1)
      {
        //a: U, b: X
        fprintf(ofp, "\tbli_obj_attach_buffer (U, rsu, csu, 1, &a);\n");
        fprintf(ofp, "\tbli_obj_attach_buffer (X, rsx, csx, 1, &b);\n");
      }
      else if (KTag==2)
      {
        //a: X, b: U'
        fprintf(ofp, "\tbli_obj_attach_buffer (X, rsx, csx, 1, &a);\n");
        fprintf(ofp, "\tbli_obj_attach_buffer (U, rsu, csu, 1, &b);\n");
      }
      fprintf(ofp, "\tbli_obj_attach_buffer (Y, rsy, csy, 1, &c);\n\n");
      fprintf(ofp, "\tbli_gemm (&alpha, &a, &b, &beta, &c);\n");
    }
    if (USE_MKL)
    {
      assert(csx==1);
      assert(csy==1);
#ifdef TIMING
fprintf(ofp, "tmp_STime = bli_clock();\n");
#endif
      if (KTag==1)
        fprintf(ofp, "\tcblas_dgemm(CblasRowMajor, transa, transb, \n\
              \t\tm, n, k, \n\
              \t\talpha, \n\
              \t\tU, rsu, \n\
              \t\tX, rsx, \n\
              \t\tbeta, \n\
              \t\tY, rsy);\n");
      else if (KTag==2)
        fprintf(ofp, "\tcblas_dgemm (CblasRowMajor, transa, transb, \n\
              \t\tm, n, k, \n\
              \t\talpha, \n\
              \t\tX, rsx, \n\
              \t\tU, rsu, \n\
              \t\tbeta, \n\
              \t\tY, rsy);\n");
      else
        printf("Invalid value for KTag.\n");
#ifdef TIMING
fprintf(ofp, "tmp_time = bli_clock()-tmp_STime;\n");
fprintf(ofp, "gemm_time = tmp_time;\n");
#endif
    }
  }

if (USE_BLIS)
{
  fprintf(ofp, "\n\tbli_obj_free(&alpha);\n");
  fprintf(ofp, "\tbli_obj_free(&beta);\n");
  fprintf(ofp, "\tbli_finalize();\n\n");
}
#ifdef TIMING
double tmp_size = 1;
for (int i=0;i<NumLoops;i++)
tmp_size *= XDimSize[Loops[i]];
fprintf(ofp, "gemm_time /= %.0f;\n", tmp_size);
fprintf(ofp, "gflops = (2.0*m*n*k)/(gemm_time*1.0e9);\n");
fprintf(ofp, "printf(\"Gemm time : %%.3e sec, Perf : %%.2f GFLOPS\\n\", gemm_time, gflops);\n");
#endif
  fprintf(ofp, "\treturn Y;\n");
  fprintf(ofp, "}\n");

  printf ("### ... Done! ###\n");

  free(XStrides);
  free(YStrides);
  if ( NumLoops > 0) 
  {
    free(Loops);
    for (int i=0;i<NumLoops;i++)
      free(LoopIndices[i]);
    free(LoopIndices);
    free(buffer);
    free(LoopStrideIndX);
    free(LoopStrideIndY);
    free(AllocThreads);
  }
  fclose(ofp);

  return 0;
}
