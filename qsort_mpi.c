/* File: qsort_mpi.c (Reagan Lopez for CS415/515)
**
** quicksort program (MPI parallel version).
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


//swap function
void swap(unsigned int * v, int i, int j)
{
  unsigned int t = v[i];
  v[i] = v[j];
  v[j] = t;
}


//quicksort function
void quicksort(unsigned int * v, unsigned int s, unsigned int n)
{
  int x, p, i;
  //base case
  if (n <= 1)
    return;

  //calculate pivot and swap with first element
  x = v[s + n/2];
  swap(v, s, s + n/2);

  p = s;
  for (i = s+1; i < s+n; i++)
    if (v[i] < x) {
      p++;
      swap(v, i, p);
    }

  swap(v, s, p); //swap pivot into place
  
  //recurse the partitions
  quicksort(v, s, p-s);
  quicksort(v, p+1, s+n-p-1);
}


//merge two sorted arrays
int * merge(unsigned int * v1, unsigned int n1, unsigned int * v2, unsigned int n2)
{
  int * result = (unsigned int *)malloc((n1 + n2) * sizeof(unsigned int));
  int i = 0;
  int j = 0;
  int k;
  for (k = 0; k < n1 + n2; k++) {
    if (i >= n1) {
      result[k] = v2[j];
      j++;
    }
    else if (j >= n2) {
      result[k] = v1[i];
      i++;
    }
    else if (v1[i] < v2[j]) {
      result[k] = v1[i];
      i++;
    }
    else {
      result[k] = v2[j];
      j++;
    }
  }
  return result;
}


int main(int argc, char ** argv)
{
  unsigned int n, s, c, o;
  unsigned int * data = NULL;
  unsigned int * chunk;
  unsigned int * other;  
  int i, p, id, size, step;
  double t1, t2, elapsed_time;
  MPI_Status status;  
  FILE * file = NULL;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &p); //get the total number of processes
  MPI_Comm_rank(MPI_COMM_WORLD, &id); //get the rank of the current process
  
  size = sizeof(unsigned int);

  if (id == 0) {// Rank 0 process reads the input file and creates chunks of numbers and distributes them to other processes for sorting.
    //t1 = MPI_Wtime(); //start the timer
    file = fopen(argv[1], "r"); //open input file file
	fread(&n, size, 1, file); //read total number of integers from input file
	
    //compute chunk size
    c = n/p; 
	if (n%p) 
	  c++;
	
    data = (unsigned int *)malloc(p*c*size);
    for (i = 0; i < n; i++)
	  fread(&data[i], size, 1, file); //read data from input file
	  
    fclose(file); //close input ifle

    for (i = n; i < p*c; i++)
      data[i] = 0; //pad rest of the data variable with 0
  }
  
  //MPI_Barrier(MPI_COMM_WORLD);
  
  t1 = MPI_Wtime(); //start the timer

  MPI_Bcast(&n, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD); //Rank 0 process broadcasts the total number of integers to all the processes.

  //compute chunk size
  c = n/p; 
  if (n%p) 
	c++;

  //Rank 0 process scatters each chunk of data to each process. Rank 0 process will also work on a chunk of data.
  chunk = (unsigned int *)malloc(c * size);
  MPI_Scatter(data, c, MPI_UNSIGNED, chunk, c, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
  free(data);
  data = NULL;

  //each process computes size of its own chunk
  if (n >= c * (id+1))
    s = c;
  else
    s = n - c * id;

  quicksort(chunk, 0, s); //each process sorts their own chunk

  //the sorted chunks are merged in log_2 p steps
  for (step = 1; step < p; step = 2*step) {
    if (id % (2*step)) {
      //if rank of current process is not a multiple of 2*step, then its sorted chunk is sent to id-step.
      MPI_Send(chunk, s, MPI_UNSIGNED, id-step, 0, MPI_COMM_WORLD);
      break;	
    }
    //if rank of current process is a multiple of 2*step, then merge its sorted chunk with the sorted chunk of id+step (if it exists)
    if (id+step < p) {
      //compute size of chunk to be received
      if (n >= c * (id+2*step))
        o = c * step;
      else
        o = n - c * (id+step);
   
      other = (unsigned int *)malloc(o * size);
      MPI_Recv(other, o, MPI_UNSIGNED, id+step, 0, MPI_COMM_WORLD, &status); //receive the sorted chunk from id+step
      data = merge(chunk, s, other, o);
      free(chunk);
      free(other);
      chunk = data;
      s = s + o;
    }
  }

  
  t2 = MPI_Wtime(); //stop the timer
  elapsed_time = t2 - t1; //calculate elapsed time

  //write sorted data to the output file
  if (id == 0) {
    file = fopen(argv[2], "w");
	fwrite(&s, size, 1, file);
    for (i = 0; i < s; i++)
	{
	  fwrite(&chunk[i], size, 1, file);	  
	}
    fclose(file);
	//t2 = MPI_Wtime(); //stop the timer
	//elapsed_time = t2 - t1; //calculate elapsed time	
    printf("Quicksort %u unsigned integers on %d processes in %f secs.\n", s, p, elapsed_time);
  }

  MPI_Finalize();
  return 0;
}