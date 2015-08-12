#include "driver.h"

char* get_arg(int argc, char** argv, const char* key)
{
    char* val = NULL;
    int keylen = strlen(key);

    for(int i = 1; i < argc; i++)
    {
        char* token = argv[i];
        if(strncmp(token, "-", 1) != 0)
            continue;
        token += 1;
        if(strncmp(token, key, keylen) != 0)
            continue;
        token += (keylen+1);
        val = token;
    }

    return val;
}

int print_error ( const char* msg)
{
  fprintf(stdout, "Error: %s.\n", msg);
  return 0;
}

int print_array (int *array, int size, const char *msg, FILE *fp)
{
	fprintf(fp, "%s: ", msg);
	fprintf(fp, "[");
	for (int i=0;i<size-1;i++)
		fprintf(fp, "%d, ", array[i]);
	fprintf(fp, "%d]\n", array[size-1]);
	return 0;
}

int print_array (long int *array, long int size, const char *msg, FILE *fp)
{
    fprintf(fp, "%s: ", msg);
      fprintf(fp, "[");
        for (int i=0;i<size-1;i++)
              fprintf(fp, "%d, ", array[i]);
          fprintf(fp, "%d]\n", array[size-1]);
            return 0;
}

int print_array (double *array, int size, const char *msg, FILE *fp)
{
	fprintf(fp, "%s: ", msg);
	fprintf(fp, "[");
	for (int i=0;i<size-1;i++)
		fprintf(fp, "%.1f, ", array[i]);
	fprintf(fp, "%.1f]\n", array[size-1]);
	return 0;
}

int print_array (char **array, int size, const char *msg, FILE *fp)
{
	fprintf(fp, "%s: ", msg);
	fprintf(fp, "[");
	for (int i=0;i<size-1;i++)
		fprintf(fp, "%s, ", array[i]);
	fprintf(fp, "%s]\n", array[size-1]);
	return 0;
}
