#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "main.h"
#include <time.h>

#define MAX_STRING 10000
#define MAX_COST 999999
#define NO_CITY -1
#define true 1
#define false 0
// typedef int bool;

// Estruturas  -----------------------------------------------------------------
typedef struct Tour_Struct {
    int cost;
    int* cities;
    int count;
} Tour;

typedef struct DEQueue_Struct {
    int head;
    int tail;
    tour_t* tours;
    int max_size;
    bool full;
}  DEQueue;

typedef struct Stack_Struct {
    int size;
    int max_size;
    int head;  // ws [HERE]
	tour_t* tours;
} Stack;
// -----------------------------------------------------------------------------

// Variaveis Globais -----------------------------------------------------------
int n; // the total number of cities in the problem
int* digraph; // data structure representing the input digraph
const int hometown = 0; // data structure representing vertex or city 0, the salesperson’s hometown
tour_t best_tour; // data structure representing the best tour so far
int best_tour_cost;

int* threads_tasks_numbers; // armazena o numero de tasks de cada thread
int thread_count;

pthread_mutex_t best_tour_mutex;

// [TODO] Initialize Variables appropriately
// MPI_Comm comm;
int cluster_count;
int cluster_rank;
// int my_rank;
// int comm_sz;

const int TOUR_TAG = 1;
// const int TOUR_TAG = 3;
// -----------------------------------------------------------------------------

typedef struct barrier_struct {
   int curr_tc;  // Number of threads that have entered the barrier
   int max_tc;   // Number of threads that need to enter the barrier
   pthread_mutex_t mutex;
   pthread_cond_t ok_to_go;
}  Barrier;


my_barrier_t bar_str;
int init_tour_count;
int queue_size;

my_queue_t queue;
my_stack_t* bigStack;


// "Macros" --------------------------------------------------------------------
int Tour_cost(tour_t tour) { return (tour->cost); }
int Tour_count(tour_t t) { return t->count; }
// -----------------------------------------------------------------------------

// "MPI" --------------------------------------------------------------------
int comm_rank;
int comm_sz;

// -----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    char *file_path;
    clock_t start, end;
    double time_spent;
    long thread;
    pthread_t* thread_handles;

    // Variaveis MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);


    if (argc < 2 + 1) {
        printf("Erro, os parametros necessários não foram informados.\
    				\nO Programa recebe os parametros nesta ordem:\
    				\n\t<numero de cidades> \
                    \n\t<arquivo com matriz>\n");
        printf(" Numero de parametros recebidos: %d\n", argc);
        exit(-1);
    }

    n = atoi(argv[1]);
    file_path = argv[2];

	read_matrix_file(file_path);
    // print_matrix(digraph); //[TODO] delete print

    // thread_count = get_nprocs(); [TODO] -use it // to get max number of threads in each process
    thread_count = strtol(argv[3], NULL, 4);
    printf("Number of threads: %d\n\n", thread_count);

	// Init best tour
	best_tour = Alloc_tour();
	best_tour->count = 0;
	best_tour->cost = MAX_COST;
    best_tour_cost = Tour_cost(best_tour);

    bigStack = malloc(thread_count*sizeof(my_stack_t));
    for(long i=0; i<thread_count ;i++){
        bigStack[i] = NULL;
    }

    thread_handles = malloc(thread_count*sizeof(pthread_t));
    bar_str = My_barrier_init(thread_count);

    start = clock();
    for (int id = 0; id < thread_count; id++) {
        pthread_create(&thread_handles[id], NULL, Thread_tree_search, (void*) id);
    }

    for (int id = 0; id < thread_count; id++) {
        pthread_join(thread_handles[id], NULL);
    }
    end = clock();

   time_spent = (double)(end - start) / (CLOCKS_PER_SEC*thread_count);
   // time_spent = (double)(end - start);
   Print_winner(best_tour);
   printf("\nBest tour: ");
   Print_tour(best_tour);
   printf("Cost = %d\n", best_tour->cost);
   printf("Time = %e s\n", time_spent);

   free(best_tour->cities);
   free(best_tour);
   free(threads_tasks_numbers);
   free(thread_handles);
   free(digraph);
   My_barrier_destroy(bar_str);
   pthread_mutex_destroy(&best_tour_mutex);

   MPI_Finalize();

   return 0;
}


