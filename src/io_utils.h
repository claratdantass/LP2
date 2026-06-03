#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stddef.h>

/*
 * Formato do arquivo de dados (binario):
 *   - 8 bytes: quantidade de inteiros (int64_t, little-endian da maquina)
 *   - N x 4 bytes: os N inteiros (int32_t)
 *
 * Usar formato binario evita o custo de parsing de texto e mantem o
 * foco do trabalho na computacao (ordenacao), nao na leitura.
 */

/* Le o arquivo binario "path" e devolve um vetor recem-alocado com os
 * inteiros. A quantidade lida e escrita em *out_count. Retorna NULL em erro. */
int *read_int_file(const char *path, size_t *out_count);

/* Escreve "count" inteiros de "arr" no arquivo binario "path".
 * Retorna 0 em sucesso, -1 em erro. */
int write_int_file(const char *path, const int *arr, size_t count);

/* Verifica se arr[0..count) esta em ordem nao-decrescente.
 * Retorna 1 se ordenado, 0 caso contrario. */
int is_sorted(const int *arr, size_t count);

#endif /* IO_UTILS_H */
