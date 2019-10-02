#### $@    Nome da regra. 
#### $<    Nome da primeira dependência 
#### $^ 	Lista de dependências
#### $? 	Lista de dependências mais recentes que a regra.
#### $* 	Nome do arquivo sem sufixo

CC=gcc
CFLAGS=-W -Wall -std=c11 
LIBS=-lm -fopenmp
EXEC=quadratura
# todos os arquivos .c da pasta serao considerados src
SRC= $(wildcard *.c)
# todos os arquivos do SRC (.c) vao gerar um obj
OBJ= $(SRC:.c=.o)
all: $(EXEC)
quadratura:$(OBJ)
	$(CC) -o $@ $^ $(LIBS)
%.o: %.c
	$(CC) -o $@ -c $^ $(CFLAGS) $(LIBS)