void* Thread_tree_search(void* rank) {
	long id = (long)rank;
	// int num_of_tasks = threads_tasks_numbers[id]; //[TODO]
    tour_t curr_tour;

    my_stack_t stack = Init_stack(); //[TODO] Get right stack

    bigStack[id] = stack;
    Print_stack(stack, id, "Init stack"); // del print

    Partition_tree(id, stack);

    // DFS
    while (!Empty_stack(stack)) {
		curr_tour = Pop(stack);
		if (Tour_count(curr_tour) == n) {
			if (Best_tour(curr_tour)) {
				Update_best_tour(curr_tour);
			}
		}
        else {
			for (int nbr = n-1; nbr >= 1; nbr--) {
				if (Feasible(curr_tour, nbr)) {
					Add_city(curr_tour, nbr);
					Push_copy(stack, curr_tour);
					Remove_last_city(curr_tour);
				}
			}
		}
		Free_tour(curr_tour);
	}
    Free_stack(stack);
    return NULL;
}


bool Terminated(my_stack_t stack, long my_rank) {
    if (!Empty_stack(stack)) {
        // not done yet
        return false;
    }

    for(long id=0; id < thread_count; id++) {
        if (id != my_rank) {
            if(bigStack[id]->size > 1) {
                // There's work to steal
                printf("\n\t\tT[%ld] will try to steal work from T[%ld]\n\n",my_rank, id);
                tour_t tStolen = WorkSteal(bigStack[id], id);
                Push(stack, tStolen);
                return false;
            }
        }
    }
    printf("T[%ld] - Terminating, stack size = %d\n", my_rank, stack->size);
    return true;
}

tour_t PopBottom(my_stack_t stack, long my_rank) {
    tour_t tmp;
    if (stack->size < 2) {
        fprintf(stderr, "Can't steal work from T[%ld]\n", my_rank);
        exit(-1);
    }
    tmp = stack->tours[stack->head];
    stack->head = (stack->head + 1) % stack->max_size;
    return tmp;
}


void set_tasks(my_stack_t stack) {
	threads_tasks_numbers = malloc(thread_count * sizeof(int));
	int num_of_tasks = (stack->size)/thread_count;
	for(int i = 0; i < thread_count; i++) {
		threads_tasks_numbers[i] = num_of_tasks;
	}
	for(int i=0; i < ((stack->size)%thread_count); i++) {
		threads_tasks_numbers[i]++;
	}
}


void read_matrix_file(char* file_path) {
    int i, j;
    FILE* matrix_file = fopen(file_path, "r");
	if (matrix_file == NULL) {
		fprintf(stderr, "[Error] Null file: %s\n", file_path);
        exit(-1);
	}
    digraph = malloc(n*n*sizeof(int));
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int offset = i*n+j;
            fscanf(matrix_file, "%d", &digraph[offset]);
            if (i == j && digraph[offset] != 0) {
            	fprintf(stderr, "[Error] Index matrix[%d][%d] must be 0\n",i,j);
              	exit(-1);
            }
        }
    }
    fclose(matrix_file);
}

void print_matrix(int* digraph) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int offset = i*n+j;
            printf("%5d ", digraph[offset]);
        }
        printf("\n");
    }
}


/*
    The function Feasible checks to see if the city or vertex has already been
    visited, and, if not, whether it can possibly lead to a least-cost tour. If
    the city is feasible, we add it to the tour, and call Depth first search
*/
bool Feasible(tour_t tour, int city) {
    int last_city = (tour->cities[(tour->count)-1]);
    for (int i = 0; i < tour->count; i++) {
        if (tour->cities[i] == city ) {
            // ja foi visitado
            return false;
        }
    }
    int tmp_cost = tour->cost + compute_cost(last_city, city);
    if (tmp_cost < best_tour->cost) {
        return true;
    } else {
        return false;
    }
}

