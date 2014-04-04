/* File: qsort_par.c (Reagan Lopez for CS515)
**
** Quicksort algorithm (Parallel version).
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define MIN_SIZE 10

pthread_t *thread_pool = NULL;
struct task_queue *queue;
pthread_mutex_t queue_lock, count_lock;
pthread_cond_t cond1;
int N, count_sort = -1;
int *array = NULL;

struct task_node
{
    int low;
	int high;
    struct task_node *next;
};

struct task_queue
{
    struct task_node *head;
	struct task_node *tail;
};

/* 
 * Checks if queue is empty 
 */
int isEmpty(struct task_queue *queue)
{    
	if ((queue->head == NULL) && (queue->tail == NULL))
		return 1;
	else
		return 0;
}


/* 
 * Inserts a task into queue 
 */
void push_queue(struct task_queue *queue, struct task_node *node)
{
    if (isEmpty(queue)) 
	{
		
        queue->head = node;
        queue->tail = node;	
	    
	    
    } 
	else 
	{
        queue->tail->next = node;
        queue->tail = node;
    }
		pthread_cond_signal(&cond1);
//		printf("signal by rank = \n");


}


/* 
 * Removes a task from the queue 
 */
struct task_node* pop_queue(struct task_queue *queue)
{
    if (isEmpty(queue))
	{
        return NULL;
    }
    struct task_node *head = queue->head;
	if (queue->head == queue->tail)	
	{
		queue->head = NULL;
		queue->tail = NULL;
	}
	else	
	{
		queue->head = head->next;
	}
    return head;
}


/* 
 * Swap two array elements 
 */
void swap(int a, int b)
{
    if (a == b) return;
    int tmp = array[a];
    array[a] = array[b];
    array[b] = tmp;
}


/* 
 * Bubble sort for the base cases
 */
void bubble_sort(int low, int high)
{
    if (low > high) return;
    int i, j;
    for (i = low; i <= high; i++)
        for (j = i+1; j <= high; j++) 
            if (array[i] > array[j])
                swap(i, j);
}


/* 
 * Partition the array into two halves and return the middle index
 */
int partition(int low, int high)
{
    /* Use the lowest element as pivot */
    int pivot = array[low], middle = low, i;

    swap(low, high);
    for(i=low; i<high; i++) {
        if(array[i] < pivot) {
            swap(i, middle);
            middle++;
        }
    }
    swap(high, middle);
 
    return middle;
}

/*
 * Quicksort the array 
 */
void quicksort(int low, int high)
{
    if (high - low < MIN_SIZE) {
        bubble_sort(low, high);
		
        // critical section - increase the sort counter
        pthread_mutex_lock(&count_lock);
        count_sort += high - low + 2;        
        pthread_mutex_unlock(&count_lock);
        
        return;
    }
	
	// partition the array into first and second segment.
    int middle = partition(low, high);
    
	// create a new node for the first segment.
    struct task_node *new_node = (struct task_node *)malloc(sizeof(struct task_node));
    new_node->low = low;
    new_node->high = middle - 1;
	new_node->next = NULL;
    
    // critical section - place the first segment unto the task queue.
    pthread_mutex_lock(&queue_lock);    
    push_queue(queue, new_node); 	
    pthread_mutex_unlock(&queue_lock);    
    
	// recurse only on the second segment.
    quicksort(middle+1, high);
}

/*
 * Every thread executes this routine, including 'main'. 
 */
void worker(long rank)
{
	#ifdef DEBUG
		printf("Thread %ld starts ...\n", rank);
	#endif
	
       pthread_mutex_lock(&queue_lock);
//	   printf("lock obtained by rank = %ld\n", rank);
        
       //if queue is empty then wait                         
       while (isEmpty(queue) && count_sort < N) {
                 pthread_cond_wait(&cond1,&queue_lock);
		//		 printf("waiting by rank = %ld\n", rank);
		}
                 
       //if queue is not empty
       while (!isEmpty(queue) && count_sort < N) {
              struct task_node *node = pop_queue(queue);
              pthread_mutex_unlock(&queue_lock);
//			  printf("unlock by rank = %ld\n", rank);
              quicksort(node->low, node->high);
			  
 			  #ifdef DEBUG
			  printf("%ld.",(long)rank);
			  #endif 			  
			  pthread_mutex_lock(&queue_lock);
//			  printf("lock obtained by rank = %ld\n", rank);
           }
		   pthread_mutex_unlock(&queue_lock);
//		   printf("unlock by rank = %ld\n", rank);
//		   printf("count = %d\n", count_sort);
      
   
} 

/* 
 * Main routine for testing quicksort
 */
int main(int argc, char **argv)
{
	int num_threads, i, j;
	
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
		printf("<num_threads> not specified. Defaulting <num_threads> to 1\n");
		num_threads = 1;
	}  
	
	if ((argc > 2) && (num_threads=atoi(argv[2])) < 1) {
		 printf("<num_threads> must not be < 1. Defaulting <num_threads> to 1\n");
		 num_threads = 1;
	} 

    // initialize the array and the task queue;
    array = (int *)malloc(sizeof(int) * N);
    for (i = 0; i < N; i++)	{
        array[i] = i + 1;
	}
    queue = (struct task_queue *)malloc(sizeof(struct task_queue));
	queue->head = NULL;
	queue->tail = NULL;
    struct task_node *initial_node = (struct task_node *)malloc(sizeof(struct task_node));
    initial_node->low = 0;
    initial_node->high = N - 1;
	initial_node->next = NULL;
	/*
	queue->head = initial_node;
    queue->tail = initial_node;
	*/
	push_queue(queue, initial_node);
    
    // randomly permute array elements
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        j = (rand()*1./RAND_MAX)*(N - 1);
        swap(i, j);
    }
    pthread_mutex_init(&queue_lock, NULL);
    pthread_mutex_init(&count_lock, NULL);
	pthread_cond_init(&cond1, NULL);   
    
    // create (num_threads-1) threads, each will execute the worker() routine;
    thread_pool = (pthread_t *)malloc(sizeof(pthread_t)*(num_threads));
    for (i = 0; i < num_threads - 1; i++) {
		pthread_create(&thread_pool[i], NULL, (void *) &worker, (void *)i);
	}

    // 'main' is also a member of the thread pool
    worker(num_threads - 1);
    // wait for other threads to join;
    for (i = 0; i < num_threads; i++) {
		pthread_join(thread_pool[i], NULL);
//		printf("joined %d", i);
	}

    // verify results;
    for (i=0; i<N-1; i++) {
        if (array[i]>array[i+1]) {
            printf("Verification failed, array[%d]=%d, array[%d]=%d\n",
                   i, array[i], i+1, array[i+1]);
            return;
        }

    }
    printf("\nSorting result verified! (cnt = %d)\n", count_sort);
}