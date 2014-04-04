/* File: ring.c (Reagan Lopez for CS415/515)
**
** ring program (MPI parallel version).
**
*/
#include "stdio.h"
#include "mpi.h"
#include <unistd.h>

#define BUFSIZE	1

int n, m;

int main(int argc, char **argv) {
  char host[20];
  int size, rank;
  MPI_Status status;
  int st_count, st_source, st_tag, to, from;
  double t1, t2, elapsed_time;  

  
  MPI_Init(&argc, &argv);
  
  MPI_Comm_size(MPI_COMM_WORLD, &size);  
  if (size < 2) {
    printf("Need at least 2 processes.\n");
    MPI_Finalize();
    return(1);
  }

  gethostname(host, 20);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  to = (rank + 1) % size;
  from = (rank + size - 1) % size;  
	 
	if (rank == 0)
	{
		n = 10;
		t1 = MPI_Wtime(); //start the timer	
		
		MPI_Send(&n, BUFSIZE, MPI_INT, to, 2001, MPI_COMM_WORLD);
		printf("rank %d on %s sent integer %d\n", rank, host, n);
		MPI_Recv(&m, BUFSIZE, MPI_INT, from, 2001, MPI_COMM_WORLD, &status);
		printf("rank %d on %s received integer %d\n", rank, host, m);
		
		t2 = MPI_Wtime(); //stop the timer
		elapsed_time = t2 - t1; //calculate elapsed time		
		printf("elapsed time is %f secs.\n", elapsed_time);		
	}
	else
	{
		MPI_Recv(&m, BUFSIZE, MPI_INT, from, 2001, MPI_COMM_WORLD, &status);
		printf("rank %d on %s received integer %d\n", rank, host, m);
		n = m + 1;
		MPI_Send(&n, BUFSIZE, MPI_INT, to, 2001, MPI_COMM_WORLD);
		printf("rank %d on %s sent integer %d\n", rank, host, n);		
	}
	



  MPI_Finalize();
  return(0);
}