void Add_city(tour_t tour, int city) {
    int tmp_last_city = tour->cities[(tour->count)-1];
    tour->cities[tour->count] = city;
    (tour->count)++;
    tour->cost = tour->cost + compute_cost(tmp_last_city, city);
}

void Remove_last_city(tour_t tour) {
    int curr_last_city, tmp_last_city;
    tmp_last_city = get_last_city(tour);
    tour->cities[tour->count-1] = NO_CITY;
    (tour->count)--;
    curr_last_city = get_last_city(tour);
    tour->cost = tour->cost - compute_cost(curr_last_city, tmp_last_city);
}

// Calcula custo da cidade c1 para c2
int compute_cost(int c1, int c2) {
    int offset = c1*n+c2;
    return digraph[offset];
}

int get_last_city(tour_t t) {
    return t->cities[(t->count)-1];
}

// Pag 305 - cap 6.2.3 - Data structures for the serial implementations
/* Find the ith city on the partial tour */
int Tour_city(tour_t tour, int i) {
	return tour->cities[i];
	//[TODO] try macro later:
	// #define Tour_city(tour, i) (tour->cities[i])
}

tour_t Pop(my_stack_t stack) {
    if (Empty_stack(stack)) {
		printf("[Error - Pop]: Stack is empty\n");
        exit(-1);
    }
	int offset = (stack->size) - 1;
    tour_t tmp = stack->tours[offset];
    stack->tours[offset] = NULL;
    (stack->size)--;
    return tmp;
}

void Push(my_stack_t stack, tour_t tour) {
	int loc = stack->size;
    if (stack->max_size == stack->size) {
        printf("[Error - Push]: Stack is already full\n");
        exit(-1);
    }
    stack->tours[loc] = tour;
    (stack->size)++;
}

void Push_copy(my_stack_t stack, tour_t tour) {
	int loc = stack->size;
    if (stack->max_size == stack->size) {
		printf("[Error - Push_copy]: Stack is already full\n");
        exit(-1);
    }
    tour_t tmp = Alloc_tour();
    Copy_tour(tour, tmp);
    stack->tours[loc] = tmp;
    (stack->size)++;
}

tour_t Alloc_tour() {
	tour_t t = malloc(sizeof(tour_t));
	t->cities = malloc(sizeof(int) * (n+1)); // n+1 beacuse last city must be hometown
	return t;
}

// Stack -----------------------------------------------------------------------
bool Empty_stack(my_stack_t stack) {
    return stack->size == 0 ? true : false;
}

my_stack_t Init_stack(void) {
    my_stack_t stack = malloc(sizeof(Stack));
    stack->tours = malloc(n*n*sizeof(tour_t));
    for (int i=0; i<(n*n); i++) {
        stack->tours[i] = NULL;
    }
    stack->size = 0;
    stack->max_size = n*n;

    stack->head = 0; //[ws here]

    return stack;
}

void Free_stack(my_stack_t stack) {
    for (int i=0; i < stack->size; i++) {
        free(stack->tours[i]->cities);
        free(stack->tours[i]);
    }
    free(stack->tours);
    free(stack);
}

tour_t WorkSteal(my_stack_t stack, long my_rank) {
   tour_t tmp;
   if (stack->size < 2) {
      fprintf(stderr, "Can't steal work from T[%ld], go find your own job!\n", my_rank);
      exit(-1);
   }
   tmp = stack->tours[stack->head];
   stack->head = (stack->head + 1) % stack->max_size;
   (stack->size)--;
   return tmp;
}  /* Pop */
// -----------------------------------------------------------------------------

void Free_tour(tour_t tour) {
	free(tour->cities);
	free(tour);
}

bool Best_tour(tour_t tour) {

	/* Create function that checks MPI messages to look for the best tour across all processes */
    // check_best_tours(); [TODO]

    int last_city = get_last_city(tour);
	int current_cost = tour->cost;
    int new_cost = current_cost + compute_cost(last_city, hometown);
    return (new_cost < best_tour->cost ? true : false);
}

