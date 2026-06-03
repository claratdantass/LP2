# Makefile -- Trabalho Pratico 1 (LPII 2026.1) -- P2: merge sort paralelo
#
# Alvos principais:
#   make            -> compila generate, sequential e parallel
#   make data       -> gera o arquivo de entrada (10 milhoes de inteiros)
#   make run-seq    -> roda a versao sequencial
#   make run-par    -> roda a versao paralela (THREADS=4 por padrao)
#   make bench      -> roda o benchmark completo (Q2/Q3/Q4)
#   make clean      -> remove binarios e dados

CC      := cc
CFLAGS  := -O2 -Wall -Wextra -std=c11 -pthread
LDFLAGS := -pthread

SRC_DIR := src
BIN_DIR := bin
DATA_DIR := data

# Parametros configuraveis pela linha de comando, ex: make run-par THREADS=8
COUNT   ?= 10000000
THREADS ?= 4
INPUT   := $(DATA_DIR)/input.bin

COMMON_SRC := $(SRC_DIR)/merge_sort.c $(SRC_DIR)/io_utils.c

BINARIES := $(BIN_DIR)/generate $(BIN_DIR)/sequential $(BIN_DIR)/parallel

.PHONY: all data run-seq run-par bench clean

all: $(BINARIES)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/generate: $(SRC_DIR)/generate.c $(COMMON_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/sequential: $(SRC_DIR)/sequential.c $(COMMON_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/parallel: $(SRC_DIR)/parallel.c $(COMMON_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ $^ $(LDFLAGS)

# Gera o arquivo de entrada apenas se ainda nao existir.
data: $(BIN_DIR)/generate
	@mkdir -p $(DATA_DIR)
	@test -f $(INPUT) || $(BIN_DIR)/generate $(INPUT) $(COUNT)

run-seq: $(BIN_DIR)/sequential data
	$(BIN_DIR)/sequential $(INPUT)

run-par: $(BIN_DIR)/parallel data
	$(BIN_DIR)/parallel $(INPUT) $(THREADS)

bench: all data
	./scripts/run_benchmark.sh

clean:
	rm -rf $(BIN_DIR) $(DATA_DIR)
