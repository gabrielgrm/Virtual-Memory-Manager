#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TAMANHO_QUADRO 256
#define NUMERO_QUADROS 128
#define TAMANHO_TLB 16
#define TAMANHO_TABELA_PAGINAS 256

int tabelaPaginas[TAMANHO_TABELA_PAGINAS];
int TLBPaginas[TAMANHO_TLB];
int TLBQuadros[TAMANHO_TLB];
int tempoQuadros[NUMERO_QUADROS];
int memoriaFisica[NUMERO_QUADROS][TAMANHO_QUADRO];
int valorpaginas[5000];

int totalFaltasPagina = 0;
int totalAcertosTLB = 0;
int proximoQuadroDisponivel = 0;
int totalEntradasTLB = 0;
int contadorGlobalTempo = 0;
int primeiroNumeroTabelaPaginasDisponivel = 0;
bool usarLRU = false, usarFIFO = false;

#define TAMANHO_ENDERECO 12
#define TAMANHO_PAGINA 256

FILE *arquivoEnderecos;
FILE *arquivoArmazenamento;
FILE *arquivoSaida;

char enderecoStr[TAMANHO_ENDERECO];
int enderecoLogico;
signed char bufferArmazenamento[TAMANHO_PAGINA];
signed char valorLido;

void obterPagina(int endereco);
void tratarFaltaPaginaLRU(int numeroPagina);
void tratarFaltaPaginaFIFO(int numeroPagina);
void inserirNaTLB(int numeroPagina, int numeroQuadro);
int encontrarQuadroLRU(int tempos[], int tamanho);

int encontrarQuadroLRU(int tempos[], int tamanho) {
    int minimo = 9999, posicao = 0;

    for (int i = 0; i < tamanho; i++) {
        if (tempos[i] < minimo) {
            minimo = tempos[i];
            posicao = i;
        }
    }

    return posicao;
}

void inicializarEstruturas() {
    for (int i = 0; i < TAMANHO_TABELA_PAGINAS; i++) {
        tabelaPaginas[i] = -1;
    }

    for (int i = 0; i < TAMANHO_TLB; i++) {
        TLBPaginas[i] = -1;
        TLBQuadros[i] = -1;
    }
}