void Copy_tour(tour_t tour1, tour_t tour2) {
    memcpy(tour2->cities, tour1->cities, (n+1)*sizeof(int));
    tour2->count = tour1->count;
    tour2->cost = tour1->cost;
}

void Update_best_tour(tour_t tour) {
    // We have to use a mutex to avoid race Condition
    pthread_mutex_lock(&best_tour_mutex);

    /* We’ve already checked Besttour, but we need to check itagain */
    if (Best_tour(tour)) {
        Copy_tour(tour, best_tour);
        Add_city(best_tour, hometown);
        best_tour_cost = Tour_cost(best_tour);

        // Boradcast asynchronously Best_cost to everyone
        // Broeadcast_best_cost_to_all(best_tour_cost); //[later]
    }
    pthread_mutex_unlock(&best_tour_mutex);
} /* [NOTE] Checking Best_tour() twice may seem wasteful, but if updates to the
     best tour are infrequent, then most ofthe timeBesttourwill returnfalseand it
     will only be rarely necessary to makethe “double” call */


void Broeadcast_best_cost_to_all(int tour_cost) {
    for (int dest = 0; dest < cluster_count; dest++) {
        if (dest != cluster_rank) {
            // MPI_Bsend(&tour_cost, 1, MPI_INT, dest, TOUR_TAG, comm);
            // MPI_Bsend(&tour_cost, 1, MPI_INT, dest, TOUR_TAG, MPI_COMM_WORLD);
        }
    }
    // best_costs_bcast++; ??
}

void Print_tour(tour_t tour) {
    // printf("Process %d: ", my_rank); [TODO] add clusters rank
    printf("Tour: ");
    for (int i = 0; i < (tour->count)-1; i++) {
        printf("%d -> ", Tour_city(tour, i));
    }
    printf("%d\n\n", Tour_city(tour, (tour->count)-1));
}

void Print_winner(tour_t tour) {
    printf("\nBest tour, fullpath:\n");
    printf("city1 -> city2 : [cost] : [total] \n");
    int total_cost=0;
    for (int i = 0; i < (tour->count)-1; i++) {
        int c1 = Tour_city(tour, i);
        int c2 = Tour_city(tour, i+1);
        int cost = compute_cost(c1,c2);
        total_cost += cost;
        printf("%2d -> %2d  : %3d : %3d\n", c1, c2, cost, total_cost);
    }
}

// void Print_stack(my_stack_t stack, long pthrdID, char* title) {
//     // printf("(Rank %ld) - thread (%ld) - %s\n", cluster_rank, pthrdID, title);  //[later]
//     printf("\n\tSTACK PRINT\n"); // delete print [TODO]
//     printf("\tT[%ld] - %s\n", pthrdID, title);
//     printf("\tCurrent_size = %d : Max_size = %d \n", stack->size, stack->max_size);
//     for (int i = 0; i < stack->size; i++) {
//         int sz = stack->tours[i]->count;
//         printf("\t[%d] - ", i);
//         Print_tour(stack->tours[i]);
//     //     for (int j = 0; j < sz-1; j++) {
//     //         printf("%d -> ", stack->tours[i]->cities[j]);
//     //     }
//     //     printf("%d\n\n", stack->tours[i]->cities[sz-1]);
//     }
// }
void Print_stack(my_stack_t stack, long my_rank, char title[]) {
    char string[MAX_STRING];
    printf("\n\tSTACK PRINT\n"); // delete print [TODO]
    printf("\tT[%ld] - %s\n", my_rank, title);
    for (int i = 0; i < stack->size; i++) {
        sprintf(string, "\tT[%ld] - Tour[%d]: ", my_rank, i);
        for (int j = 0; j < stack->tours[i]->count; j++) {
            sprintf(string + strlen(string), "%d ", stack->tours[i]->cities[j]);
        }
        printf("%s\n", string);
    }
}
// [TODO] MPI functions Receive/Send (pag 330)
/*
void Send tour(tour t tour, int dest) {
	int position = 0;
	MPI Pack(tour->cities, n+1, MPI INT, contig buf, LARGE, &position, comm);
	MPI Pack(&tour->count, 1, MPI INT, contig buf, LARGE, &position, comm);
	MPI Pack(&tour->cost, 1, MPI INT, contig buf, LARGE, &position, comm);
	MPI Send(contig buf, position, MPI PACKED, dest, 0, comm);
}

void Receive tour(tour t tour, int src) {
	int position = 0;
	MPI Recv(contig buf, LARGE, MPI PACKED, src, 0, comm, MPI STATUS IGNORE);
	MPI Unpack(contig buf, LARGE, &position, tour->cities, n+1, MPI INT, comm);
	MPI Unpack(contig buf, LARGE, &position, &tour->count, 1, MPI INT, comm);
	MPI Unpack(contig buf, LARGE, &position, &tour->cost, 1, MPI INT, comm);
}
*/

