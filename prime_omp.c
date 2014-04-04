/* File: prime_omp.c (Reagan Lopez for CS415/515)
**
** Eratosthenes' prime-finding algorithm (Parallel version).
**
*/
#include <omp.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char **argv)
{
  int  *array;
  int  N, limit, cnt, i, j;
  int T = 1;
  int nthreads, tid;

  /* check command line first */
  if (argc < 3) {
    printf ("Usage: prime_seq <bound>\n");
    exit(0);
  }
  if ((N=atoi(argv[1])) < 2) {
    printf ("<bound> must be greater than 1\n");
    exit(0);
  }
/*
  if ((T=atoi(argv[2])) < 3) {
    printf ("<bound> must be greater than 2\n");
    exit(0);
  }
*/

	T = atoi(argv[2]);

	//set the number of threads
	omp_set_num_threads(T);

//  printf("Finding primes in range 1..%d\n", N);

  array = (int *) malloc(sizeof(int)*(N+1));
  #pragma omp parallel for
	  for (i=2; i<=N; i++) {
		array[i] = 1;
	}


  limit = (int) sqrt((double) N);
  #pragma omp parallel for private(i,j)
  for (i=2; i<=limit; i++) {
    if (array[i]==1) {
      for (j=i+i; j<=N; j+=i) {
		array[j] = 0;
	  }
    }
  }

  cnt = 0;
 #pragma omp parallel for reduction(+:cnt)
  for (i=2; i<=N; i++) {
    if (array[i]==1)
      cnt += 1;
	  //cnt++;
  }

  //printf("Total %d primes found\n", cnt);

#ifdef DEBUG
  { char ans;
    printf("Print all (y/n)? ");
    scanf("%c", &ans);
    if (ans=='y') {
      for (i=2; i<=N; i++)
	if (array[i]==1)
	  printf("%d, ", i);
      printf("\n");
    }
  }
#endif /* DEBUG */
}