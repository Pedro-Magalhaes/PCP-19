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


sem_t ew; // exclusão de escritores
sem_t rw;
sem_t *empty;
sem_t *full;
Buffer buff;

int total_consumidores = 0;
int total_produtores = 0;
long int total_prime_prod = 1;

void buffer_init(int num_consumidores, int num_produtores, int tamanho_buffer) {



    sem_init(&ew, 1, 1);
   
    buff.buffer_W_offset = 0; //writers starts at first poisition
    sem_init(&rw, 1, 1); //sem rw needs to be initialized with 1, so writers starts before readers 
    
    buff.size = tamanho_buffer;
    buff.data = (int*) malloc(sizeof(int) * buff.size);
    buff.buffer_R_offset = (int*) malloc(sizeof(int) * buff.size);
    buff.num_reads = (int*) malloc(sizeof(int) * buff.size);

    empty = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    full = (sem_t*) malloc(sizeof(sem_t) * buff.size);

    if(buff.data == NULL || buff.buffer_R_offset == NULL || buff.num_reads == NULL
        || full == NULL || empty == NULL) {
        printf("Erro inicializando buffer, erro de malloc");
        exit(-1);
    }
    total_consumidores = num_consumidores;
    total_produtores = num_produtores;
    
    // Calcula o produto total dos números primos associados aos Consumidores
    for(int i=0; i < num_consumidores; i++) {
    	total_prime_prod *= prime_numbers[i];
	}
	printf("total prime prod: %d \n",total_prime_prod);
	// Inicializa todas as posições do vetor 'num_reads' com o total do produto, para garantir que nenhum consumidor tentará ler algo que ainda não foi escrito na 1a iteração
	for(int i=0; i< tamanho_buffer; i++) {
		buff.num_reads[i] = total_prime_prod;
        sem_init(&empty[i], 1, 1);
        sem_init(&full[i], 1, 0);
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
    // printf(" Thread Cons[%d] -inicio\n",meuid); 
    int data = 0;
    int num_primo = prime_numbers[meuid];
    int offset = buff.buffer_R_offset[meuid];
    buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset
    // printf("consumer pegando ew, off: %d\n",offset);
    
    while(1) {
        sem_wait(&ew); //P(ew)
        // printf("consumer PEGOU ew, off: %d\n",offset);
        if((buff.num_reads[offset] % num_primo) == 0) {
            // printf("consumer id %d, no off %d, aguardando escrita. meu primo:%d\n",meuid,offset,num_primo);
            sem_post(&ew); //V(ew)

        // if(buff.num_reads[offset] == total_prime_prod) {
        //     sem_post(&empty[offset]);
        // }
        // sem_wait(&full[offset]);
        } else {
            break;
        }
    }
    
    
    num_primo = prime_numbers[meuid];	
    
    
    // printf(" Thread Cons[%d] - P(rw)\n",meuid); 
    
    data = buff.data[offset];
    //sem_wait(&ew);
    buff.num_reads[offset] = buff.num_reads[offset] * num_primo;
    sem_post(&ew);
    if(buff.num_reads[offset] == total_prime_prod) {
        // printf("Preso aqui?\n");
        sem_post(&empty[offset]);
    } 
    sem_post(&rw); //V(rw)
    // printf(" Thread Cons[%d] - V(rw)\n",meuid); 

    //printf("Thread - C[%d] Num-Primo = %d, Consumiu: %d\n",meuid, num_primo, data);

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
    // printf(" deposita inicio - item: %d\n",item); 
    
    // TODO como produtor sabe que deve ficar esperando offset resetar ou deve ir para prox espaço vazio? 
    int offset;
    sem_wait(&rw);
    offset = buff.buffer_W_offset;
    buff.buffer_W_offset = (buff.buffer_W_offset + 1) % buff.size;
    sem_post(&rw);
    //printf("Entered Deposita - item = %d - offset = %d\n",item, offset);
    
    sem_wait(&ew);
    if (buff.num_reads[offset] < total_prime_prod) {
        // printf("deposita aguradando.\n");
        sem_post(&ew);
        sem_wait(&empty[offset]);
    }
    //printf("Thread - P[%d] escreveu: %d\n",item); //TODO Como pegar o ID da thread sem alterar a interface da função 'deposita'?
    // printf("Thread escreveu: %d na pos %d\n",item, offset);
    
    //sem_wait(&ew); //P(rw)
    // printf(" Thread Prod[%d] - P(rw)\n",offset); 
    
    buff.data[offset] = item;
    buff.num_reads[offset] = 1;
    sem_post(&ew); //V(rw)
    
    // printf("prendendo o empty[%d]\n",offset);
    sem_trywait(&empty[offset]);
    
    // printf(" Thread Prod[n] - V(rw)\n");
    
}


void free_buffer() {
    printf("liberando memória\n");
    free(buff.buffer_R_offset);
    free(buff.data);
    sem_destroy(&rw);
    for(int i=0; i< buff.size; i++) {
        sem_destroy(&empty[i]);
        sem_destroy(&full[i]);
    }
}

//TODO Delete unused Prints! 