void Partition_tree(long my_rank, my_stack_t stack) {
   int my_first_tour, my_last_tour, i;

   if (my_rank == 0) queue_size = Get_upper_bd_queue_sz();
   // My_barrier(bar_str); // My_barrier [HERE]
   printf("T[%ld] > queue_size = %d\n", my_rank, queue_size);
   if (queue_size == 0) pthread_exit(NULL);

   if (my_rank == 0) Build_initial_queue();
   // My_barrier(bar_str); // My_barrier [HERE]
   Set_init_tours(my_rank, &my_first_tour, &my_last_tour);
   printf("T[%ld] > init_tour_count = %d, first = %d, last = %d\n",
         my_rank, init_tour_count, my_first_tour, my_last_tour);
   for (i = my_last_tour; i >= my_first_tour; i--) {
      Print_tour(getDEQsTour(queue,i));
      Push(stack, getDEQsTour(queue,i));
   }
   Print_stack(stack, my_rank, "After set up");
}

int Get_upper_bd_queue_sz(void) {
   int fact = n-1;
   int size = n-1;

   while (size < thread_count) {
      fact++;
      size *= fact;
   }
   return size;
}

// My_barrieir [HERE]
// void My_barrier(my_barrier_t bar) {
//     pthread_mutex_lock(&bar->mutex);
//     bar->curr_tc++;
//     if (bar->curr_tc == bar->max_tc) {
//         bar->curr_tc = 0;
//         pthread_cond_broadcast(&bar->ok_to_go);
//     }
//     else {
//         // Wait unlocks mutex and puts thread to sleep.
//         //    Put wait in while loop in case some other
//         // event awakens thread.
//         while (pthread_cond_wait(&bar->ok_to_go, &bar->mutex) != 0);
//         // Mutex is relocked at this point.
//     }
//     pthread_mutex_unlock(&bar->mutex);
// }

void Build_initial_queue(void) {
   int curr_sz = 0;
   int nbr;
   tour_t tour = Alloc_tour(NULL);

   Init_tour(tour, 0);
   queue = Init_queue(2*queue_size);

   /* Breadth-first search */
   Push_Bottom_DEQ(queue, tour);  // Enqueues a copy
// printf("Freeing %p\n", tour);
   Free_tour(tour);
   curr_sz++;
   while (curr_sz < thread_count) {
      tour = Pop_Top_DEQ(queue);
//    printf("Dequeued %p\n", tour);
      curr_sz--;
      for (nbr = 1; nbr < n; nbr++)
         if (!wasVisited(tour, nbr)) {
            Add_city(tour, nbr);
            Push_Bottom_DEQ(queue, tour);
            curr_sz++;
            Remove_last_city(tour);
         }
//    printf("Freeing %p\n", tour);
      Free_tour(tour);
   }  /* while */
   init_tour_count = curr_sz;

// # ifdef DEBUG
   Print_queue(queue, 0, "Initial queue");
// # endif
}


