#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include <mpi.h>
#include <string.h>

#define MAX_COST 999999
#define NO_CITY -1
#define true 1
#define false 0
typedef int bool;

// Estruturas  -----------------------------------------------------------------
typedef struct Tour_Struct {
    int cost;
    int* cities;
    int num_cities;
} Tour;

typedef struct DEQueue_Struct {
    int head;
    int tail;
    Tour** list;
}  DEQueue;
// -----------------------------------------------------------------------------

// Variaveis Globais -----------------------------------------------------------
int* digraph;
Tour* best_tour;
int n; // number of cities
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


    // [TODO] BFS non-rec
    // Push_copy(stack, tour); TODO
    // while (!Empty(&stack)) {
    while (true) {
        // curr_tour = Pop(stack);
        Tour* curr_tour = 0;
        int City_count = curr_tour->num_cities;
        if (City_count == n) {
            printf("[TODO] check best_tour and update"); //[TODO] delete line
            /* TODO
            if (Best_tour(curr_tour)) {
                Update_best_tour(curr_tour);
            }
            */
        } else {
            for (int nbr = n-1; nbr >= 1; nbr--) {
                if (Feasible(curr_tour, nbr)) {
                    Add_city(curr_tour, nbr);
                    // Push_copy(stack, curr_tour); TODO
                    Remove_last_city(curr_tour);
                }
            }
        }
        // Free_tour(curr_tour); [TODO]
    }

    free(digraph);
    return 0;
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
bool Feasible(Tour* tour, int city) {
    int last_city = (tour->cities[(tour->num_cities)-1]);
    for (int i = 0; i < tour->num_cities; i++) {
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

void Add_city(Tour* tour, int city) {
    int tmp_last_city = tour->cities[(tour->num_cities)-1];
    tour->cities[tour->num_cities] = city;
    (tour->num_cities)++;
    tour->cost = tour->cost + compute_cost(tmp_last_city, city);
}

void Remove_last_city(Tour* tour) {
    int curr_last_city, tmp_last_city;
    tmp_last_city = get_last_city(tour);
    tour->cities[tour->num_cities-1] = NO_CITY;
    (tour->num_cities)--;
    curr_last_city = get_last_city(tour);
    tour->cost = tour->cost - compute_cost(curr_last_city, tmp_last_city);
}

// Calcula custo da cidade c1 para c2
int compute_cost(int c1, int c2) {
    int offset = c1*n+c2;
    return digraph[offset];
}

int get_last_city(Tour* t) {
    return t->cities[(t->num_cities)-1];
}
