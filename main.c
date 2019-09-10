#include <stdio.h>

#include <stdlib.h>

#include <pthread.h>

#include <semaphore.h>

#include "buffer.h"

void * Produtor(void * );
void * Consumidor(void * );

int C, P, N, nIters;

sem_t sem_dado;
int cont_dado;

int main(int argc, char * argv[]) {
    long i;
    pthread_t * producer_id, * consumer_id;

    pthread_attr_t attr; /* descriptors */
    pthread_attr_init( & attr);
    pthread_attr_setscope( & attr, PTHREAD_SCOPE_SYSTEM);

    long int consumers_total_prod = 1;

    if (argc < 4 + 1) {
        printf("Erro, os parametros necessários não foram informados.\
		\nO Programa recebe os parametros nesta ordem:\
		\n\t<numero de consumidores>\n\t<numero de produtores\
		\n\t<tamanho do buffer>\n\t<numero de iterações>\n");
        printf(" Numero de parametros recebidos: %d\n", argc);
        exit(-1);
    }

    C = atoi(argv[1]);
    P = atoi(argv[2]);
    N = atoi(argv[3]);
    nIters = atoi(argv[4]);

    // inicializa o buffer
    buffer_init(C, P, N);

    sem_init( & sem_dado, 1, 1);
    cont_dado = 1;

    // aloca espaço para os processos consumidores e produtores
    consumer_id = malloc(sizeof(pthread_t) * C);
    producer_id = malloc(sizeof(pthread_t) * P);

    // cria os consumidores
    for (i = 0; i < C; i++) {
        pthread_create( & consumer_id[i], & attr, Consumidor, (void * ) i);
    }

    // cria os produtores
    for (i = 0; i < P; i++) {
        pthread_create( & producer_id[i], & attr, Produtor, (void * ) i);
    }

    // espera até os processos terminarem
    for (i = 0; i < C; i++) {
        pthread_join(consumer_id[i], NULL);
    }

    for (i = 0; i < P; i++) {
        pthread_join(producer_id[i], NULL);
    }
    free_buffer();
    return 0;
}

void * Produtor(void * arg) {
    int id = (int) arg;
    for (int i = 0; i < nIters * N; i++) {
        int dado = i + id * P;
        //printf("<%d> Produtor - Escreveu: %d\n",id,dado);
		//deposita(dado, id);
		deposita(dado);
    }
}

void *Consumidor(void *arg) {
    int id = (int)arg;
    for (int i=0; i<nIters*N /* *P */; i++) {
        int dado = consome(id);
        //printf("<%d> Consumidor - Leu: %d\n",id,dado);
	}
}