my_barrier_t My_barrier_init(int thr_count) {
    my_barrier_t bar = malloc(sizeof(Barrier));
    bar->curr_tc = 0;
    bar->max_tc = thr_count;
    pthread_mutex_init(&bar->mutex, NULL);
    pthread_cond_init(&bar->ok_to_go, NULL);
    return bar;
}

void My_barrier_destroy(my_barrier_t bar) {
    pthread_mutex_destroy(&bar->mutex);
    pthread_cond_destroy(&bar->ok_to_go);
    free(bar);
}

void Set_init_tours(long my_rank, int* my_first_tour_p, int* my_last_tour_p) {
    int quotient, remainder, my_count;

    quotient = init_tour_count/thread_count;
    remainder = init_tour_count % thread_count;
    if (my_rank < remainder) {
        my_count = quotient+1;
        *my_first_tour_p = my_rank*my_count;
    }
    else {
        my_count = quotient;
        *my_first_tour_p = my_rank*my_count + remainder;
    }
    *my_last_tour_p = *my_first_tour_p + my_count - 1;
}

tour_t getDEQsTour(my_queue_t queue, int i) {
    int top = queue->head + i;
    int offset = top % queue->max_size;
    return (queue->tours[offset]);
}

// ----------------------------------------------------------------------------

void Init_tour(tour_t tour, int cost) {
    tour->cities[0] = 0;
    for (int i = 1; i <= n; i++) {
        tour->cities[i] = NO_CITY;
    }
    tour->cost = cost;
    tour->count = 1;
}

my_queue_t Init_queue(int size) {
   my_queue_t new_queue = malloc(sizeof(DEQueue));
   new_queue->tours = malloc(size*sizeof(tour_t));
   new_queue->max_size = size;
   new_queue->head = new_queue->tail = new_queue->full = 0;

   // Print_queue(queue, 0, "Init queue"); // PRINT QUEUE

   return new_queue;
}

// Remove the tour at the head of the queue
tour_t Pop_Top_DEQ(my_queue_t queue) {
    tour_t tmp;
    if (isQueueEmpty(queue)) {
        exit(-1);
        fprintf(stderr, "[Error]: Can't pop from a empty queue\n");
    }
    tmp = queue->tours[queue->head];
    queue->head = (queue->head + 1) % queue->max_size;

    // Print_queue(queue, 0, "Pop queue"); // PRINT QUEUE

    return tmp;
}

// Add a new tour to the tail of the queue
void Push_Bottom_DEQ(my_queue_t queue, tour_t tour) {
    tour_t tmp;
    if (queue->full == true) {
        printf(stderr, "[Error]: Can't push into a full queue\n");
        printf(stderr, "Max_size = %d, head = %d, tail = %d\n", \
               queue->max_size, queue->head, queue->tail);
        exit(-1);
    }
    tmp = Alloc_tour(NULL);
    Copy_tour(tour, tmp);
    queue->tours[queue->tail] = tmp;
    queue->tail = (queue->tail + 1) % queue->max_size;
    if (queue->tail == queue->head) {
        queue->full = true;
    }
    // Print_queue(queue, 0, "Push queue"); // PRINT QUEUE

}

bool wasVisited(tour_t tour, int city) {
    for(int i = 0; i < Tour_count(tour); i++) {
        if (Tour_city(tour,i) == city ) {
            return true;
        }
    }
   return false;
}

bool isQueueEmpty(my_queue_t queue) {
    if (queue->full == true) {
        return false;
    }
    else if (queue->head != queue->tail) {
        return false;
    }
    else {
        return true;
    }
}


void Print_queue(my_queue_t queue, long id, char* title) {
    char string[1000];
    printf("Queue: id > title : %ld > %s\n", id, title);
    for (int i = queue->head; i != queue->tail; i = (i+1) % queue->max_size) {
        sprintf(string, "Queue: id > tours : %ld > %p = ", id, queue->tours[i]);
        for (int j = 0; j < queue->tours[i]->count; j++) {
            sprintf(string + strlen(string), "%d ", queue->tours[i]->cities[j]);
        }
        printf("%s\n", string);
    }
}
