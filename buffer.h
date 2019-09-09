#ifndef _BUFFER_H
#define _BUFFER_H 1

void buffer_init(int consumidores, int produtores, int tamanho_buffer);
int consome(long meuid);
void deposita(int item);
void free_buffer();

#endif