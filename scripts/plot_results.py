#!/usr/bin/env python3
"""
plot_results.py -- le results/benchmark.csv, calcula media/speedup/eficiencia,
gera a tabela de escalabilidade (results/summary.md) e um grafico de speedup
em SVG (results/speedup.svg).

Usa apenas a biblioteca padrao do Python (sem matplotlib), entao funciona em
qualquer ambiente com Python 3. O SVG renderiza direto no README do GitHub.
"""

import csv
import os
import statistics
from collections import defaultdict

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CSV_PATH = os.path.join(BASE, "results", "benchmark.csv")
SUMMARY_PATH = os.path.join(BASE, "results", "summary.md")
SVG_PATH = os.path.join(BASE, "results", "speedup.svg")


def carregar():
    """Agrupa os tempos por (modo, threads), descartando a execucao 1 (aquecimento)."""
    tempos = defaultdict(list)
    with open(CSV_PATH, newline="") as f:
        for row in csv.DictReader(f):
            if int(row["execucao"]) == 1:
                continue  # descarta o primeiro run (warmup)
            chave = (row["modo"], int(row["threads"]))
            tempos[chave].append(float(row["tempo_s"]))
    return tempos


def resumir(tempos):
    """Calcula media, desvio, speedup e eficiencia por contagem de threads."""
    seq_media = statistics.mean(tempos[("sequencial", 1)])
    linhas = []
    threads = sorted(t for (modo, t) in tempos if modo == "paralelo")
    for nt in threads:
        amostras = tempos[("paralelo", nt)]
        media = statistics.mean(amostras)
        desvio = statistics.pstdev(amostras) if len(amostras) > 1 else 0.0
        speedup = seq_media / media if media > 0 else 0.0
        eficiencia = speedup / nt
        linhas.append({
            "threads": nt,
            "media": media,
            "desvio": desvio,
            "speedup": speedup,
            "eficiencia": eficiencia,
        })
    return seq_media, linhas


def escrever_tabela(seq_media, linhas):
    with open(SUMMARY_PATH, "w") as f:
        f.write("# Tabela de escalabilidade\n\n")
        f.write(f"Tempo medio sequencial (baseline): **{seq_media:.4f} s**\n\n")
        f.write("| Threads | Tempo medio (s) | Desvio (s) | Speedup | Eficiencia |\n")
        f.write("|--------:|----------------:|-----------:|--------:|-----------:|\n")
        for l in linhas:
            f.write("| {threads} | {media:.4f} | {desvio:.4f} | {speedup:.2f}x | {efp:.0f}% |\n".format(
                efp=l["eficiencia"] * 100, **l))
    print(f"Tabela escrita em {SUMMARY_PATH}")


def escrever_svg(linhas):
    """Grafico de speedup observado vs ideal (linear)."""
    W, H = 640, 420
    ml, mr, mt, mb = 70, 30, 40, 60   # margens
    pw, ph = W - ml - mr, H - mt - mb

    threads = [l["threads"] for l in linhas]
    speedups = [l["speedup"] for l in linhas]
    max_t = max(threads)
    max_y = max(max_t, max(speedups)) * 1.1  # ideal vai ate max_t

    def px(t):  # eixo X: numero de threads
        return ml + (t / max_t) * pw

    def py(v):  # eixo Y: speedup
        return mt + ph - (v / max_y) * ph

    s = []
    s.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" font-family="sans-serif">')
    s.append(f'<rect width="{W}" height="{H}" fill="white"/>')
    s.append(f'<text x="{W/2}" y="24" text-anchor="middle" font-size="16" font-weight="bold">Speedup vs numero de threads</text>')

    # eixos
    s.append(f'<line x1="{ml}" y1="{mt}" x2="{ml}" y2="{mt+ph}" stroke="black"/>')
    s.append(f'<line x1="{ml}" y1="{mt+ph}" x2="{ml+pw}" y2="{mt+ph}" stroke="black"/>')

    # marcas do eixo Y
    yt = 1
    while yt <= max_y:
        y = py(yt)
        s.append(f'<line x1="{ml-4}" y1="{y}" x2="{ml}" y2="{y}" stroke="black"/>')
        s.append(f'<text x="{ml-8}" y="{y+4}" text-anchor="end" font-size="11">{yt}x</text>')
        s.append(f'<line x1="{ml}" y1="{y}" x2="{ml+pw}" y2="{y}" stroke="#eee"/>')
        yt += 1

    # marcas do eixo X (nos pontos medidos)
    for t in threads:
        x = px(t)
        s.append(f'<line x1="{x}" y1="{mt+ph}" x2="{x}" y2="{mt+ph+4}" stroke="black"/>')
        s.append(f'<text x="{x}" y="{mt+ph+20}" text-anchor="middle" font-size="11">{t}</text>')
    s.append(f'<text x="{ml+pw/2}" y="{H-15}" text-anchor="middle" font-size="12">threads</text>')
    s.append(f'<text x="18" y="{mt+ph/2}" text-anchor="middle" font-size="12" transform="rotate(-90 18 {mt+ph/2})">speedup</text>')

    # linha ideal (speedup linear = numero de threads)
    s.append(f'<line x1="{px(1)}" y1="{py(1)}" x2="{px(max_t)}" y2="{py(max_t)}" '
             f'stroke="#bbb" stroke-dasharray="6 4"/>')
    s.append(f'<text x="{px(max_t)}" y="{py(max_t)-6}" text-anchor="end" font-size="11" fill="#888">ideal (linear)</text>')

    # linha do speedup observado
    pts = " ".join(f"{px(t):.1f},{py(sp):.1f}" for t, sp in zip(threads, speedups))
    s.append(f'<polyline points="{pts}" fill="none" stroke="#1f77b4" stroke-width="2.5"/>')
    for t, sp in zip(threads, speedups):
        s.append(f'<circle cx="{px(t):.1f}" cy="{py(sp):.1f}" r="4" fill="#1f77b4"/>')
        s.append(f'<text x="{px(t):.1f}" y="{py(sp)-10:.1f}" text-anchor="middle" font-size="11" fill="#1f77b4">{sp:.2f}x</text>')

    s.append('</svg>')
    with open(SVG_PATH, "w") as f:
        f.write("\n".join(s))
    print(f"Grafico escrito em {SVG_PATH}")


def main():
    tempos = carregar()
    seq_media, linhas = resumir(tempos)
    escrever_tabela(seq_media, linhas)
    escrever_svg(linhas)
    print()
    print(f"Baseline sequencial: {seq_media:.4f} s")
    for l in linhas:
        print(f"  {l['threads']:>2} threads: {l['media']:.4f} s  "
              f"speedup {l['speedup']:.2f}x  eficiencia {l['eficiencia']*100:.0f}%")


if __name__ == "__main__":
    main()
