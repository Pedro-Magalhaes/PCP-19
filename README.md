# PCP-19
Programação concorrente e paralela:

Instruções:
 Para executar o programa vá para a pasta onde o projeto se encontra e digite 'make' e depois execute o arquivo gerado ('bufferlimitado') passando os atributos: Nconsumidores Nprodutores TamanhoBuffer Niteracoes, (nesta ordem)
 
        $ cd < path-to-PCP-19/ >
        $ make
        $ ./bufferlimitado  Nconsumidores Nprodutores TamanhoBuffer Niteracoes
        [EXEMPLO] $ ./bufferlimitado 6 5 3 2 
        

Observações:
Em nossa solução consideramos um valor máximo de 15 consumidores (readers). Onde cada um:

  * Verifica o resto da divisão do seu número primo pelo total do produto dos números primos de cada Leitor
  * Para saber quem já leu algum item, tem-se um contador (dado por 'num_reads[offset]') correspondente a cada posição do buffer
  * A cada vez que um leitor lê um item, seu número primo é multiplicado à esse contador
  * Dessa forma, se o resto da divisão for igual à 0, podemos garantir que ele já leu esse item
  * Caso, contrário é porque ele ainda não leu aquela posição ou ela foi resetada.
  * O número primo 1, não é associado à nenhum consumidor, consideramos apenas os números primos a partir de 2 para associa-los aos consumidores. Dessa quando nenhum leitor tiver lido uma determinada mensagem ainda, 'num_reads[offset]' tem o valor igual à 1, pois dessa forma o resto da divisão será sempre diferente de 0.

        <await ((buff.num_reads[offset] % num_primo) != 0)>

Nossa solução consiste em fazer com que todo o leitor, após a leitura de um item, espere até que todos os outros leitores leiam o mesmo item, e só então ele irá ler o próximo item. Dessa forma evitamos problemas de concorrência e conseguimos deixar o código mais simples.


Dificuldades:
Tentamos utilizar uma solução que liberasse uma maior concorrência entre os consumidores e leitores. No entanto acabamos tendo muita dificauldade em tratar consumidores mais rápidos que davam a volta no buffer e encontravam leitores atrasados. Não conseguimos achar uma solução para diferenciar um leitor que já tinha dado a volta e estava bloqueado e acabavamos passando o bastão para ele ler uma informação repetida.

Um dos maiores problemas encontrados foi que qualquer pequeno ajuste que tentavamos fazer, fugindo da notação do Andrews acabava por criar diversos problemas e a complexidade das soluções aumentava rápidamente

