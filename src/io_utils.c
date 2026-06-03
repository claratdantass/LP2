#include "io_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int *read_int_file(const char *path, size_t *out_count) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror("fopen (leitura)");
        return NULL;
    }

    int64_t count = 0;
    if (fread(&count, sizeof(count), 1, fp) != 1 || count <= 0) {
        fprintf(stderr, "Cabecalho invalido em %s\n", path);
        fclose(fp);
        return NULL;
    }

    int *arr = malloc((size_t)count * sizeof(int));
    if (arr == NULL) {
        fprintf(stderr, "Falha ao alocar %lld inteiros\n", (long long)count);
        fclose(fp);
        return NULL;
    }

    size_t lidos = fread(arr, sizeof(int), (size_t)count, fp);
    fclose(fp);
    if (lidos != (size_t)count) {
        fprintf(stderr, "Esperava %lld inteiros, li %zu\n", (long long)count, lidos);
        free(arr);
        return NULL;
    }

    *out_count = (size_t)count;
    return arr;
}

int write_int_file(const char *path, const int *arr, size_t count) {
    FILE *fp = fopen(path, "wb");
    if (fp == NULL) {
        perror("fopen (escrita)");
        return -1;
    }

    int64_t header = (int64_t)count;
    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        perror("fwrite (cabecalho)");
        fclose(fp);
        return -1;
    }
    if (fwrite(arr, sizeof(int), count, fp) != count) {
        perror("fwrite (dados)");
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int is_sorted(const int *arr, size_t count) {
    for (size_t i = 1; i < count; i++) {
        if (arr[i - 1] > arr[i]) {
            return 0;
        }
    }
    return 1;
}
