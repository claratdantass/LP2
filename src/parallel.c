/*
 * parallel.c -- merge sort paralelo com pthreads.
 *
 * Estrategia:
 *   1. O vetor e dividido em T segmentos contiguos (T = numero de threads).
 *   2. Cada thread ordena o seu segmento com merge sort (em paralelo).
 *   3. A thread principal mescla os T segmentos ja ordenados, em arvore
 *      (mescla de pares adjacentes ate sobrar um unico intervalo).
 *
 * Uso:
 *   ./parallel <arquivo_entrada> [num_threads] [arquivo_saida]
 *
 * O numero de threads e configuravel por argumento (padrao: 4).
 * Cronometra SOMENTE a computacao (ordenacao + mescla final).
 */

#include "io_utils.h"
#include "merge_sort.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define DEFAULT_THREADS 4

/* Dados que cada thread recebe para ordenar o seu segmento. */
typedef struct {
    int *arr;     /* vetor compartilhado                       */
    int *tmp;     /* buffer auxiliar compartilhado             */
    size_t lo;    /* inicio do segmento (inclusivo)            */
    size_t hi;    /* fim do segmento (exclusivo)               */
} sort_task_t;

/* Funcao executada por cada thread: ordena arr[lo..hi).
 * Os segmentos sao disjuntos, entao cada thread escreve apenas na sua
 * fatia de arr e de tmp -- nao ha condicao de corrida. */
static void *sort_segment(void *arg) {
    sort_task_t *task = (sort_task_t *)arg;
    merge_sort(task->arr, task->tmp, task->lo, task->hi);
    return NULL;
}

/* Mescla os T segmentos ordenados (delimitados por "bounds") em arvore.
 * bounds tem T+1 elementos: bounds[0]=0, ..., bounds[T]=n. */
static void merge_runs(int *arr, int *tmp, size_t *bounds, int runs) {
    while (runs > 1) {
        int next = 0;
        for (int i = 0; i < runs; i += 2) {
            if (i + 1 < runs) {
                /* Mescla o par adjacente [bounds[i], bounds[i+2]). */
                merge(arr, tmp, bounds[i], bounds[i + 1], bounds[i + 2]);
                bounds[next++] = bounds[i];
            } else {
                /* Run impar sem par: apenas sobe de nivel. */
                bounds[next++] = bounds[i];
            }
        }
        bounds[next] = bounds[runs];
        runs = next;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> [num_threads] [arquivo_saida]\n",
                argv[0]);
        return 1;
    }

    const char *in_path = argv[1];
    int num_threads = (argc > 2) ? atoi(argv[2]) : DEFAULT_THREADS;
    const char *out_path = (argc > 3) ? argv[3] : NULL;

    if (num_threads < 1) {
        fprintf(stderr, "num_threads deve ser >= 1\n");
        return 1;
    }

    /* ---- Fora do timer: leitura e alocacao ---- */
    size_t n = 0;
    int *arr = read_int_file(in_path, &n);
    if (arr == NULL) {
        return 1;
    }

    /* Nao faz sentido ter mais threads do que elementos. */
    if ((size_t)num_threads > n) {
        num_threads = (int)n;
    }

    int *tmp = malloc(n * sizeof(int));
    /* Copia do vetor original para gerar a referencia sequencial e
     * verificar automaticamente a corretude no fim. */
    int *reference = malloc(n * sizeof(int));
    int *ref_tmp = malloc(n * sizeof(int));
    if (tmp == NULL || reference == NULL || ref_tmp == NULL) {
        fprintf(stderr, "Falha ao alocar buffers auxiliares\n");
        free(arr); free(tmp); free(reference); free(ref_tmp);
        return 1;
    }
    memcpy(reference, arr, n * sizeof(int));

    pthread_t *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    sort_task_t *tasks = malloc((size_t)num_threads * sizeof(sort_task_t));
    size_t *bounds = malloc((size_t)(num_threads + 1) * sizeof(size_t));
    if (threads == NULL || tasks == NULL || bounds == NULL) {
        fprintf(stderr, "Falha ao alocar estruturas das threads\n");
        free(arr); free(tmp); free(reference); free(ref_tmp);
        free(threads); free(tasks); free(bounds);
        return 1;
    }

    /* Calcula as fronteiras dos segmentos, distribuindo o resto. */
    size_t base = n / (size_t)num_threads;
    size_t rest = n % (size_t)num_threads;
    bounds[0] = 0;
    for (int i = 0; i < num_threads; i++) {
        size_t len = base + (((size_t)i < rest) ? 1 : 0);
        bounds[i + 1] = bounds[i] + len;
    }

    /* ---- Dentro do timer: somente a computacao ---- */
    double t0 = now_seconds();

    /* Fase 1: cada thread ordena o seu segmento. */
    for (int i = 0; i < num_threads; i++) {
        tasks[i].arr = arr;
        tasks[i].tmp = tmp;
        tasks[i].lo = bounds[i];
        tasks[i].hi = bounds[i + 1];
        if (pthread_create(&threads[i], NULL, sort_segment, &tasks[i]) != 0) {
            fprintf(stderr, "Falha ao criar thread %d\n", i);
            return 1;
        }
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Fase 2: a thread principal mescla os segmentos ordenados. */
    merge_runs(arr, tmp, bounds, num_threads);

    double t1 = now_seconds();
    double elapsed = t1 - t0;

    /* ---- Fora do timer: verificacao automatica de corretude ---- */
    merge_sort(reference, ref_tmp, 0, n);  /* referencia sequencial */
    int igual = (memcmp(arr, reference, n * sizeof(int)) == 0);
    int ok = is_sorted(arr, n);

    printf("modo=paralelo threads=%d n=%zu tempo=%.6f s ordenado=%s verificacao=%s\n",
           num_threads, n, elapsed,
           ok ? "SIM" : "NAO",
           igual ? "OK" : "FALHOU");

    if (out_path != NULL) {
        if (write_int_file(out_path, arr, n) != 0) {
            free(arr); free(tmp); free(reference); free(ref_tmp);
            free(threads); free(tasks); free(bounds);
            return 1;
        }
        printf("Saida ordenada escrita em %s\n", out_path);
    }

    free(arr); free(tmp); free(reference); free(ref_tmp);
    free(threads); free(tasks); free(bounds);
    return (ok && igual) ? 0 : 2;
}
