/*
 * sequential.c -- merge sort sequencial (uma unica thread).
 *
 * Uso:
 *   ./sequential <arquivo_entrada> [arquivo_saida]
 *
 * Cronometra SOMENTE a ordenacao. Leitura e alocacao ficam fora do timer.
 */

#include "io_utils.h"
#include "merge_sort.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> [arquivo_saida]\n", argv[0]);
        return 1;
    }

    const char *in_path = argv[1];
    const char *out_path = (argc > 2) ? argv[2] : NULL;

    /* ---- Fora do timer: leitura e alocacao ---- */
    size_t n = 0;
    int *arr = read_int_file(in_path, &n);
    if (arr == NULL) {
        return 1;
    }
    int *tmp = malloc(n * sizeof(int));
    if (tmp == NULL) {
        fprintf(stderr, "Falha ao alocar buffer auxiliar\n");
        free(arr);
        return 1;
    }

    /* ---- Dentro do timer: somente a computacao ---- */
    double t0 = now_seconds();
    merge_sort(arr, tmp, 0, n);
    double t1 = now_seconds();

    /* ---- Fora do timer: verificacao e saida ---- */
    double elapsed = t1 - t0;
    int ok = is_sorted(arr, n);
    printf("modo=sequencial n=%zu tempo=%.6f s ordenado=%s\n",
           n, elapsed, ok ? "SIM" : "NAO");

    if (out_path != NULL) {
        if (write_int_file(out_path, arr, n) != 0) {
            free(arr);
            free(tmp);
            return 1;
        }
        printf("Saida ordenada escrita em %s\n", out_path);
    }

    free(arr);
    free(tmp);
    return ok ? 0 : 2;
}
