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
sem_t *r; // delay para  reader
sem_t *w; // delay para writer

int *nr, *nw;
int *dr, *dw;
int *bloked_readers;  // reader bloqueado por já ter lido informação do buff[i], aguarda nova escrita

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
    wait_write = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    r = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    w = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    e = (sem_t*) malloc(sizeof(sem_t) * buff.size);
    if (nr == NULL || nw == NULL || dr == NULL || dw == NULL
        || wait_write == NULL || r == NULL || w == NULL || e == NULL) {
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
        nr[i],nw[i],dr[i],dw[i],bloked_readers[i] = 0;
        sem_init(&wait_write[i], 1, 0);
        sem_init(&e[i], 1, 1);
        sem_init(&r[i], 1, 0);
        sem_init(&w[i], 1, 1);
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
    sem_wait(&ge);
    
    int data = 0;
    int num_primo = prime_numbers[meuid];
    int offset = buff.buffer_R_offset[meuid];
    
    // printf("consumer pegando ew, ofs: %d\n",offset);
    buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset
    // printf("consumer pegando ew, ofs: %d\n",offset);
    sem_post(&ge);

    sem_wait(&e[offset]); //P(ew)
    // printf("consumer PEGOU e[%d]\n",offset);
    if((buff.num_reads[offset] % num_primo) == 0) {
        // printf("consumer id %d, no off %d, aguardando escrita. meu primo:%d\n",meuid,offset,num_primo);
        bloked_readers[offset]++;
        sem_post(&e[offset]); //V(e)
        sem_wait(&wait_write[offset]);
    } else {
        sem_post(&e[offset]); //V(e)
    }
    // printf(" Thread Cons[%d] -depois do wait_write\n",meuid); 
    // sem_wait(&e[offset]);
    if(nw[offset] > 0) {
        dr[offset]++;
        sem_post(&e[offset]); //V(e)
        sem_wait(&r[offset]);
    }
    
    nr[offset]++;

    if(dr[offset] > 0) {
        dr[offset]--;
        sem_post(&r[offset]);
    } else {
        sem_post(&e[offset]);
    }

    data = buff.data[offset];
    buff.num_reads[offset] = buff.num_reads[offset] * num_primo;
    printf("CONSUMER<%d> leu %d da pos %d\n",meuid,data,offset);

    sem_wait(&e[offset]);
    nr[offset]--;
    
    if(nr[offset] == 0 && bloked_readers[offset] > 0) {
            bloked_readers[offset]--;
            sem_post(&wait_write[offset]);
    } else if(nr[offset] == 0 && dw[offset] > 0 && buff.num_reads[offset] >= total_prime_prod) {
        dw[offset]--;
        sem_post(&w[offset]);
    } else {
        sem_post(&e[offset]);
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
    // printf(" deposita inicio - item: %d\n",item); 
    
    // TODO como produtor sabe que deve ficar esperando offset resetar ou deve ir para prox espaço vazio? 
    int offset;
    sem_wait(&ge);
    offset = buff.buffer_W_offset;
    buff.buffer_W_offset = (buff.buffer_W_offset + 1) % buff.size;
    sem_post(&ge);
    // printf("Entered Deposita - item = %d - offset = %d\n",item, offset);
    
    sem_wait(&e[offset]);
    if (buff.num_reads[offset] < total_prime_prod || nr[offset] > 0) {
        // printf("deposita aguradando total_prime_prod == total || nr.\n");
        dw[offset]++;
        sem_post(&e[offset]);
        sem_wait(&w[offset]);
    }
    // printf("deposita escrevendo.\n");
    nw[offset]++;
    sem_post(&e[offset]);
    buff.data[offset] = item;
    buff.num_reads[offset] = 1;
    printf("dado <%d> inserido na pos %d do buff \n",item,offset);
    sem_wait(&e[offset]);
    nw[offset]--;
    if(bloked_readers[offset] > 0) {
        // printf("deposita soltando blocked.\n");
        bloked_readers[offset]--;
        sem_post(&wait_write[offset]);
    } else if(dr[offset] > 0) {
        // printf("deposita soltando delayed R.\n");
        dr[offset]--;
        sem_post(&r[offset]);
    } else if(dw[offset] > 0) { // provavelmente nao precisa pela exclisão no offset de w
        // printf("deposita soltando delayed W.\n");
        dw[offset]--;
        sem_post(&w[offset]);
    } else {
        // printf("deposita soltando exclusao.\n");
        sem_post(&e[offset]);
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
    }
    free(w);
    free(r);
    free(e);
    free(wait_write);
}

//TODO Delete unused Prints! 
