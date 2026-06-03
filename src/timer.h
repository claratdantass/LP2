#ifndef TIMER_H
#define TIMER_H

#include <time.h>

/*
 * Relogio monotonico de alta resolucao para cronometrar SOMENTE a
 * computacao (a ordenacao). Leitura de arquivo e alocacao ficam de fora.
 */

static inline double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

#endif /* TIMER_H */
