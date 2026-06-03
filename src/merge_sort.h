#ifndef MERGE_SORT_H
#define MERGE_SORT_H

#include <stddef.h>

/*
 * Mescla os dois sub-intervalos ordenados arr[lo..mid) e arr[mid..hi)
 * num unico intervalo ordenado, usando "tmp" como buffer auxiliar.
 */
void merge(int *arr, int *tmp, size_t lo, size_t mid, size_t hi);

/*
 * Ordena arr[lo..hi) em ordem crescente usando merge sort top-down.
 * "tmp" deve ter pelo menos "hi" elementos de espaco.
 */
void merge_sort(int *arr, int *tmp, size_t lo, size_t hi);

#endif /* MERGE_SORT_H */
