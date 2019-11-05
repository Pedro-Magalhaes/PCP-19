#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include <mpi.h>
#include <string.h>
#include <sys/sysinfo.h>


#define MAX_COST 999999
#define NO_CITY -1
#define true 1
#define false 0
typedef int bool;

// Estruturas  -----------------------------------------------------------------
typedef struct Tour_Struct {
    int cost;
    int* cities;
    int count;
} Tour;
typedef Tour* tour_t;

typedef struct DEQueue_Struct {
    int head;
    int tail;
    tour_t* tours;
}  DEQueue;

typedef struct Stack_Struct {
    int size;
    int max_size;
	tour_t* tours;
} Stack;
typedef Stack* my_stack_t
// -----------------------------------------------------------------------------

// Variaveis Globais -----------------------------------------------------------
int n; // the total number of cities in the problem
int* digraph; // data structure representing the input digraph
const int hometown = 0; // data structure representing vertex or city 0, the salesperson’s hometown
tour_t best_tour; // data structure representing the best tour so far

int* threads_tasks_numbers; // armazena o numero de tasks de cada thread
int num_cpu;
// -----------------------------------------------------------------------------


int main(int argc, char* argv[]) {

    if (argc < 2 + 1) {
        printf("Erro, os parametros necessários não foram informados.\
    				\nO Programa recebe os parametros nesta ordem:\
    				\n\t<numero de cidades> \
                    \n\t<arquivo com matriz>\n");
        printf(" Numero de parametros recebidos: %d\n", argc);
        exit(-1);
    }

    n = atoi(argv[1]);
    char *file_path = argv[2];

	read_matrix_file(file_path);
    print_matrix(digraph); //[TODO] delete print

	num_cpu = get_nprocs(); // to get max number of threads in each process

	// Init best tour
	best_tour = malloc(sizeof(tour_t));
	best_tour->cities = malloc(sizeof(int) * (n+1)); // n+1 beacuse last city must be hometown
	best_tour->count = 0;
	best_tour->cost = MAX_COST;

    free(digraph);
    return 0;
}


void TreeSearch(void* num) {

	// [TODO] BFS non-rec
	long threadID = (long)num;
	int num_of_tasks = threads_tasks_numbers[threadID];
	Push_copy(stack, tour); TODO
	while (!Empty_stack(&stack)) {
		tour_t curr_tour = Pop(stack);
		int City_count = curr_tour->count;
		if (City_count == n) {
			if (Best_tour(curr_tour)) {
				printf("[TODO] create update best tour"); //[TODO] delete line
				// Update_best_tour(curr_tour);
			}
		} else {
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
}


void set_tasks(my_stack_t stack) {
	threads_tasks_numbers = malloc(num_cpu * sizeof(int));
	int num_of_tasks = (stack->size)/num_cpu;
	for(int i = 0; i < num_cpu; i++) {
		threads_tasks_numbers[i] = num_of_tasks;
	}
	for(int i=0; i < ((stack->size)%num_cpu); i++) {
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
            	fprintf(stderr, "[Error] Index matrix[%d][%d] must be 0\n");
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

void Push(my_stack_t stack, int city) {
	int loc = stack->size;
    if (stack->max_size == stack->size) {
        printf("[Error - Push]: Stack is already full\n");
        exit(-1);
    }
    stack->tours[loc] = city;
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

bool Empty_stack(my_stack_t stack) {
    return stack->size == 0 ? true : false;
}

void Free_tour(tour_t tour) {
	free(tour->cities);
	free(tour);
}

bool Best_tour(tour_t tour) {

	/* Create function that checks MPI messages to look for the best tour across all processes */
    // check_best_tours(); [TODO]

    int last_city = get_last_city(tour);
	int current_cost = tour->cost;
    int new_cost = tour->cost + compute_cost(last_city, hometown);
    return (newCost < best_tour->cost ? true : false);
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
