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
    
    <await ((buff.prod_reads[offset] % meuid) != 0)>
    <await (nr == 0 && nw == 0)>
    data = buff.data[offset];
    buff.num_reads[offset] = buff.num_reads[offset] * meuid; >
    
    SIGNAL
    */
    
    
    
    int data = 0;
    int num_primo = prime_numbers[meuid];
    int offset = buff.buffer_R_offset[meuid];
    int resto;
    
    buff.buffer_R_offset[meuid] = (offset+1) % buff.size; // não precisa ser atomico pois cada thread tem 1 offset

    //<await ((buff.prod_reads[offset] % meuid) != 0)>
    sem_wait(&e[offset]); 
    resto = buff.num_reads[offset] % num_primo;
    if(resto == 0) {
        // printf("consumer id %d, no off %d, aguardando escrita. meu primo:%d\n",meuid,offset,num_primo);
        bloked_readers[offset]++;
        sem_post(&e[offset]); //V(e)
        sem_wait(&wait_write[offset]);
    } else {
        sem_post(&e[offset]); //V(e)
    }

    //<await (nr == 0 && nw == 0)>
    if(nw[offset] > 0 || nr[offset > 0]) {
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

    printf("CONSUMER<%d> leu %d da pos %d\n",meuid,data, offset);

    sem_wait(&e[offset]);
    nr[offset]--;
    
    //SIGNAL
    if(nr[offset] == 0 && bloked_readers[offset] > 0) {
            // printf("passando pro br\n");
            bloked_readers[offset]--;
            sem_post(&wait_write[offset]);
    } else if (buff.num_reads[offset] >= total_prime_prod && bloked_writers[offset] > 0) {
           bloked_writers--;
           sem_post(&wait_read[offset]);
    } else if(nr[offset] == 0 && dw[offset] > 0  ) {
        // printf("passando pro w\n");
        dw[offset]--;
        sem_post(&w[offset]);
    } else {
        // printf("liberando exclusao\n");
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
    <await (buff.num_reads[offset] == total_prod_consumidores)>
    <await (nr == 0 && nw == 0)
    buff.data[offset] = item
    buff.num_reads[offset] = 1>

    SIGNAL
    */
    // printf(" deposita inicio - item: %d\n",item); 
    
    
    // <int offset = buffer_W_offset++;>
    int offset;
    sem_wait(&ge);
    offset = buff.buffer_W_offset;
    buff.buffer_W_offset = (buff.buffer_W_offset + 1) % buff.size;
    sem_post(&ge);
    // printf("Entered Deposita - item = %d - offset = %d\n",item, offset);
    

    // <await (buff.num_reads[offset] == total_prod_consumidores)>
    sem_wait(&e[offset]); //P(ew)
    if(buff.num_reads[offset] < total_prime_prod ) {
        bloked_writers[offset]++;
        sem_post(&e[offset]); //V(e)
        sem_wait(&wait_read[offset]);
    } else {
        sem_post(&e[offset]); //V(e)
    }

    // <await (nr == 0 && nw == 0)
    if ( nw[offset] > 0 || nr[offset] > 0) {
        dw[offset]++;
        sem_post(&e[offset]);
        sem_wait(&w[offset]);
    }
    nw[offset]++;
    sem_post(&e[offset]);

    buff.data[offset] = item;
    buff.num_reads[offset] = 1;
    printf("dado <%d> inserido na pos %d do buff \n",item,offset);
    
    sem_wait(&e[offset]);
    nw[offset]--;

    // SIGNAL
    if(bloked_readers[offset] > 0) {
        bloked_readers[offset]--;
        sem_post(&wait_write[offset]);
    } else if(dr[offset] > 0) {
        dr[offset]--;
        sem_post(&r[offset]);
    } else if(dw[offset] > 0) { // provavelmente nao precisa pela exclisão no offset de w
        dw[offset]--;
        sem_post(&w[offset]);
    } else {
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
        sem_destroy(&wait_read[i]);
    }
    free(w);
    free(r);
    free(e);
    free(wait_write);
    free(wait_read);
}

//TODO Delete unused Prints! 
