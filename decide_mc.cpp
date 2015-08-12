#include "driver.h"

int decide_component_mode (int Dim, long int* XDimSize, int ModeComn, int strategy, int degree, int *ModeCom, int *ModeComLen, int *KTag)
{
	int tmpLen = 0;  //Template for ModeComLen
	int tmpTag = 0;	//tmpTag=1,Use U*X kernel; tmpTag=2, Use X*U' kernel.

	if (ModeComn < Dim/2)	//TODO: Add "=" or decide some split value
	{
		tmpLen = Dim-ModeComn-1;
    if (degree < tmpLen)
    {
      // Get component modes from the end
      for (int i=Dim-degree;i<Dim;i++)
        ModeCom[i-Dim+degree] = i;
      tmpLen = degree;
    }
    else
    {
      for (int i=ModeComn+1;i<Dim;i++)
        ModeCom[i-ModeComn-1] = i;
    }
		tmpTag = 1;
	}
	else if (ModeComn == Dim-1)
	{
		tmpLen = ModeComn;
    if (degree < tmpLen)
    {
      // Get component modes from back (ModeComn)
      for (int i=ModeComn-degree;i<ModeComn;i++)
        ModeCom[i-ModeComn+degree] = i;
      tmpLen = degree;
    }
    else
    {
      for (int i=0;i<ModeComn;i++)
        ModeCom[i] = i;
    }
		tmpTag = 2;
	}
	else  // Dim/2 <= ModeComn < Dim-1
	{
		switch (strategy)
		{
			// Best loop strategy
			case 0:
				break;

			// Forward strategy: Leading dimension strategy
			case 1:
				tmpLen = Dim-ModeComn-1;
        if ( degree < tmpLen)
        {
          // Get component modes from the end
          for (int i=Dim-degree;i<Dim;i++)
            ModeCom[i-Dim+degree] = i;
          tmpLen = degree;
        }
        else
        {
          for (int i=ModeComn+1;i<Dim;i++)
            ModeCom[i-ModeComn-1] = i;
        }
				tmpTag = 1;			
				break;				

	    	// Backward strategy: Large matrix strategy
			case 2:
				tmpLen = ModeComn;
        if (degree < tmpLen)
        {
          // Get component modes from ModeComn
          for (int i=ModeComn-degree;i<ModeComn;i++)
            ModeCom[i-ModeComn+degree] = i;
          tmpLen = degree;
        }
        else
        {
          for (int i=0;i<ModeComn;i++)
            ModeCom[i] = i;
        }
				tmpTag = 2;
				break;
		}
	}	//End of switch(strategy)

 	*ModeComLen = tmpLen;
 	*KTag = tmpTag;

	return 0;
}


int decide_paras(int Dim, long int* XDimSize, int R, int ModeComn, int strategy, int degree, int* out_degree, int *out_omp_nt, int *out_mkl_nt)
{
  long int mat_dimsize = 1;
  double MM_size = 0;
  int tmp_degree = 0;
  int i = 1;

  switch (strategy)
  {
    case 1:
      while (MM_size <= MSTH)
      {
        mat_dimsize *= XDimSize[Dim-i];
        MM_size = (mat_dimsize * XDimSize[ModeComn] + R*XDimSize[ModeComn] + R *mat_dimsize)*8/1.0e6;
        i ++;
      printf("mat_dimsize: %lu\n", mat_dimsize);
      printf("MM_size: %f\n", MM_size);
      }
      if (MM_size <= MLTH)
        tmp_degree = i-1;
      else 
        if (i>2)
          tmp_degree = i-2;
        else
          tmp_degree = 1;

      // User defined degree value
      if (degree != tmp_degree && degree > 0)
        tmp_degree = degree;

      mat_dimsize = 1;
      for (int j=1; j<=tmp_degree; j++)
        mat_dimsize *= XDimSize[Dim-j];
      MM_size = (mat_dimsize * XDimSize[ModeComn] + R*XDimSize[ModeComn] + R *mat_dimsize)*8/1.0e6;
      //printf("mat_dimsize: %lu\n", mat_dimsize);
      //printf("MM_size: %f\n", MM_size);
      if (MM_size <= PTH)
      {
        *out_omp_nt = MAXTHS;
        *out_mkl_nt = 1;
      }
      else
      {
        *out_omp_nt = 1;
        *out_mkl_nt = MAXTHS;
      }
      break;

    case 2:
      while (MM_size <= MSTH)
      {
        mat_dimsize *= XDimSize[i-1];
        MM_size = (mat_dimsize * XDimSize[ModeComn] + R*XDimSize[ModeComn] + R *mat_dimsize)*8/1.0e6;
        i ++;
      }
      if (MM_size <= MLTH)
        tmp_degree = i-1;
      else 
        if (i>2)
          tmp_degree = i-2;
        else
          tmp_degree = 1;

      // User defined degree value
      if (degree != tmp_degree && degree > 0)
        tmp_degree = degree;

      mat_dimsize = 1;
      for (int j=1; j<=tmp_degree; j++)
        mat_dimsize *= XDimSize[j-1];
      MM_size = (mat_dimsize * XDimSize[ModeComn] + R*XDimSize[ModeComn] + R *mat_dimsize)*8/1.0e6;
      //printf("mat_dimsize: %lu\n", mat_dimsize);
      //printf("MM_size: %f\n", MM_size);
      if (MM_size <= PTH)
      {
        *out_omp_nt = MAXTHS;
        *out_mkl_nt = 1;
      }
      else
      {
        *out_omp_nt = 1;
        *out_mkl_nt = MAXTHS;
      }
      break;
  }

  *out_degree = tmp_degree;

  return 0;
}
