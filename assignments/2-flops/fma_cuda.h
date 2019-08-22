
int fma_dev_initialize (int N, int T, int *numDevices, float ***a);
int fma_dev_free (int N, int T, int *numDevices, float ***a);

int fma_dev_start (int N, int T, int blocksize, int gridsize, int numDevices, float **a, float b, float c);
int fma_dev_end (int N, int T, int numDevices, float **a, float b, float c);
