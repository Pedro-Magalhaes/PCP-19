#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include <mpi.h>
#include <string.h>

#define MAX_COST 999999

// Global vars -----------------------------------------------------------------
int* digraph;
// -----------------------------------------------------------------------------

// Estruturas  -----------------------------------------------------------------
typedef int city_t;
typedef int cost_t;
typedef struct tour_struct {
    int custo;
    int* cidades;
    int n_cidades;
} tour;

typedef struct DEQueue_struct {
    int head;
    int tail;
    tour** list;
}  dequeue;
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

    int n = atoi(argv[1]);
    char *file_path = argv[2];

	read_matrix_file(file_path, n);
    print_matrix(digraph, n);

    free(digraph);
    return 0;
}

void read_matrix_file(char* file_path, int n) {
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

void print_matrix(int* digraph, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int offset = i*n+j;
            printf("%5d ", digraph[offset]);
        }
        printf("\n");
    }
}
