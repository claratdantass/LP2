# Trabalho Prático 1 — LPII 2026.1 — P2: Merge Sort Paralelo

Ordenação de um arquivo grande de inteiros (≥ 5 milhões) com **merge sort**,
comparando uma versão **sequencial** com uma versão **paralela usando pthreads**,
e medindo o ganho de desempenho (speedup).

- **Disciplina:** Linguagens de Programação II — UFPB
- **Problema escolhido:** P2 — Merge sort de arquivo grande
- **Linguagem:** C (C11) + POSIX threads

---

## Estratégia de paralelização

1. O vetor com os `N` inteiros é dividido em `T` segmentos contíguos
   (`T` = número de threads), com o resto distribuído entre os primeiros segmentos.
2. **Fase 1 (paralela):** cada thread ordena o seu segmento com merge sort.
   Os segmentos são disjuntos, então cada thread escreve apenas na sua fatia do
   vetor e do buffer auxiliar — **não há condição de corrida** e nenhum lock é
   necessário.
3. **Fase 2 (mescla):** a thread principal mescla os `T` segmentos já ordenados,
   em árvore (mescla de pares adjacentes até sobrar um único intervalo ordenado).

Essa divisão segue exatamente o enunciado: *"cada thread ordena um pedaço do
vetor; depois o programa principal faz o merge de todos os pedaços"*.

---

## Estrutura do projeto

```
.
├── Makefile                 # compilação e alvos de execução/benchmark
├── README.md
├── src/
│   ├── merge_sort.h/.c      # merge sort sequencial (núcleo reutilizado pelas duas versões)
│   ├── io_utils.h/.c        # leitura/escrita binária e verificação de ordenação
│   ├── timer.h              # relógio monotônico (cronometra só a computação)
│   ├── generate.c           # gera o arquivo de entrada
│   ├── sequential.c         # versão sequencial   (Q2)
│   └── parallel.c           # versão paralela      (Q3/Q4)
├── scripts/
│   ├── run_benchmark.sh     # roda ≥5 execuções de cada configuração -> CSV
│   └── plot_results.py      # gera a tabela de escalabilidade e o gráfico SVG
└── results/
    ├── benchmark.csv        # tempos brutos de cada execução
    ├── summary.md           # tabela de escalabilidade
    └── speedup.svg          # gráfico de speedup
```

---

## Como compilar e executar

Requisitos: `cc`/`gcc` ou `clang`, `make` e `python3` (apenas para gerar a tabela/gráfico).

```bash
# 1. Compilar tudo
make

# 2. Gerar o arquivo de entrada (10 milhões de inteiros por padrão)
make data
#    para mudar o tamanho:  make data COUNT=5000000

# 3. Rodar a versão sequencial
make run-seq
#    ou diretamente:  ./bin/sequential data/input.bin

# 4. Rodar a versão paralela (número de threads configurável)
make run-par THREADS=8
#    ou diretamente:  ./bin/parallel data/input.bin 8
```

O **número de threads é configurável por argumento** de linha de comando
(`./bin/parallel <entrada> <num_threads>`), conforme exigido.

### Benchmark completo (Q2/Q3/Q4)

```bash
make bench                  # roda 5 execuções de seq + 1/2/4/8 threads
python3 scripts/plot_results.py   # gera results/summary.md e results/speedup.svg
```

---

## Metodologia de medição

- **Cronometra-se apenas a computação** (a ordenação, e na versão paralela também
  a mescla final). A **leitura do arquivo e a alocação de memória ficam fora do
  timer**, como pede o enunciado. Usa-se `clock_gettime(CLOCK_MONOTONIC)`.
- Cada configuração é executada **5 vezes**; reporta-se a **média** e o desvio.
- **Verificação automática de corretude:** a versão paralela guarda uma cópia do
  vetor original, ordena essa cópia com o merge sort sequencial e compara
  (`memcmp`) byte a byte com o resultado paralelo. A saída traz
  `verificacao=OK` somente se os dois resultados forem **idênticos**, garantindo
  que *a versão paralela produz o mesmo resultado da sequencial*. Também verifica
  que o vetor está de fato ordenado (`is_sorted`).

Exemplo de saída:

```
modo=sequencial n=10000000 tempo=0.456800 s ordenado=SIM
modo=paralelo threads=8 n=10000000 tempo=0.124000 s ordenado=SIM verificacao=OK
```

---

## Resultados (Q2 / Q3 / Q4)

Ambiente de teste: Apple (10 núcleos), `clang -O2`, `N = 10.000.000` inteiros,
média de 5 execuções.

**Baseline sequencial (Q2): 0,4568 s**

### Tabela de escalabilidade (Q4)

| Threads | Tempo médio (s) | Desvio (s) | Speedup | Eficiência |
|--------:|----------------:|-----------:|--------:|-----------:|
| 1 | 0,4562 | 0,0065 | 1,00x | 100% |
| 2 | 0,2426 | 0,0019 | 1,88x | 94% |
| 4 | 0,1549 | 0,0002 | 2,95x | 74% |
| 8 | 0,1240 | 0,0013 | 3,68x | 46% |

> O speedup (Q3) é calculado como `tempo_sequencial / tempo_paralelo`; a
> eficiência como `speedup / num_threads`.

### Gráfico de speedup

![Speedup vs número de threads](results/speedup.svg)

---

## Discussão (Q4)

- **Ganho real e claro:** de 1 para 8 threads o tempo cai de ~0,46 s para ~0,12 s,
  um speedup de **3,68x**. Com 1 thread, a versão paralela tem o mesmo desempenho
  da sequencial (1,00x), como esperado — não há sobrecarga relevante de criação
  de threads para esse tamanho.
- **A eficiência cai conforme aumentam as threads** (94% → 74% → 46%). Isso é
  esperado e tem duas causas principais:
  1. **Mescla final sequencial (Lei de Amdahl):** depois que as threads ordenam
     seus pedaços, a junção dos `T` segmentos é feita majoritariamente pela thread
     principal. Essa parte serial não acelera com mais threads e passa a dominar o
     tempo conforme a ordenação paralela fica mais rápida.
  2. **Limite de banda de memória:** merge sort é mais *memory-bound* do que
     *CPU-bound* — todas as threads competem pelo mesmo barramento de memória ao
     copiar dados entre o vetor e o buffer auxiliar, o que limita o ganho ao
     adicionar núcleos.
- **Como melhorar:** paralelizar também a fase de mescla (mescla em árvore com
  threads) reduziria a fração serial e empurraria a eficiência para cima em
  contagens altas de threads.

---

## Mapeamento das questões do enunciado

| Questão | Onde está |
|---|---|
| Q1 — Repositório + build + README | este repositório, `Makefile`, este `README.md` |
| Q2 — Sequencial cronometrado (≥5 execuções) | `src/sequential.c`, `scripts/run_benchmark.sh`, `results/` |
| Q3 — Paralelo com pthreads, corretude e speedup | `src/parallel.c` (verificação `memcmp` automática), tabela acima |
| Q4 — Escalabilidade 1/2/4/8, gráfico e discussão | `results/summary.md`, `results/speedup.svg`, seção *Discussão* |

---

## Convenções de código

- Identificadores em **inglês** e em **snake_case**.
- Comentários em **português**.
