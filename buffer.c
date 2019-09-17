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

sem_t *wait_write; // bloquear leitores apressados (quando tiverem dado a volta no buffer)
sem_t *wait_read; // bloquear escritores apressados (quando tiverem dado a volta no buffer)

// Used semaphores
sem_t ge; // exclusão global
sem_t e; // exclusão pra cada pos do buffer
sem_t r; // delay para  reader
sem_t w; // delay para writer

// Used counters
int nr, nw;
int dr, dw;

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

  bloked_readers = (int*) malloc(sizeof(int) * buff.size);
  bloked_writers = (int*) malloc(sizeof(int) * buff.size);
  wait_write = (sem_t*) malloc(sizeof(sem_t) * buff.size);
  wait_read = (sem_t*) malloc(sizeof(sem_t) * buff.size);
  if (wait_write == NULL
    // || r == NULL || w == NULL || e == NULL || nr == NULL || nw == NULL || dr == NULL || dw == NULL ||
      || bloked_writers == NULL || wait_read == NULL) {
    printf("Erro inicializando buffer, erro de malloc");
    exit(-1);
  }

  sem_init(&e, 0, 1);
  sem_init(&r, 0, 0);
  sem_init(&w, 0, 0);
  sem_init(&ge, 0, 1);

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
	for(int i=0; i < tamanho_buffer; i++) {
		buff.num_reads[i] = total_prime_prod;
    bloked_readers[i] = 0;
    bloked_writers[i] = 0;
    sem_init(&wait_write[i], 1, 0);
	}
}


/*
Andrews Notation:
  <  await ( (buff.num_reads[offset[id]] % prime_numbers[meuid]) != 0 && nw == 0 && dr == (total_consumidores - 1))  nr++;  >
  data = buff.data[offset[id]];
  <  buff.num_reads[offset[id]] = buff.num_reads[offset[id]] * meuid;  >
  offset[id] = (offset[id] + 1) % buff.size;
  <  nc--;  >
  SIGNAL
*/

int consome(int meuid) {
  int data = 0;
  int num_primo = prime_numbers[meuid];
  int offset = buff.buffer_R_offset[meuid];
  int resto;

  // < await ( (buff.num_reads[offset[id]] % prime_numbers[meuid]) != 0 && nw == 0 && dr == (total_consumidores - 1))  nr++; >
  sem_wait(&ge);
  resto = (buff.num_reads[offset] % num_primo);
  if(resto == 0 && nw > 0 && dr < (total_consumidores - 1)) {
    dr++;
    sem_post(&ge);
    sem_wait(&r);
  }
  nr++;
  sem_post(&ge);

  data = buff.data[offset]; // Read
  printf("Consumer<%d>, leu: %d\n",meuid, data);

  // < buff.num_reads[offset[id]] = buff.num_reads[offset[id]] * meuid; >
  sem_wait(&ge);
  buff.num_reads[offset] = buff.num_reads[offset] * num_primo;
  sem_post(&ge);

  buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset

  // < nr--; >
  sem_wait(&ge);
  nr--;

  //  SIGNAL
  if (nr == 0 && dw > 0 && buff.num_reads[offset] == total_prime_prod) {
    dw--;
    sem_post(&w);
  }
  else {
    sem_post(&ge);
  }
  return data;
}


/*
Andrews Notation
  < await (buff.num_reads[offset] == total_prod_consumidores && nc == 0 && nw == 0) nw++; >
  buff.data[offset] = item;
  < buff.num_reads[offset] = 1
  offset = (offset + 1) % buff.size;
  --nw; >
  SIGNAL
*/
void deposita(int id, int item) {
  int offset;

  // < await (buff.num_reads[offset] == total_prod_consumidores && nc == 0 && nw == 0) nw++; >
  sem_wait(&ge);
  offset = buff.buffer_W_offset;
  if(buff.num_reads[offset] < total_prime_prod && nr > 0 && nw > 0) {
    dw++;
    sem_post(&ge);
    sem_wait(&w);
  }
  nw++;
  sem_post(&ge);

  buff.data[offset] = item; // Write

  // < buff.num_reads[offset] = 1
  // offset = (offset + 1) % buff.size;
  // --nw; >
  sem_wait(&ge);
  printf("Producer<%d>, depositou: %d, na posicao: [%d] +\n", id, item, offset);
  buff.num_reads[offset] = 1;
  buff.buffer_W_offset = (buff.buffer_W_offset + 1) % buff.size;
  nw--;

  // SIGNAL
  if (dr > 0 && dr == total_consumidores) {
    dr--;
    sem_post(&r);
  } else if (dw > 0  && buff.num_reads[offset] < total_prime_prod) {
    dw--;
    sem_post(&w);
  } else {
    sem_post(&ge);
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
    sem_destroy(&e);
    sem_destroy(&w);
    sem_destroy(&r);
    for(int i=0; i< buff.size; i++) {
        // sem_destroy(&w);
        // sem_destroy(&r);
        // sem_destroy(&e);
        sem_destroy(&wait_write[i]);
        sem_destroy(&wait_read[i]);
    }
    // free(w);
    // free(r);
    // free(e);
    free(wait_write);
    free(wait_read);
}

//TODO Delete unused Prints!
//TODO delete unused vars and old comments