void obterPagina(int enderecoLogico) {
    int deslocamento = enderecoLogico & 0xFF;
    int numeroPagina = enderecoLogico >> 8;

    int numeroQuadro = -1;
    bool acertoTLB = false;
    int indiceTLB = -1;

    for (int i = 0; i < TAMANHO_TLB; i++) {
        if (TLBPaginas[i] == numeroPagina) {
            numeroQuadro = TLBQuadros[i];
            totalAcertosTLB++;
            acertoTLB = true;
            indiceTLB = i;
            break;
        }
    }

    if (numeroQuadro == -1) {
        if (tabelaPaginas[numeroPagina] != -1) {
            numeroQuadro = tabelaPaginas[numeroPagina];
        } else {
            if (usarFIFO) {
                tratarFaltaPaginaFIFO(numeroPagina);
            } else if (usarLRU) {
                tratarFaltaPaginaLRU(numeroPagina);
            }

            totalFaltasPagina++;
            numeroQuadro = tabelaPaginas[numeroPagina];
        }
    }

    if (!acertoTLB) {
        inserirNaTLB(numeroPagina, numeroQuadro);
        indiceTLB = totalEntradasTLB - 1;
    }

    fseek(arquivoArmazenamento, enderecoLogico, SEEK_SET);
    fread(&valorLido, sizeof(signed char), 1, arquivoArmazenamento);

    contadorGlobalTempo++;
    tempoQuadros[numeroQuadro] = contadorGlobalTempo;

    fprintf(arquivoSaida, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n", enderecoLogico, indiceTLB, (numeroQuadro << 8) | deslocamento, valorLido);
}

void inserirNaTLB(int numeroPagina, int numeroQuadro) {
    for (int i = 0; i < TAMANHO_TLB; i++) {
        if (TLBPaginas[i] == numeroPagina) {
            return;
        }
    }

    if (totalEntradasTLB >= TAMANHO_TLB) {
        totalEntradasTLB = 0;
    }

    TLBPaginas[totalEntradasTLB] = numeroPagina;
    TLBQuadros[totalEntradasTLB] = numeroQuadro;
    totalEntradasTLB++;
}

void tratarFaltaPaginaFIFO(int numeroPagina) {
    fseek(arquivoArmazenamento, numeroPagina * TAMANHO_PAGINA, SEEK_SET);
    fread(bufferArmazenamento, sizeof(signed char), TAMANHO_PAGINA, arquivoArmazenamento);

    if (proximoQuadroDisponivel >= NUMERO_QUADROS) {
        proximoQuadroDisponivel = 0;
    }

    for (int i = 0; i < TAMANHO_PAGINA; i++) {
        memoriaFisica[proximoQuadroDisponivel][i] = bufferArmazenamento[i];
    }

    for (int i = 0; i < TAMANHO_TABELA_PAGINAS; i++) {
        if (tabelaPaginas[i] == proximoQuadroDisponivel) {
            tabelaPaginas[i] = -1;
        }
    }

    tabelaPaginas[numeroPagina] = proximoQuadroDisponivel;
    proximoQuadroDisponivel++;
}

void tratarFaltaPaginaLRU(int numeroPagina) {
    fseek(arquivoArmazenamento, numeroPagina * TAMANHO_PAGINA, SEEK_SET);
    fread(bufferArmazenamento, sizeof(signed char), TAMANHO_PAGINA, arquivoArmazenamento);

    if (primeiroNumeroTabelaPaginasDisponivel >= NUMERO_QUADROS) {
        proximoQuadroDisponivel = encontrarQuadroLRU(tempoQuadros, NUMERO_QUADROS);
    }

    for (int i = 0; i < TAMANHO_PAGINA; i++) {
        memoriaFisica[proximoQuadroDisponivel][i] = bufferArmazenamento[i];
    }

    for (int i = 0; i < TAMANHO_TABELA_PAGINAS; i++) {
        if (tabelaPaginas[i] == proximoQuadroDisponivel) {
            tabelaPaginas[i] = -1;
        }
    }

    tabelaPaginas[numeroPagina] = proximoQuadroDisponivel;
    if (proximoQuadroDisponivel < NUMERO_QUADROS) {
        proximoQuadroDisponivel++;
    }
    primeiroNumeroTabelaPaginasDisponivel++;
}

int main(int argc, char *argv[]) {
    for (int i = 0; i < 5000; i++) {
        valorpaginas[i] = -1;
    }
    if (argc != 3) {
        fprintf(stderr, "./vm [local.txt] [fifo/lru]\n");
        return -1;
    }

    if (strcmp(argv[2], "fifo") == 0) {
        usarFIFO = true;
    } else if (strcmp(argv[2], "lru") == 0) {
        usarLRU = true;
    }

    arquivoArmazenamento = fopen("BACKING_STORE.bin", "rb");
    if (arquivoArmazenamento == NULL) {
        fprintf(stderr, "Error opening backing store file\n");
        return -1;
    }

    arquivoEnderecos = fopen(argv[1], "r");
    if (arquivoEnderecos == NULL) {
        fprintf(stderr, "Error opening address file\n");
        return -1;
    }

    arquivoSaida = fopen("correct.txt", "w+");
    if (arquivoSaida == NULL) {
        fprintf(stderr, "Error opening output file\n");
        return -1;
    }

    inicializarEstruturas();

    int totalEnderecosTraduzidos = 0;
    while (fgets(enderecoStr, TAMANHO_ENDERECO, arquivoEnderecos) != NULL) {
        enderecoLogico = atoi(enderecoStr);
        obterPagina(enderecoLogico);
        totalEnderecosTraduzidos++;
    }

    fprintf(arquivoSaida, "Number of Translated Addresses = %d\n", totalEnderecosTraduzidos);
    double taxaFaltasPagina = (float) totalFaltasPagina / (float)totalEnderecosTraduzidos;
    double taxaAcertosTLB = (float) totalAcertosTLB / (float)totalEnderecosTraduzidos;

    fprintf(arquivoSaida, "Page Faults = %d\n", totalFaltasPagina);
    fprintf(arquivoSaida, "Page Fault Rate = %.3f\n", taxaFaltasPagina);
    fprintf(arquivoSaida, "TLB Hits = %d\n", totalAcertosTLB);
    fprintf(arquivoSaida, "TLB Hit Rate = %.3f\n", taxaAcertosTLB);

    fclose(arquivoEnderecos);
    fclose(arquivoArmazenamento);
    fclose(arquivoSaida);

    return 0;
}