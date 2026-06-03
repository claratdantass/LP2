/*
 * generate.c -- gera um arquivo binario com N inteiros pseudoaleatorios.
 *
 * Uso:
 *   ./generate <arquivo_saida> [quantidade] [semente]
 *
 * Exemplo (10 milhoes de inteiros, acima do minimo de 5 milhoes exigido):
 *   ./generate data/input.bin 10000000
 */

#include "io_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_COUNT 10000000  /* 10 milhoes */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_saida> [quantidade] [semente]\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    size_t count = (argc > 2) ? strtoull(argv[2], NULL, 10) : DEFAULT_COUNT;
    unsigned int seed = (argc > 3) ? (unsigned int)strtoul(argv[3], NULL, 10) : 42u;

    if (count == 0) {
        fprintf(stderr, "Quantidade deve ser maior que zero\n");
        return 1;
    }

    int *arr = malloc(count * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Falha ao alocar %zu inteiros\n", count);
        return 1;
    }

    /* Semente fixa por padrao -> dados reproduziveis entre execucoes. */
    srand(seed);
    for (size_t i = 0; i < count; i++) {
        /* Combina dois rand() para cobrir toda a faixa de int. */
        arr[i] = (int)((rand() << 16) ^ rand());
    }

    if (write_int_file(path, arr, count) != 0) {
        free(arr);
        return 1;
    }

    printf("Gerados %zu inteiros (semente %u) em %s\n", count, seed, path);
    free(arr);
    return 0;
}
