#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct Buffers {
    int buffer_W_offset; //posição "corrente" de escrita
    int *buffer_R_offset; // array com a posição de leitura de cada consumidor
    int *data;
    int *num_reads; // numero total de leituras da posição da memória
    int size; // tamanho do buffer
} Buffer;


sem_t empty;
sem_t full;
Buffer buff;

int total_consumidores = 0;
int total_produtores = 0;

void buffer_init(int tamanho_buffer, int num_produtores, int num_consumidores) {
    sem_init(&empty, 1, 1);
    sem_init(&full, 1, 0);
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
        <await buff.num_reads[offset] < total_consumidores;
        data = buff.data[offset];
        buff.num_reads[offset]++; >
    */

}


/*
    Escritores.
    Só podem escrever quando buffer[i]==total_consumidores, setando buffer[i]==0

*/
void deposita(int item) {
    /*
    <int offset = buffer_W_offset++;>
    <await (buff.num_reads[offset] == total_consumidores)
    buff.data=item
    buff.num_reads[offset] = 0>
    */
}




void free_buffer() {
    printf("liberando memória\n");
    free(buff.buffer_R_offset);
    free(buff.data);
}