# PCP-19
Programação concorrente e paralela:


Observações:
Em nossa solução consideramos um valor máximo de 15 consumidores (readers). Onde cada um:

  * Verifica o resto da divisão do seu número primo pelo total do produto dos números primos de cada Leitor
  * Para saber quem já leu algum item, tem-se um contador (dado por 'num_reads[offset]') correspondente a cada posição do buffer
  * A cada vez que um leitor lê um item, seu número primo é multiplicado à esse contador
  * Dessa forma, se o resto da divisão for igual à 0, podemos garantir que ele já leu esse item
  * Caso, contrário é porque ele ainda não leu aquela posição ou ela foi resetada.
  * O número primo 1, não é associado à nenhum consumidor, consideramos apenas os números primos a partir de 2 para associa-los aos consumidores. Dessa quando nenhum leitor tiver lido uma determinada mensagem ainda, 'num_reads[offset]' tem o valor igual à 1, pois dessa forma o resto da divisão será sempre diferente de 0.

        <await ((buff.num_reads[offset] % num_primo) != 0)>


Dificuldades:
Tentamos utilizar uma solução que liberasse uma maior concorrência entre os consumidores e leitores. No entanto acabamos tendo muita dificauldade em tratar consumidores mais rápidos que davam a volta no buffer e encontravam leitores atrasados. Não conseguimos achar uma solução para diferenciar um leitor que já tinha dado a volta e estava bloqueado e acabavamos passando o bastão para ele ler uma informação repetida.

Um dos maiores problemas encontrados foi que qualquer pequeno ajuste que tentavamos fazer, fugindo da notação do Andrews acabava por criar diversos problemas e a complexidade das soluções aumentava rápidamente
