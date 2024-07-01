# Sistema de Gerenciamento de Memória Virtual

Este programa simula um sistema de gerenciamento de memória virtual que utiliza algoritmos de substituição de página FIFO (First In, First Out) e LRU (Least Recently Used). Ele lê endereços lógicos de um arquivo de entrada, traduz esses endereços para endereços físicos e registra o valor lido de um arquivo de armazenamento simulado.

## Requisitos

Para compilar e executar este programa, você precisa de um ambiente de desenvolvimento C com suporte para leitura e gravação de arquivos.

## Estruturas de Dados Utilizadas

- **Tabela de Páginas:** Um array que mapeia números de páginas para números de quadros de memória.
- **TLB (Translation Lookaside Buffer):** Um cache que armazena mapeamentos recentes de página para quadro para acelerar a tradução de endereços.
- **Memória Física:** Um array 2D que simula a memória física dividida em quadros.
- **Tempo de Quadros:** Um array que armazena o tempo de uso de cada quadro para o algoritmo LRU.

## Funcionalidades

### Inicialização

O programa inicializa as estruturas de dados necessárias para o funcionamento do sistema de gerenciamento de memória virtual.

### Tradução de Endereços

Para cada endereço lógico lido do arquivo de entrada, o programa:

1. Extrai o número da página e o deslocamento.
2. Verifica a TLB para um mapeamento da página.
3. Se não houver um acerto na TLB, verifica a tabela de páginas.
4. Se a página não estiver na memória, trata a falta de página usando FIFO ou LRU.
5. Insere o mapeamento na TLB.
6. Lê o valor do endereço físico correspondente e registra o resultado no arquivo de saída.

### Algoritmos de Substituição de Página

- **FIFO:** Substitui a página mais antiga na memória.
- **LRU:** Substitui a página que não é utilizada há mais tempo.

### Estatísticas

Ao final da execução, o programa registra estatísticas de desempenho, incluindo o número de faltas de página e a taxa de acertos da TLB.

## Compilação e Execução

Para compilar o programa:

```bash
gcc -o vm vm.c
```

Para executar o programa, use o seguinte comando:
```bash
./vm [arquivo_enderecos.txt] [fifo/lru]
```
- `arquivo_enderecos.txt`: Arquivo de texto contendo os endereços lógicos a serem traduzidos.
- `fifo/lru`: Especifica qual algoritmo de substituição de página usar.

Exemplo de uso:
```bash
./vm addresses.txt fifo
```

## Exemplo de Arquivo de Entrada

O arquivo de entrada deve conter um endereço lógico por linha. Exemplo:
```bash
16916
62493
30198
53683
40185
...
```

## Estrutura de Arquivos
- `BACKING_STORE.bin`: Arquivo binário que simula o armazenamento secundário.
- `correct.txt`: Arquivo de saída que contém os resultados da tradução de endereços e as estatísticas de desempenho.
