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
int total_prime_prod;

void buffer_init(int tamanho_buffer, int num_produtores, int num_consumidores) {
    sem_init(&empty, 1, 1);
    sem_init(&full, 1, 0);
    
    //TODO
    sem_init(&rw, 1, 0); 
    buff.buffer_W_offset = 0;
    //TODO
    
    buff.size = tamanho_buffer;
    buff.data = (int*) malloc(sizeof(int) * buff.size);
    buff.buffer_R_offset = (int*) malloc(sizeof(int) * buff.size);
    buff.num_reads = (int*) malloc(sizeof(int) * buff.size); //TODO: Garantir, na 1a iteracao, que nenhum reader vai tentar ler um posição que não tenha sido escrita por nenhum writer ainda 
    if(buff.data == NULL || buff.buffer_R_offset == NULL || buff.num_reads == NULL) {
        printf("Erro inicializando buffer, erro de malloc");
        exit(-1);
    }
    total_consumidores = num_consumidores;
    total_produtores = num_produtores;
}

void buffer_max_readers(int consumers_total_prod) {
    total_prime_prod = consumers_total_prod;
}

/* 
    Consumidores. 
    precisamos atomicamente incrementar o contador do buffer[i]
    quando buffer[i] == total_consumidores a memoria pode ser sobrescrita
*/
int consome(long meuid) {
	/*
    int data;
    int offset = buff.buffer_R_offset[meuid];
    buff.buffer_R_offset[meuid] = (offset+1)%buff.size; // não precisa ser atomico pois cada thread tem 1 offset
    <await ((buff.prod_reads[offset] % meuid) != 0)
    data = buff.data[offset];
    buff.num_reads[offset] = buff.num_reads[offset] * meuid; >
    */
    
    int data;
    
    printf("Entered if Consome - Thread [%ld]\n", meuid);
    
    int offset = buff.buffer_R_offset[meuid];
    buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset
    if ((buff.num_reads[offset] % meuid) != 0) {
    	printf("Entered if Consome\n");
    
   		sem_wait(&rw); //P(rw)
    	data = buff.data[offset];
	    buff.num_reads[offset] = buff.num_reads[offset] * meuid;
	    sem_post(&rw); //V(rw)
	    printf("Consumidor - Leu: %d\n",data);
    }
    return total_prime_prod; //FIXME usar retorno real do que foi lido
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
    
    // TODO como produtor sabe que deve ficar esperando offset resetar ou deve ir para prox espaço vazio
    
    int offset;
    offset = buff.buffer_W_offset;
    if (buff.num_reads[offset] == total_prime_prod) {
    	printf("Entered if Deposita\n");
    
		buff.buffer_W_offset++;
	    printf("Produtor - Escreveu: %d\n",item);
    	sem_wait(&rw); //P(rw)
    	buff.data[offset] = item;
    	buff.num_reads[offset] = 1;
    	sem_post(&rw); //V(rw)
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

/*
	TODO: Verificar se os printfs deveriam ficar dentro das funções 'consome/deposita' para indicar precisamente o momento em que a thread realmente acessou a regiao critica
*/