Exemplo de Input/Output do programa:
```
[Marcelo@Marcelo PCP-19]$ ./bufferlimitado 3 4 4 2
c: 3, p: 4, N: 4
		Producer, depositou: 0, na posicao: [0] +
Consumer<2>, leu: 0
Consumer<1>, leu: 0
Consumer<0>, leu: 0
		Producer, depositou: 12, na posicao: [1] +
		Producer, depositou: 8, na posicao: [2] +
		Producer, depositou: 4, na posicao: [3] +
		Producer, depositou: 1, na posicao: [0] +
Consumer<1>, leu: 12
Consumer<2>, leu: 12
Consumer<0>, leu: 12
		Producer, depositou: 2, na posicao: [1] +
Consumer<0>, leu: 8
Consumer<1>, leu: 8
Consumer<2>, leu: 8
		Producer, depositou: 5, na posicao: [2] +
Consumer<1>, leu: 4
Consumer<0>, leu: 4
Consumer<2>, leu: 4
		Producer, depositou: 9, na posicao: [3] +
Consumer<0>, leu: 1
Consumer<1>, leu: 1
Consumer<2>, leu: 1
		Producer, depositou: 13, na posicao: [0] +
Consumer<1>, leu: 2
Consumer<0>, leu: 2
Consumer<2>, leu: 2
		Producer, depositou: 3, na posicao: [1] +
Consumer<0>, leu: 5
Consumer<1>, leu: 5
Consumer<2>, leu: 5
		Producer, depositou: 6, na posicao: [2] +
Consumer<2>, leu: 9
Consumer<1>, leu: 9
Consumer<0>, leu: 9
		Producer, depositou: 10, na posicao: [3] +
Consumer<1>, leu: 13
Consumer<2>, leu: 13
Consumer<0>, leu: 13
		Producer, depositou: 14, na posicao: [0] +
Consumer<2>, leu: 3
Consumer<1>, leu: 3
Consumer<0>, leu: 3
		Producer, depositou: 4, na posicao: [1] +
Consumer<1>, leu: 6
Consumer<2>, leu: 6
Consumer<0>, leu: 6
		Producer, depositou: 7, na posicao: [2] +
Consumer<2>, leu: 10
Consumer<1>, leu: 10
Consumer<0>, leu: 10
		Producer, depositou: 11, na posicao: [3] +
Consumer<1>, leu: 14
Consumer<0>, leu: 14
Consumer<2>, leu: 14
		Producer, depositou: 15, na posicao: [0] +
Consumer<0>, leu: 4
Consumer<1>, leu: 4
Consumer<2>, leu: 4
		Producer, depositou: 5, na posicao: [1] +
Consumer<1>, leu: 7
Consumer<0>, leu: 7
Consumer<2>, leu: 7
		Producer, depositou: 8, na posicao: [2] +
Consumer<0>, leu: 11
Consumer<1>, leu: 11
Consumer<2>, leu: 11
		Producer, depositou: 12, na posicao: [3] +
Consumer<1>, leu: 15
Consumer<0>, leu: 15
Consumer<2>, leu: 15
		Producer, depositou: 16, na posicao: [0] +
Consumer<0>, leu: 5
Consumer<1>, leu: 5
Consumer<2>, leu: 5
		Producer, depositou: 6, na posicao: [1] +
Consumer<1>, leu: 8
Consumer<0>, leu: 8
Consumer<2>, leu: 8
		Producer, depositou: 9, na posicao: [2] +
Consumer<0>, leu: 12
Consumer<1>, leu: 12
Consumer<2>, leu: 12
		Producer, depositou: 13, na posicao: [3] +
Consumer<1>, leu: 16
Consumer<0>, leu: 16
Consumer<2>, leu: 16
		Producer, depositou: 17, na posicao: [0] +
Consumer<0>, leu: 6
Consumer<1>, leu: 6
Consumer<2>, leu: 6
		Producer, depositou: 7, na posicao: [1] +
Consumer<1>, leu: 9
Consumer<0>, leu: 9
Consumer<2>, leu: 9
		Producer, depositou: 10, na posicao: [2] +
Consumer<0>, leu: 13
Consumer<1>, leu: 13
Consumer<2>, leu: 13
		Producer, depositou: 14, na posicao: [3] +
Consumer<1>, leu: 17
Consumer<2>, leu: 17
Consumer<0>, leu: 17
		Producer, depositou: 18, na posicao: [0] +
Consumer<2>, leu: 7
Consumer<1>, leu: 7
Consumer<0>, leu: 7
		Producer, depositou: 11, na posicao: [1] +
Consumer<1>, leu: 10
Consumer<2>, leu: 10
Consumer<0>, leu: 10
		Producer, depositou: 15, na posicao: [2] +
Consumer<2>, leu: 14
Consumer<0>, leu: 14
Consumer<1>, leu: 14
		Producer, depositou: 19, na posicao: [3] +
Consumer<0>, leu: 18
Consumer<2>, leu: 18
Consumer<1>, leu: 18
Consumer<2>, leu: 11
Consumer<0>, leu: 11
Consumer<1>, leu: 11
Consumer<0>, leu: 15
Consumer<2>, leu: 15
Consumer<1>, leu: 15
Consumer<2>, leu: 19
Consumer<0>, leu: 19
Consumer<1>, leu: 19
liberando memória
```


