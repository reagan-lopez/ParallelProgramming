/* File: qsort_omp.c (Reagan Lopez for CS415/515)
**
** Quicksort algorithm (openMP parallel version).
**
*/
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MIN_SIZE 10

int T = 2;
int nthreads, tid;

/* Swap two array elements
 */
void swap(int *array, int i, int j)
{
  if (i == j) return;
  int tmp = array[i];
  array[i] = array[j];
  array[j] = tmp;
}

/* Bubble sort for the base cases
 */
void bubble_sort(int *array, int low, int high)
{
  if (low >= high) return;
  int i, j;

  for (i = low; i <= high; i++)
    for (j = i+1; j <= high; j++)
      if (array[i] > array[j])
	swap(array, i, j);
}

/* Partition the array into two halves and return
 * the middle index
 */
int partition(int *array, int low, int high)
{
  /* Use the lowest element as pivot */
  int pivot = array[low], middle = low, i;

  swap(array, low, high);

  for(i=low; i<high; i++) {
    if(array[i] < pivot) {
      swap(array, i, middle);
      middle++;
    }
  }
  swap(array, high, middle);

  return middle;
}

/* Quicksort the array
 */
void quicksort(int *array, int low, int high)
{

  if (low >= high) return;
  if (high - low < MIN_SIZE) {
    bubble_sort(array, low, high);
    return;
  }

  int middle = partition(array, low, high);

/*
	int start[2], end[2];
	start[0] = low;
	end[0] = middle - 1;
	start[1] = 	middle + 1;
	end[1] =  high;
	int i;
	#pragma omp parallel num_threads(T)
	{
	#pragma omp for nowait
		 for(i = 0; i <= 1; i++) {
		   quicksort(array, start[i], end[i]);
		 }
	}
*/

   #pragma omp task
	quicksort(array, low, middle-1);
   #pragma omp task
	quicksort(array, middle+1, high);
   #pragma omp taskwait


}

/* Main routine for testing quicksort
 */
int main(int argc, char **argv)
{
  int *array, N, i, j;

	/* check command line first */
	if (argc < 2) {
		printf ("Usage : qsort_seq <array_size>\n");
		exit(0);
	}

	if ((N=atoi(argv[1])) < 2) {
		printf ("<array_size> must be greater than 2\n");
		exit(0);
	}

	if (argc == 2) {
		printf("<T> not specified. Defaulting <T> to 1\n");
		T = 1;
	}

	if ((argc > 2) && (T=atoi(argv[2])) < 1) {
		 printf("<T> must not be < 1. Defaulting <T> to 1\n");
		 T = 1;
	}

	//set the number of threads
	omp_set_num_threads(T);

  //printf("Create an array of %d randomized integers ...\n", N);

  /* init an array with values 1. N-1 in parallel*/
  array = (int *) malloc(sizeof(int) * N);
  #pragma omp parallel for
  for (i=0; i<N; i++)
    array[i] = i+1;

  /* randomly permute array elements in parallel*/
  srand(time(NULL));
  #pragma omp parallel for private(j)
  for (i=0; i<N; i++) {
    j = (rand()*1./RAND_MAX)*(N-1);
    swap(array, i, j);
  }

  //printf("Start sorting ...\n");

  #pragma omp parallel
  {
	#pragma omp single
		quicksort(array, 0, N-1);
  }

  /* Verify the result */
  for (i=0; i<N-1; i++) {
    if (array[i]>array[i+1]) {
      printf("Verification failed, array[%d]=%d, array[%d]=%d\n",
	     i, array[i], i+1, array[i+1]);
      return;
    }
  }
  //printf("Sorting result verified!\n");
}