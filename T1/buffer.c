#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct Buffers {
    int buffer_W_offset; //posição "corrente" de escrita
    int *buffer_R_offset; // array com a posição de leitura de cada consumidor
    int *data;
    long unsigned int *num_reads; // numero total de leituras da posição da memória (identificada pelo produto das IDs dos consumidores que a leram, até o momento)
    int size; // tamanho do buffer
} Buffer;

// Semaphores
sem_t ge; // exclusão global
sem_t r; // delay para  reader
sem_t w; // delay para writer

// Counters
int nr, nw; // number of active readers / writers
int dr, dw; // number of delayed readers / writers

Buffer buff;

int total_consumidores = 0;
int total_produtores = 0;
long unsigned int total_prime_prod = 1;

void buffer_init(int num_consumidores, int num_produtores, int tamanho_buffer) {

  buff.size = tamanho_buffer;
  total_consumidores = num_consumidores;
  total_produtores = num_produtores;

  sem_init(&r, 0, 0); // used to delay readers
  sem_init(&w, 0, 0); // used to delay writers
  sem_init(&ge, 0, 1); // controls entry to critical sections

  buff.buffer_W_offset = 0; //writers starts at first poisition

  buff.data = (int*) malloc(sizeof(int) * buff.size);
  buff.buffer_R_offset = (int*) malloc(sizeof(int) * num_consumidores);
  buff.num_reads = (long unsigned int*) malloc(sizeof(long unsigned int) * buff.size);

  if(buff.data == NULL || buff.buffer_R_offset == NULL || buff.num_reads == NULL) {
    printf("Erro inicializando buffer, erro de malloc");
    exit(-1);
  }

  // Calcula o produto total dos números primos associados aos Consumidores
  for(int i=0; i < num_consumidores; i++) {
  	total_prime_prod *= prime_numbers[i];
    buff.buffer_R_offset[i] = 0;
  }

	// Inicializa todas as posições do vetor 'num_reads' com o total do produto, para garantir que nenhum consumidor tentará ler algo que ainda não foi escrito na 1a iteração
	for(int i=0; i < tamanho_buffer; i++) {
		buff.num_reads[i] = total_prime_prod;
	}
}


/*
Andrews Notation:
  <  await ( (buff.num_reads[offset[id]] % prime_numbers[meuid]) != 0 && nw == 0 && dr == (total_consumidores - 1))  nr++;  >
  data = buff.data[offset[id]];
  <  buff.num_reads[offset[id]] = buff.num_reads[offset[id]] * meuid;  >
  offset[id] = (offset[id] + 1) % buff.size;
  <  nr--;  >
  SIGNAL
*/
int consome(int meuid) {
  int data = -1;
  unsigned int num_primo = prime_numbers[meuid];
  int offset = buff.buffer_R_offset[meuid];
  int resto;

  // < await ( (buff.num_reads[offset[id]] % prime_numbers[meuid]) != 0 && nw == 0 && dr == (total_consumidores - 1))  nr++; >
  sem_wait(&ge);
  resto = (buff.num_reads[offset] % num_primo);
  if(resto == 0 || nw > 0 || dr < (total_consumidores - 1)) {
    dr++;
    sem_post(&ge);
    sem_wait(&r);
  }
  nr++;

  if(dr > 0) {
    dr--;
    sem_post(&r);
  } else {
    sem_post(&ge);
  }

  data = buff.data[offset]; // Read

  // < buff.num_reads[offset[id]] = buff.num_reads[offset[id]] * meuid; >
  sem_wait(&ge);
  printf("Consumer<%d>, leu: %d\n",meuid, buff.data[offset]);
  buff.num_reads[offset] = buff.num_reads[offset] * num_primo;
  // sem_post(&ge);

  buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset

  // < nr--; >
  // sem_wait(&ge);
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
  < await (buff.num_reads[offset] == total_prod_consumidores && nr == 0 && nw == 0) nw++; >
  buff.data[offset] = item;
  < buff.num_reads[offset] = 1
  offset = (offset + 1) % buff.size;
  nw--; >
  SIGNAL
*/
void deposita(int item) {

  // < await (buff.num_reads[offset] == total_prod_consumidores && nr == 0 && nw == 0) nw++; >
  sem_wait(&ge);

  if(buff.num_reads[buff.buffer_W_offset] < total_prime_prod || nr > 0 || nw > 0) {
    dw++;
    sem_post(&ge);
    sem_wait(&w);
  }
  nw++;
  sem_post(&ge);

  buff.data[buff.buffer_W_offset] = item; // Write

  // < buff.num_reads[offset] = 1
  // offset = (offset + 1) % buff.size;
  // nw--; >
  sem_wait(&ge);
  printf("\t\tProducer, depositou: %d, na posicao: [%d] +\n", item, buff.buffer_W_offset);
  buff.num_reads[buff.buffer_W_offset] = 1;
  buff.buffer_W_offset = (buff.buffer_W_offset + 1) % buff.size;
  nw--;

  // SIGNAL
  if (dr > 0 && dr == total_consumidores) {
    dr--;
    sem_post(&r);
  } else if (dw > 0  && buff.num_reads[buff.buffer_W_offset] == total_prime_prod) {
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
    free(buff.num_reads);
    sem_destroy(&ge);
    sem_destroy(&w);
    sem_destroy(&r);
}

//TODO Delete unused Prints!
//TODO delete unused vars and old comments
