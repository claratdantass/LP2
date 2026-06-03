#include "merge_sort.h"

/* Mescla arr[lo..mid) e arr[mid..hi) usando tmp como area de trabalho. */
void merge(int *arr, int *tmp, size_t lo, size_t mid, size_t hi) {
    size_t i = lo;   /* cursor do sub-intervalo esquerdo  */
    size_t j = mid;  /* cursor do sub-intervalo direito   */
    size_t k = lo;   /* cursor de escrita no buffer tmp   */

    while (i < mid && j < hi) {
        if (arr[i] <= arr[j]) {
            tmp[k++] = arr[i++];
        } else {
            tmp[k++] = arr[j++];
        }
    }
    /* Copia o que sobrou de cada lado. */
    while (i < mid) tmp[k++] = arr[i++];
    while (j < hi)  tmp[k++] = arr[j++];

    /* Devolve o intervalo ja mesclado para o vetor original. */
    for (size_t t = lo; t < hi; t++) {
        arr[t] = tmp[t];
    }
}

/* Merge sort top-down classico sobre o intervalo arr[lo..hi). */
void merge_sort(int *arr, int *tmp, size_t lo, size_t hi) {
    if (hi - lo <= 1) {
        return; /* intervalo de 0 ou 1 elemento ja esta ordenado */
    }
    size_t mid = lo + (hi - lo) / 2;
    merge_sort(arr, tmp, lo, mid);
    merge_sort(arr, tmp, mid, hi);
    merge(arr, tmp, lo, mid, hi);
}
