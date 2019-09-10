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

sem_t rw;
sem_t empty;
sem_t full;
Buffer buff;

int total_consumidores = 0;
int total_produtores = 0;
long int total_prime_prod = 1;

void buffer_init(int tamanho_buffer, int num_produtores, int num_consumidores) {
    sem_init(&empty, 1, 1);
    sem_init(&full, 1, 0);
    
    buff.buffer_W_offset = 0; //writers starts at first poisition
    sem_init(&rw, 1, 1); //sem rw needs to be initialized with 1, so writers starts before readers 
    
    buff.size = tamanho_buffer;
    buff.data = (int*) malloc(sizeof(int) * buff.size);
    buff.buffer_R_offset = (int*) malloc(sizeof(int) * buff.size);
    buff.num_reads = (int*) malloc(sizeof(int) * buff.size);
    if(buff.data == NULL || buff.buffer_R_offset == NULL || buff.num_reads == NULL) {
        printf("Erro inicializando buffer, erro de malloc");
        exit(-1);
    }
    total_consumidores = num_consumidores;
    total_produtores = num_produtores;
    
    // Calcula o produto total dos números primos associados aos Consumidores
    for(int i=0; i < num_consumidores; i++) {
    	total_prime_prod *= prime_numbers[i];
	}
	
	// Inicializa todas as posições do vetor 'num_reads' com o total do produto, para garantir que nenhum consumidor tentará ler algo que ainda não foi escrito na 1a iteração
	for(int i=0; i< tamanho_buffer; i++) {
		buff.num_reads[i] = total_prime_prod;
	}
}


/* 
    Consumidores. 
    precisamos atomicamente incrementar o contador do buffer[i]
    quando buffer[i] == total_consumidores a memoria pode ser sobrescrita
*/
int consome(int meuid) {
	/*
    int data;
    int offset = buff.buffer_R_offset[meuid];
    buff.buffer_R_offset[meuid] = (offset+1)%buff.size; // não precisa ser atomico pois cada thread tem 1 offset
    <await ((buff.prod_reads[offset] % meuid) != 0)
    data = buff.data[offset];
    buff.num_reads[offset] = buff.num_reads[offset] * meuid; >
    */
    
    int data = 0;
    int num_primo = prime_numbers[meuid];
    int offset = buff.buffer_R_offset[meuid];
    buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset
    
    
    if ((buff.num_reads[offset] % num_primo) != 0) {
    	num_primo = prime_numbers[meuid];	
   		
   		sem_wait(&rw); //P(rw)
   		printf(" Thread Cons[%d] - P(rw)\n",meuid); 
   		
    	data = buff.data[offset];
	    buff.num_reads[offset] = buff.num_reads[offset] * num_primo;
	    
	    sem_post(&rw); //V(rw)
   		printf(" Thread Cons[%d] - V(rw)\n",meuid); 

	    //printf("Thread - C[%d] Num-Primo = %d, Consumiu: %d\n",meuid, num_primo, data);
    }
    return data; 
}


/*
    Escritores.
    Só podem escrever quando buffer[i]==total_consumidores, setando buffer[i]==0

*/
void deposita(int item) {
    /*
    <int offset = buffer_W_offset++;>
    <await (buff.num_reads[offset] == total_prod_consumidores)
    buff.data[offset] = item
    buff.num_reads[offset] = 1>
    */
    
    // TODO como produtor sabe que deve ficar esperando offset resetar ou deve ir para prox espaço vazio? 
    int offset;
    offset = buff.buffer_W_offset;
    
    //printf("Entered Deposita - item = %d - offset = %d\n",item, offset);
    
    if (buff.num_reads[offset] == total_prime_prod) {
    
		buff.buffer_W_offset++;
		if (buff.buffer_W_offset >= buff.size) {
			buff.buffer_W_offset = 0;
		}
		
	    //printf("Thread - P[%d] escreveu: %d\n",item); //TODO Como pegar o ID da thread sem alterar a interface da função 'deposita'?
		printf("Thread escreveu: %d\n",item);
		
    	sem_wait(&rw); //P(rw)
    	printf(" Thread Prod[n] - P(rw)\n"); 
    	
    	buff.data[offset] = item;
    	buff.num_reads[offset] = 1;
    	
    	sem_post(&rw); //V(rw)
    	printf(" Thread Prod[n] - V(rw)\n");
    }
}


void free_buffer() {
    printf("liberando memória\n");
    free(buff.buffer_R_offset);
    free(buff.data);
    sem_destroy(&rw);
    sem_destroy(&empty);
    sem_destroy(&full);
}

//TODO Delete unused Prints! 
