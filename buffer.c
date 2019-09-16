#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct Buffers {
    int buffer_W_offset; //posição "corrente" de escrita
    int *buffer_R_offset; // array com a posição de leitura de cada consumidor
    int *data;
    int *num_reads; // numero total de leituras da posição da memória (identificada pelo produto das IDs dos consumidores que a leram, até o momento)
    int size; // tamanho do buffer
} Buffer;

sem_t ge; // exclusão global
sem_t *e; // exclusão pra cada pos do buffer
sem_t *wait_write; // bloquear leitores apressados (quando tiverem dado a volta no buffer)
sem_t *wait_read; // bloquear escritores apressados (quando tiverem dado a volta no buffer)
sem_t *r; // delay para  reader
sem_t *w; // delay para writer

int *nr, *nw;
int *dr, *dw;
int *bloked_readers;  // reader bloqueado por já ter lido informação do buff[i], aguarda nova escrita
int *bloked_writers;  // writer bloqueado por que leitores ainda não leram dado da posição

Buffer buff;

int total_consumidores = 0;
int total_produtores = 0;
long int total_prime_prod = 1;

void buffer_init(int num_consumidores, int num_produtores, int tamanho_buffer) {

    buff.size = tamanho_buffer;
    total_consumidores = num_consumidores;
    total_produtores = num_produtores;

    nr = (int*) malloc(sizeof(int) * buff.size);
    nw = (int*) malloc(sizeof(int) * buff.size);
    dr = (int*) malloc(sizeof(int) * buff.size);
    dw = (int*) malloc(sizeof(int) * buff.size);
    bloked_readers = (int*) malloc(sizeof(int) * buff.size);
    bloked_writers = (int*) malloc(sizeof(int) * buff.size);
    wait_write = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    wait_read = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    r = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    w = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    e = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    if (nr == NULL || nw == NULL || dr == NULL || dw == NULL
        || wait_write == NULL || r == NULL || w == NULL || e == NULL
        || bloked_writers == NULL || wait_read == NULL) {
        printf("Erro inicializando buffer, erro de malloc");
        exit(-1);
    }

    sem_init(&ge, 1, 1);

    buff.buffer_W_offset = 0; //writers starts at first poisition

    buff.data = (int*) malloc(sizeof(int) * buff.size);
    buff.buffer_R_offset = (int*) malloc(sizeof(int) * num_consumidores);
    buff.num_reads = (int*) malloc(sizeof(int) * buff.size);

    if(buff.data == NULL || buff.buffer_R_offset == NULL || buff.num_reads == NULL) {
        printf("Erro inicializando buffer, erro de malloc");
        exit(-1);
    }

    // Calcula o produto total dos números primos associados aos Consumidores
    for(int i=0; i < num_consumidores; i++) {
    	total_prime_prod *= prime_numbers[i];
        buff.buffer_R_offset[i] = 0;
	}
    buff.buffer_R_offset[0] = 0;
	// Inicializa todas as posições do vetor 'num_reads' com o total do produto, para garantir que nenhum consumidor tentará ler algo que ainda não foi escrito na 1a iteração
	for(int i=0; i< tamanho_buffer; i++) {
		buff.num_reads[i] = total_prime_prod;
        nr[i],nw[i],dr[i],dw[i],bloked_readers[i],bloked_writers[i] = 0;
        sem_init(&wait_write[i], 1, 0);
        sem_init(&e[i], 1, 1);
        sem_init(&r[i], 1, 0);
        sem_init(&w[i], 1, 1);
	}
}


int consome(int meuid) {
  int data = 0;
  int num_primo = prime_numbers[meuid];
  int offset = buff.buffer_R_offset[meuid];
  int resto;

  sem_wait(&ge);  //TODO check e ge
  resto = (buff.num_reads[offset] % num_primo);
  // TODO check
  // if (produced[next_data[id]] == FALSE || np > 0 || consumer_delay_size < (con_size - 1)) {
  // if (resto == 0 || nw[offset] > 0 || dr[offset] < (total_consumidores - 1)) {
  if (resto == 0 || nw[offset] > 0) {
    dr[offset] += 1;
    sem_post(&ge);  //TODO check e ge
    sem_wait(&r[offset]);
  }
  nr[offset] += 1;

  if(dr[offset] > 0) {
    dr[offset] -= 1;
    sem_post(&ge);  //TODO check e ge
  }
  else {
    sem_post(&ge);  //TODO check e ge
  }

  data = buff.data[offset];
  printf("Thread - C[%d], leu: %d \t| NumPrimo = %d \t | TotalProd[%d] = %d\n",meuid, data, num_primo, offset, buff.num_reads[offset]);
  // printf("Consumer<%d>, leu: %d, posicao: [%d]\n",meuid, data, offset);

  sem_wait(&ge);  //TODO check e ge
  buff.num_reads[offset] = buff.num_reads[offset] * num_primo;

  buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset
  nr[offset] -= 1;
  if (nr[offset] == 0 && dw[offset] > 0 && buff.num_reads[offset] == total_prime_prod) {
    dw[offset] -= 1;
    sem_post(&w[offset]);
  }
  else {
    sem_post(&ge);  //TODO check e ge
  }
  return data;
}


void deposita(int id, int item) {
  int offset;
  sem_wait(&ge);
  offset = buff.buffer_W_offset;
  buff.buffer_W_offset = (buff.buffer_W_offset + 1) % buff.size;
  sem_post(&ge);

  sem_wait(&ge); //TODO check e ge
  if (buff.num_reads[offset] < total_prime_prod || nr[offset] > 0 || nw > 0) {
    dw[offset] += 1;
    sem_post(&ge); //TODO check e ge
    sem_wait(&w[offset]);
  }
  nw[offset] += 1;
  sem_post(&ge); //TODO check e ge

  buff.data[offset] = item;

  sem_wait(&ge); //TODO check e ge
  printf("Producer<%d>, depositou: %d, na posicao: [%d]\n", id, item, offset);

  if(dr[offset] == total_consumidores) {
    dr[offset] -= 1;
    sem_post(&r[offset]);
  }
  else if (dw[offset] > 0 && buff.num_reads[offset] == total_prime_prod) {
    dw[offset] -= 1;
    sem_post(&w[offset]);
  }
  else {
    sem_post(&ge);  //TODO check e ge
  }

}


void free_buffer() {
    printf("liberando memória\n");
    free(buff.buffer_R_offset);
    free(buff.data);
    free(nr);
    free(nw);
    free(dr);
    free(dw);
    free(bloked_readers);
    sem_destroy(&ge);
    for(int i=0; i< buff.size; i++) {
        sem_destroy(&w[i]);
        sem_destroy(&r[i]);
        sem_destroy(&e[i]);
        sem_destroy(&wait_write[i]);
        sem_destroy(&wait_read[i]);
    }
    free(w);
    free(r);
    free(e);
    free(wait_write);
    free(wait_read);
}

//TODO Delete unused Prints!
