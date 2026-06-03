#!/usr/bin/env bash
#
# run_benchmark.sh -- mede o desempenho das versoes sequencial e paralela.
#
# Para cada configuracao executa REPS vezes (>= 5) e grava cada tempo em
# results/benchmark.csv. A media e o speedup sao calculados depois pelo
# script de plotagem (scripts/plot_results.py) ou pela propria tabela.
#
# Variaveis de ambiente:
#   REPS     numero de repeticoes por configuracao (padrao 5)
#   THREADS  lista de contagens de threads (padrao "1 2 4 8")
#   INPUT    arquivo de entrada (padrao data/input.bin)

set -euo pipefail

cd "$(dirname "$0")/.."

REPS="${REPS:-5}"
THREADS_LIST="${THREADS:-1 2 4 8}"
INPUT="${INPUT:-data/input.bin}"
OUT_DIR="results"
CSV="$OUT_DIR/benchmark.csv"

mkdir -p "$OUT_DIR"

if [[ ! -f "$INPUT" ]]; then
    echo "Arquivo de entrada '$INPUT' nao encontrado. Rode 'make data' antes." >&2
    exit 1
fi

# Extrai o valor de "tempo=<x>" da saida dos programas.
extract_time() {
    sed -n 's/.*tempo=\([0-9.]*\).*/\1/p'
}

echo "modo,threads,execucao,tempo_s" > "$CSV"

echo ">> Versao SEQUENCIAL ($REPS execucoes)"
for r in $(seq 1 "$REPS"); do
    t=$(./bin/sequential "$INPUT" | extract_time)
    echo "sequencial,1,$r,$t" >> "$CSV"
    echo "   execucao $r: ${t}s"
done

for nt in $THREADS_LIST; do
    echo ">> Versao PARALELA com $nt thread(s) ($REPS execucoes)"
    for r in $(seq 1 "$REPS"); do
        t=$(./bin/parallel "$INPUT" "$nt" | extract_time)
        echo "paralelo,$nt,$r,$t" >> "$CSV"
        echo "   execucao $r: ${t}s"
    done
done

echo
echo "Resultados brutos gravados em $CSV"
echo "Gere a tabela e o grafico com: python3 scripts/plot_results.py"
