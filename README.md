# PCP-19
Programação concorrente e paralela:


Observações:
Em nossa solução consideramos um valor máximo de 46 consumidores (readers). Onde cada um tem um
/*
  * Verifica o resto da divisão do seu número primo pelo total do produto dos números primos de cada Leitor
  * Para saber quem já leu algum item, tem-se um contador (dado por 'num_reads[offset]') correspondente a cada posição do buffer
  * A cada vez que um leitor lê um item, seu número primo é multiplicado à esse contador
  * Dessa forma, se o resto da divisão for igual à 0, podemos garantir que ele já leu esse item
  * Caso, contrário é porque ele ainda não leu aquela posição ou ela foi resetada.
  * O número primo 1, não é associado à nenhum consumidor, consideramos apenas os números primos a partir de 2 para associa-los aos consumidores. Dessa quando nenhum leitor tiver lido uma determinada mensagem ainda, 'num_reads[offset]' tem o valor igual à 1, pois dessa forma o resto da divisão será sempre diferente de 0.

        <await ((buff.prod_reads[offset] % meuid) != 0)>
*/
