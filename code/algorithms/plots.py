#!/usr/bin/env python3
"""
plots.py - Generador de gráficos y análisis para TDPDP Evolutivo.

Este módulo procesa los resultados almacenados en 'code/data/results' y los parámetros
en 'code/data/parameters_code', consolidando datasets y generando gráficos comparativos
organizados en 'code/data/plots'.

Uso:
    python code/algorithms/plots.py [--all] [--output code/data/plots]
"""

import sys
import os
from pathlib import Path

# Configurar directorio de configuración de matplotlib temporal para evitar problemas de permisos
os.environ.setdefault("MPLCONFIGDIR", "/tmp/matplotlib_cache")

# Si se ejecuta con un python sin matplotlib ni pandas, intentar usar el venv local
try:
    import matplotlib
    import pandas
except ImportError:
    repo_root = Path(__file__).resolve().parents[2]
    venv_py = repo_root / ".venv" / "bin" / "python"
    if venv_py.exists() and sys.executable != str(venv_py):
        os.execv(str(venv_py), [str(venv_py)] + sys.argv)
    else:
        raise

import argparse
import glob
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Usar backend Agg sin entorno gráfico
matplotlib.use("Agg")

# Estilo general de gráficos
plt.rcParams.update({
    "figure.titlesize": 14,
    "axes.titlesize": 12,
    "axes.labelsize": 11,
    "xtick.labelsize": 9,
    "ytick.labelsize": 9,
    "legend.fontsize": 9,
    "grid.alpha": 0.35,
    "grid.linestyle": "--",
    "figure.autolayout": False,
})

COLOR_GRASP = "#1f77b4"      # Azul sobrio
COLOR_RANDOM = "#ff7f0e"     # Naranja sobrio
COLOR_ACCENT = "#2ca02c"     # Verde
PALETTE = ["#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd", "#8c564b", "#e377c2", "#7f7f7f"]


def resolve_paths(base_dir=None):
    """Resuelve rutas absolutas relativas a la raíz del repositorio."""
    script_path = Path(__file__).resolve()
    repo_root = script_path.parents[2]  # INF295-TDPDP
    
    results_dir = repo_root / "code" / "data" / "results"
    params_dir = repo_root / "code" / "data" / "parameters_code"
    plots_dir = repo_root / "code" / "data" / "plots"

    return {
        "repo_root": repo_root,
        "results_dir": results_dir,
        "params_dir": params_dir,
        "plots_dir": plots_dir,
    }


def load_and_combine_results(results_dir, save_combined=True):
    """
    Carga results.csv y results-randomstart.csv, les añade la columna 'init_type'
    ('grasp' y 'random-start') y exporta 'results_combined.csv'.
    """
    results_dir = Path(results_dir)
    grasp_path = results_dir / "results.csv"
    rnd_path = results_dir / "results-randomstart.csv"

    if not grasp_path.exists():
        raise FileNotFoundError(f"No se encontró: {grasp_path}")
    if not rnd_path.exists():
        raise FileNotFoundError(f"No se encontró: {rnd_path}")

    df_grasp = pd.read_csv(grasp_path)
    df_grasp["init_type"] = "grasp"

    df_rnd = pd.read_csv(rnd_path)
    df_rnd["init_type"] = "random-start"

    df_combined = pd.concat([df_grasp, df_rnd], ignore_index=True)

    if save_combined:
        combined_path = results_dir / "results_combined.csv"
        df_combined.to_csv(combined_path, index=False)
        print(f"[✓] Archivo conjunto guardado en: {combined_path} ({len(df_combined)} registros)")

    return df_combined


def load_and_combine_iterations(results_dir, save_combined=True):
    """
    Carga results_per_iteration.csv y results_per_iteration-randomstart.csv,
    añade la columna 'init_type' y exporta 'results_per_iteration_combined.csv'.
    """
    results_dir = Path(results_dir)
    grasp_path = results_dir / "results_per_iteration.csv"
    rnd_path = results_dir / "results_per_iteration-randomstart.csv"

    if not grasp_path.exists():
        raise FileNotFoundError(f"No se encontró: {grasp_path}")
    if not rnd_path.exists():
        raise FileNotFoundError(f"No se encontró: {rnd_path}")

    df_grasp_iter = pd.read_csv(grasp_path)
    df_grasp_iter["init_type"] = "grasp"

    df_rnd_iter = pd.read_csv(rnd_path)
    df_rnd_iter["init_type"] = "random-start"

    df_iter_combined = pd.concat([df_grasp_iter, df_rnd_iter], ignore_index=True)

    if save_combined:
        combined_path = results_dir / "results_per_iteration_combined.csv"
        df_iter_combined.to_csv(combined_path, index=False)
        print(f"[✓] Archivo conjunto de iteraciones guardado en: {combined_path} ({len(df_iter_combined)} registros)")

    return df_iter_combined


def plot_seeds_by_instance(df_results, output_dir):
    """
    1. Gráfico que compara los resultados por instancia para los parámetros en 05_seeds.
    Genera un gráfico individual por cada una de las 20 instancias, mostrando
    las 5 semillas (seed=1, 2, 3, 4, 5) y comparando GRASP vs Random-Start.
    Además, genera un gráfico resumen consolidador.
    """
    out_dir = Path(output_dir) / "01_seeds_by_instance"
    out_dir.mkdir(parents=True, exist_ok=True)

    # Filtrar únicamente parámetros de 05_seeds: n=200, g=250, p_mut=0.2, p_cro=0.3, seed in [1..5]
    seeds_list = [1, 2, 3, 4, 5]
    df_seeds = df_results[
        (df_results["n"] == 200) &
        (df_results["g"] == 250) &
        (df_results["p_mut"] == 0.2) &
        (df_results["p_cro"] == 0.3) &
        (df_results["seed"].isin(seeds_list))
    ].copy()

    instances = sorted(df_seeds["instance"].unique())
    print(f"[*] Generando gráficos de 05_seeds para {len(instances)} instancias...")

    for inst in instances:
        sub = df_seeds[df_seeds["instance"] == inst]
        
        # Agrupar por semilla e init_type
        pivot = sub.pivot_table(index="seed", columns="init_type", values="eval", aggfunc="mean")
        
        fig, ax = plt.subplots(figsize=(8, 5), dpi=200)
        x = np.arange(len(seeds_list))
        width = 0.35

        grasp_vals = [pivot.loc[s, "grasp"] if (s in pivot.index and "grasp" in pivot.columns) else np.nan for s in seeds_list]
        rnd_vals = [pivot.loc[s, "random-start"] if (s in pivot.index and "random-start" in pivot.columns) else np.nan for s in seeds_list]

        rects1 = ax.bar(x - width/2, grasp_vals, width, label="GRASP (results.csv)", color=COLOR_GRASP, alpha=0.9, edgecolor="black", linewidth=0.5)
        rects2 = ax.bar(x + width/2, rnd_vals, width, label="Random-Start (randomstart.csv)", color=COLOR_RANDOM, alpha=0.9, edgecolor="black", linewidth=0.5)

        # Líneas de promedio
        mean_grasp = np.nanmean(grasp_vals)
        mean_rnd = np.nanmean(rnd_vals)
        ax.axhline(mean_grasp, color=COLOR_GRASP, linestyle="--", alpha=0.7, label=f"Media GRASP: {mean_grasp:.1f}")
        ax.axhline(mean_rnd, color=COLOR_RANDOM, linestyle="--", alpha=0.7, label=f"Media Random: {mean_rnd:.1f}")

        # Etiquetas con valores sobre barras
        for rect in rects1:
            h = rect.get_height()
            if not np.isnan(h):
                ax.annotate(f"{h:.1f}", xy=(rect.get_x() + rect.get_width() / 2, h),
                            xytext=(0, 3), textcoords="offset points", ha="center", va="bottom", fontsize=7.5, rotation=0)
        for rect in rects2:
            h = rect.get_height()
            if not np.isnan(h):
                ax.annotate(f"{h:.1f}", xy=(rect.get_x() + rect.get_width() / 2, h),
                            xytext=(0, 3), textcoords="offset points", ha="center", va="bottom", fontsize=7.5, rotation=0)

        # Ajuste de escala Y para apreciar diferencias
        min_v = np.nanmin([np.nanmin(grasp_vals), np.nanmin(rnd_vals)])
        max_v = np.nanmax([np.nanmax(grasp_vals), np.nanmax(rnd_vals)])
        padding = (max_v - min_v) * 0.25 if max_v > min_v else min_v * 0.1
        ax.set_ylim(max(0, min_v - padding), max_v + padding * 1.5)

        ax.set_title(f"Comparación por Semillas (05_seeds) - {inst}\n(n=200, g=250, mut=0.2, cro=0.3)", pad=10)
        ax.set_xlabel("Semilla (-s)")
        ax.set_ylabel("Fitness (Eval - Menor es mejor)")
        ax.set_xticks(x)
        ax.set_xticklabels([f"Seed {s}" for s in seeds_list])
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.legend(loc="upper right", frameon=True, framealpha=0.9)

        base_name = inst.replace(".txt", "")
        fig_path = out_dir / f"{base_name}.png"
        plt.tight_layout()
        fig.savefig(fig_path)
        plt.close(fig)

    # Gráfico Resumen general de 05_seeds
    fig, ax = plt.subplots(figsize=(14, 6), dpi=200)
    summary = df_seeds.groupby(["instance", "init_type"])["eval"].mean().unstack()
    inst_names = [i.replace("_new.txt", "").replace("Instance_", "") for i in summary.index]
    x = np.arange(len(inst_names))
    width = 0.38

    ax.bar(x - width/2, summary["grasp"], width, label="Media GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
    ax.bar(x + width/2, summary["random-start"], width, label="Media Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)

    ax.set_title("Resumen General: Media de Semillas (05_seeds) para todas las Instancias", pad=12)
    ax.set_xlabel("Instancia")
    ax.set_ylabel("Fitness Promedio (Eval)")
    ax.set_xticks(x)
    ax.set_xticklabels(inst_names, rotation=45, ha="right")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend(frameon=True)
    plt.tight_layout()
    summary_path = out_dir / "00_summary_all_instances.png"
    fig.savefig(summary_path)
    plt.close(fig)

    print(f"[✓] Gráficos de 05_seeds completados en: {out_dir}")


def plot_operator_balance(df_results, output_dir):
    """
    2. Gráfico que compara los resultados de los parámetros en:
       - code/data/parameters_code/02_operator_balance
       - code/data/parameters_code/01_zero_types
       Se identifican claramente los parámetros (ej: 'mut=0.1, cro=0.7').
    """
    out_dir = Path(output_dir) / "02_operator_balance"
    out_dir.mkdir(parents=True, exist_ok=True)

    # Configuraciones de 01_zero_types y 02_operator_balance (n=200, g=200, seed=-1)
    configs = [
        {"mut": 0.3, "cro": 0.0, "label": "mut=0.3, cro=0.0\n(no_cro)", "cat": "01_zero"},
        {"mut": 0.0, "cro": 0.6, "label": "mut=0.0, cro=0.6\n(no_mut)", "cat": "01_zero"},
        {"mut": 0.9, "cro": 0.0, "label": "mut=0.9, cro=0.0\n(random_walk)", "cat": "01_zero"},
        {"mut": 0.1, "cro": 0.7, "label": "mut=0.1, cro=0.7", "cat": "02_balance"},
        {"mut": 0.2, "cro": 0.6, "label": "mut=0.2, cro=0.6", "cat": "02_balance"},
        {"mut": 0.3, "cro": 0.3, "label": "mut=0.3, cro=0.3", "cat": "02_balance"},
        {"mut": 0.6, "cro": 0.1, "label": "mut=0.6, cro=0.1", "cat": "02_balance"},
    ]

    print("[*] Generando gráficos para 02_operator_balance y 01_zero_types...")

    # Filtrar datos relevantes
    sub_list = []
    for cfg in configs:
        mask = (
            (df_results["n"] == 200) &
            (df_results["g"] == 200) &
            (df_results["seed"] == -1) &
            (np.isclose(df_results["p_mut"], cfg["mut"])) &
            (np.isclose(df_results["p_cro"], cfg["cro"]))
        )
        match = df_results[mask].copy()
        match["param_label"] = cfg["label"]
        match["category"] = cfg["cat"]
        sub_list.append(match)

    df_ops = pd.concat(sub_list, ignore_index=True)

    # 2.A: Gráfico de 4 paneles dividido por tamaño de clientes c in [10, 50, 100, 200]
    fig, axes = plt.subplots(2, 2, figsize=(16, 12), dpi=200)
    axes = axes.flatten()
    c_sizes = [10, 50, 100, 200]
    labels = [c["label"] for c in configs]

    for idx, c_val in enumerate(c_sizes):
        ax = axes[idx]
        df_c = df_ops[df_ops["c"] == c_val]
        
        pivot = df_c.groupby(["param_label", "init_type"])["eval"].mean().unstack()
        pivot = pivot.reindex(labels)

        x = np.arange(len(labels))
        width = 0.38

        bars1 = ax.bar(x - width/2, pivot["grasp"], width, label="GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
        bars2 = ax.bar(x + width/2, pivot["random-start"], width, label="Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)

        # Destacar el mejor parámetro (menor valor)
        min_grasp_idx = np.nanargmin(pivot["grasp"].values)
        bars1[min_grasp_idx].set_edgecolor("#00ff00")
        bars1[min_grasp_idx].set_linewidth(1.8)

        ax.set_title(f"Instancias de c = {c_val} Clientes", fontweight="bold")
        ax.set_xlabel("Configuración de Parámetros")
        ax.set_ylabel("Fitness Promedio (Eval)")
        ax.set_xticks(x)
        ax.set_xticklabels(labels, rotation=30, ha="right", fontsize=8.5)
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.legend(loc="upper left")

    fig.suptitle("Comparación de Parámetros: Balance de Operadores (02) y Casos Límite (01)\n(n=200, g=200)", fontsize=15, y=0.995)
    plt.tight_layout()
    fig.savefig(out_dir / "operator_balance_by_c.png")
    plt.close(fig)

    # 2.B: Gráfico de Rendimiento Relativo Normalizado (% sobre el mejor resultado por instancia)
    # Permite comparar en una sola métrica agregada cómo se desempeñan en todas las 20 instancias
    best_per_instance = df_ops.groupby("instance")["eval"].transform("min")
    df_ops["rel_gap_pct"] = ((df_ops["eval"] - best_per_instance) / best_per_instance) * 100

    fig, ax = plt.subplots(figsize=(12, 6), dpi=200)
    gap_summary = df_ops.groupby(["param_label", "init_type"])["rel_gap_pct"].mean().unstack().reindex(labels)

    x = np.arange(len(labels))
    width = 0.38
    ax.bar(x - width/2, gap_summary["grasp"], width, label="GRASP (Gap % Promedio)", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
    ax.bar(x + width/2, gap_summary["random-start"], width, label="Random-Start (Gap % Promedio)", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)

    ax.set_title("Brecha Relativa al Mejor Resultado (% Gap respecto al Óptimo Local por Instancia)\nMenor porcentaje indica mayor efectividad global", pad=12)
    ax.set_xlabel("Configuración de Parámetros (01_zero_types & 02_operator_balance)")
    ax.set_ylabel("Brecha Promedio (%) respecto al mejor")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=25, ha="right")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend(frameon=True)
    plt.tight_layout()
    fig.savefig(out_dir / "operator_balance_relative_gap.png")
    plt.close(fig)

    print(f"[✓] Gráficos de Balance de Operadores completados en: {out_dir}")


def plot_ng_differences(df_results, output_dir):
    """
    3. Gráfico que compara los resultados de los parámetros en:
       - code/data/parameters_code/03_n_g_differences
       Se identifican claramente los parámetros (ej: 'n=50,g=400').
    """
    out_dir = Path(output_dir) / "03_n_g_differences"
    out_dir.mkdir(parents=True, exist_ok=True)

    configs = [
        {"n": 50, "g": 400, "label": "n=50, g=400\n(Alta explotación gen)", "short_label": "n=50, g=400"},
        {"n": 100, "g": 200, "label": "n=100, g=200\n(Equilibrado)", "short_label": "n=100, g=200"},
        {"n": 200, "g": 100, "label": "n=200, g=100\n(Alta exploración pop)", "short_label": "n=200, g=100"},
    ]

    print("[*] Generando gráficos para 03_n_g_differences...")

    sub_list = []
    for cfg in configs:
        mask = (
            (df_results["n"] == cfg["n"]) &
            (df_results["g"] == cfg["g"]) &
            (df_results["seed"] == -1) &
            (np.isclose(df_results["p_mut"], 0.2)) &
            (np.isclose(df_results["p_cro"], 0.3))
        )
        match = df_results[mask].copy()
        match["param_label"] = cfg["label"]
        match["short_label"] = f"n={cfg['n']}, g={cfg['g']}"
        sub_list.append(match)

    df_ng = pd.concat(sub_list, ignore_index=True)
    labels = [c["label"] for c in configs]
    short_labels = [c["short_label"] for c in configs]

    # 3.A: Gráfico de 4 paneles por tamaño de cliente
    fig, axes = plt.subplots(2, 2, figsize=(15, 10), dpi=200)
    axes = axes.flatten()
    c_sizes = [10, 50, 100, 200]

    for idx, c_val in enumerate(c_sizes):
        ax = axes[idx]
        df_c = df_ng[df_ng["c"] == c_val]
        pivot = df_c.groupby(["param_label", "init_type"])["eval"].mean().unstack().reindex(labels)

        x = np.arange(len(labels))
        width = 0.35

        ax.bar(x - width/2, pivot["grasp"], width, label="GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
        ax.bar(x + width/2, pivot["random-start"], width, label="Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)

        ax.set_title(f"Instancias con c = {c_val} Clientes", fontweight="bold")
        ax.set_xlabel("Presupuesto Fijo (20,000 evaluaciones totales)")
        ax.set_ylabel("Fitness Promedio (Eval)")
        ax.set_xticks(x)
        ax.set_xticklabels(labels, rotation=15, ha="right")
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.legend(loc="upper left")

    fig.suptitle("Comparación de Distribución de Presupuesto (03_n_g_differences)\nPresupuesto constante: N × G = 20,000", fontsize=14, y=0.995)
    plt.tight_layout()
    fig.savefig(out_dir / "n_g_differences_by_c.png")
    plt.close(fig)

    # 3.B: Gráfico de barras agrupadas comparando las 20 instancias
    fig, ax = plt.subplots(figsize=(15, 6), dpi=200)
    pivot_all = df_ng[df_ng["init_type"] == "grasp"].pivot_table(index="instance", columns="short_label", values="eval", aggfunc="mean")
    inst_clean = [i.replace("_new.txt", "").replace("Instance_", "") for i in pivot_all.index]
    
    x = np.arange(len(inst_clean))
    width = 0.25
    colors = ["#2b5c8f", "#388e3c", "#d32f2f"]

    for i, col in enumerate(short_labels):
        ax.bar(x + (i - 1) * width, pivot_all[col], width, label=col, color=colors[i], edgecolor="black", linewidth=0.5)

    ax.set_title("Comparación Detallada por Instancia (Inicio GRASP) - 03_n_g_differences", pad=12)
    ax.set_xlabel("Instancia")
    ax.set_ylabel("Fitness (Eval)")
    ax.set_xticks(x)
    ax.set_xticklabels(inst_clean, rotation=45, ha="right")
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend(title="Configuración", frameon=True)
    plt.tight_layout()
    fig.savefig(out_dir / "n_g_differences_instances_grasp.png")
    plt.close(fig)

    print(f"[✓] Gráficos de 03_n_g_differences completados en: {out_dir}")


def plot_scale_differences(df_results, output_dir):
    """
    4. Gráfico que compara los resultados de los parámetros en:
       - code/data/parameters_code/04_scale_differences
       Muestra la evolución del desempeño a medida que escala el esfuerzo computacional.
    """
    out_dir = Path(output_dir) / "04_scale_differences"
    out_dir.mkdir(parents=True, exist_ok=True)

    configs = [
        {"n": 30, "g": 100, "evals": 3000, "label": "n=30, g=100\n(3k evals)"},
        {"n": 50, "g": 200, "evals": 10000, "label": "n=50, g=200\n(10k evals)"},
        {"n": 100, "g": 200, "evals": 20000, "label": "n=100, g=200\n(20k evals)"},
        {"n": 200, "g": 150, "evals": 30000, "label": "n=200, g=150\n(30k evals)"},
        {"n": 200, "g": 250, "evals": 50000, "label": "n=200, g=250\n(50k evals)"},
        {"n": 250, "g": 200, "evals": 50000, "label": "n=250, g=200\n(50k evals)"},
    ]

    print("[*] Generando gráficos para 04_scale_differences...")

    sub_list = []
    for cfg in configs:
        mask = (
            (df_results["n"] == cfg["n"]) &
            (df_results["g"] == cfg["g"]) &
            (df_results["seed"] == -1) &
            (np.isclose(df_results["p_mut"], 0.2)) &
            (np.isclose(df_results["p_cro"], 0.3))
        )
        match = df_results[mask].copy()
        match["param_label"] = cfg["label"]
        match["evals"] = cfg["evals"]
        match["short_label"] = f"n={cfg['n']}, g={cfg['g']}"
        sub_list.append(match)

    df_scale = pd.concat(sub_list, ignore_index=True)
    labels = [c["label"] for c in configs]

    # 4.A: Gráfico de 4 paneles por tamaño de instancia c
    fig, axes = plt.subplots(2, 2, figsize=(16, 11), dpi=200)
    axes = axes.flatten()
    c_sizes = [10, 50, 100, 200]

    for idx, c_val in enumerate(c_sizes):
        ax = axes[idx]
        df_c = df_scale[df_scale["c"] == c_val]
        pivot = df_c.groupby(["param_label", "init_type"])["eval"].mean().unstack().reindex(labels)

        x = np.arange(len(labels))
        width = 0.38

        ax.bar(x - width/2, pivot["grasp"], width, label="GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
        ax.bar(x + width/2, pivot["random-start"], width, label="Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)

        ax.set_title(f"Instancias con c = {c_val} Clientes", fontweight="bold")
        ax.set_xlabel("Escalamiento de Recursos (n, g)")
        ax.set_ylabel("Fitness Promedio (Eval)")
        ax.set_xticks(x)
        ax.set_xticklabels(labels, rotation=25, ha="right", fontsize=8.5)
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.legend(loc="upper right")

    fig.suptitle("Impacto del Escalamiento de Recursos (04_scale_differences)\n(mut=0.2, cro=0.3)", fontsize=15, y=0.995)
    plt.tight_layout()
    fig.savefig(out_dir / "scale_differences_by_c.png")
    plt.close(fig)

    # 4.B: Curva de convergencia por esfuerzo total (evals vs fitness promedio)
    fig, ax = plt.subplots(figsize=(10, 6), dpi=200)
    # Normalizar fitness por instancia para poder trazar la curva agregada
    best_eval = df_scale.groupby("instance")["eval"].transform("min")
    df_scale["gap_to_best"] = ((df_scale["eval"] - best_eval) / best_eval) * 100

    effort_df = df_scale.groupby(["evals", "short_label", "init_type"])["gap_to_best"].mean().unstack().reset_index()
    effort_df = effort_df.sort_values("evals")

    ax.plot(effort_df["evals"], effort_df["grasp"], marker="o", linewidth=2, color=COLOR_GRASP, label="GRASP (Gap % Promedio)")
    ax.plot(effort_df["evals"], effort_df["random-start"], marker="s", linewidth=2, color=COLOR_RANDOM, label="Random-Start (Gap % Promedio)")

    for _, row in effort_df.iterrows():
        ax.annotate(row["short_label"], (row["evals"], row["grasp"]), textcoords="offset points", xytext=(0, 10), ha="center", fontsize=8)

    ax.set_title("Curva de Rendimiento vs Presupuesto Total de Evaluaciones", pad=12)
    ax.set_xlabel("Total de Evaluaciones de Fitness (N × G)")
    ax.set_ylabel("Brecha Promedio respecto al Mejor (%)")
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend(frameon=True)
    plt.tight_layout()
    fig.savefig(out_dir / "scale_evaluation_curve.png")
    plt.close(fig)

    print(f"[✓] Gráficos de 04_scale_differences completados en: {out_dir}")


def plot_iterations_by_instance(df_iterations, output_dir):
    """
    5. Gráfico de puntos/línea que muestra el avance de cada iteración para cada instancia,
       únicamente de aquellos parámetros en:
       - code/data/parameters_code/05_seeds (seeds 1 a 5)
       - code/data/parameters_code/00_reference/param_tuned_g250 (seed=-1)
       Cada línea corresponde al avance de un parámetro distinto.
       Genera un gráfico para cada instancia (20 instancias).
    """
    out_dir = Path(output_dir) / "05_iterations_by_instance"
    out_dir.mkdir(parents=True, exist_ok=True)

    # Filtrar parámetros objetivo: n=200, g=250, mut=0.2, cro=0.3, seed in [-1, 1, 2, 3, 4, 5]
    seeds_target = [-1, 1, 2, 3, 4, 5]
    mask = (
        (df_iterations["n"] == 200) &
        (df_iterations["g"] == 250) &
        (np.isclose(df_iterations["p_mut"], 0.2)) &
        (np.isclose(df_iterations["p_cro"], 0.3)) &
        (df_iterations["seed"].isin(seeds_target))
    )
    df_target = df_iterations[mask].copy()

    # Si hay ejecuciones duplicadas para seed=-1 (por aparecer en 00_reference y 04_scale), tomar la primera corrida de 250 iteraciones
    df_target = df_target.drop_duplicates(subset=["instance", "init_type", "seed", "i"], keep="first")

    instances = sorted(df_target["instance"].unique())
    print(f"[*] Generando curvas de iteración para {len(instances)} instancias (seeds 1..5 y tuned_g250)...")

    # Mapeo de parámetros y estilos de línea
    param_styles = {
        -1: {"label": "tuned_g250 (seed default -1)", "color": "#000000", "marker": "o", "ls": "-"},
        1:  {"label": "tuned_g250_seed1", "color": "#1f77b4", "marker": "s", "ls": "-"},
        2:  {"label": "tuned_g250_seed2", "color": "#ff7f0e", "marker": "^", "ls": "-"},
        3:  {"label": "tuned_g250_seed3", "color": "#2ca02c", "marker": "v", "ls": "-"},
        4:  {"label": "tuned_g250_seed4", "color": "#d62728", "marker": "D", "ls": "-"},
        5:  {"label": "tuned_g250_seed5", "color": "#9467bd", "marker": "p", "ls": "-"},
    }

    for inst in instances:
        sub_inst = df_target[df_target["instance"] == inst]

        # Crear figura con 2 subplots lado a lado: Panel Izquierdo (GRASP), Panel Derecho (Random-Start)
        fig, (ax_grasp, ax_rnd) = plt.subplots(1, 2, figsize=(16, 6), sharey=False, dpi=200)

        for init_type, ax, title_prefix in [
            ("grasp", ax_grasp, "Inicio GRASP (results_per_iteration.csv)"),
            ("random-start", ax_rnd, "Inicio Aleatorio (randomstart.csv)"),
        ]:
            sub_init = sub_inst[sub_inst["init_type"] == init_type]

            for s in seeds_target:
                df_seed = sub_init[sub_init["seed"] == s].sort_values("i")
                if df_seed.empty:
                    continue
                style = param_styles[s]
                # Graficar línea y puntos (muestreando cada 20 iteraciones para claridad visual)
                ax.plot(
                    df_seed["i"], df_seed["eval"],
                    label=style["label"],
                    color=style["color"],
                    linestyle=style["ls"],
                    linewidth=1.5,
                    marker=style["marker"],
                    markevery=20,
                    markersize=5,
                    alpha=0.85
                )

            ax.set_title(f"{title_prefix}", fontweight="bold", pad=10)
            ax.set_xlabel("Iteración / Generación (i)")
            ax.set_ylabel("Fitness Mejor Histórico (Eval)")
            ax.grid(True, linestyle="--", alpha=0.35)
            ax.legend(loc="upper right", frameon=True, fontsize=8.5)

        fig.suptitle(f"Avance por Iteración - {inst}\nParámetros: 05_seeds & 00_reference/param_tuned_g250 (n=200, g=250, mut=0.2, cro=0.3)", fontsize=13, y=0.99)
        plt.tight_layout()
        base_name = inst.replace(".txt", "")
        fig.savefig(out_dir / f"{base_name}_iteration.png")
        plt.close(fig)

    print(f"[✓] Gráficos de iteración completados en: {out_dir}")


def plot_runtime(df_results, output_dir):
    """
    6. Gráfico que compara los tiempos de ejecución que toma cada instancia en terminar (en promedio).
    Incluye desglose por método de inicialización (GRASP vs Random-Start) y tiempo medio global.
    """
    out_dir = Path(output_dir) / "06_runtime"
    out_dir.mkdir(parents=True, exist_ok=True)

    print("[*] Generando gráficos de tiempos de ejecución...")

    # Calcular estadísticas de tiempo por instancia e init_type
    runtime_summary = df_results.groupby(["instance", "c", "init_type"])["runtime"].agg(["mean", "std"]).reset_index()
    
    # Ordenar instancias por c y luego por nombre
    instances_order = df_results.sort_values(by=["c", "instance"])["instance"].unique()
    clean_labels = [i.replace("_new.txt", "").replace("Instance_", "") for i in instances_order]

    pivot_mean = df_results.pivot_table(index="instance", columns="init_type", values="runtime", aggfunc="mean").reindex(instances_order)
    overall_mean = df_results.groupby("instance")["runtime"].mean().reindex(instances_order)
    overall_std = df_results.groupby("instance")["runtime"].std().reindex(instances_order)

    # 6.A: Gráfico de barras horizontal o vertical para las 20 instancias
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10), dpi=200)

    x = np.arange(len(instances_order))
    width = 0.38

    # Panel superior: Escala lineal
    ax1.bar(x - width/2, pivot_mean["grasp"], width, label="Media GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
    ax1.bar(x + width/2, pivot_mean["random-start"], width, label="Media Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)
    ax1.plot(x, overall_mean, color="black", marker="o", linewidth=1.5, linestyle=":", label="Media Global")

    ax1.set_title("Tiempo de Ejecución Promedio por Instancia (Escala Lineal)", pad=10)
    ax1.set_ylabel("Tiempo de Ejecución (segundos)")
    ax1.set_xticks(x)
    ax1.set_xticklabels(clean_labels, rotation=45, ha="right")
    ax1.grid(axis="y", linestyle="--", alpha=0.4)
    ax1.legend(loc="upper left")

    # Panel inferior: Escala logarítmica (para distinguir con claridad las instancias pequeñas c=10, 50)
    ax2.bar(x - width/2, pivot_mean["grasp"], width, label="Media GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
    ax2.bar(x + width/2, pivot_mean["random-start"], width, label="Media Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)
    ax2.plot(x, overall_mean, color="black", marker="o", linewidth=1.5, linestyle=":", label="Media Global")

    ax2.set_yscale("log")
    ax2.set_title("Tiempo de Ejecución Promedio por Instancia (Escala Logarítmica)", pad=10)
    ax2.set_xlabel("Instancia")
    ax2.set_ylabel("Tiempo de Ejecución (s, log scale)")
    ax2.set_xticks(x)
    ax2.set_xticklabels(clean_labels, rotation=45, ha="right")
    ax2.grid(axis="y", which="both", linestyle="--", alpha=0.3)
    ax2.legend(loc="upper left")

    # Líneas divisorias visuales entre grupos de c
    for div_pos in [4.5, 9.5, 14.5]:
        ax1.axvline(div_pos, color="gray", linestyle="-.", alpha=0.6)
        ax2.axvline(div_pos, color="gray", linestyle="-.", alpha=0.6)

    fig.suptitle("Comparación de Tiempo de Ejecución por Instancia (Runtime Promedio)", fontsize=14, y=0.995)
    plt.tight_layout()
    fig.savefig(out_dir / "runtime_by_instance.png")
    plt.close(fig)

    # 6.B: Gráfico por tamaño de clientes c in [10, 50, 100, 200]
    fig, ax = plt.subplots(figsize=(8, 5), dpi=200)
    c_runtime = df_results.groupby(["c", "init_type"])["runtime"].mean().unstack()
    c_labels = [f"c = {c}" for c in c_runtime.index]
    x_c = np.arange(len(c_labels))

    ax.bar(x_c - width/2, c_runtime["grasp"], width, label="GRASP", color=COLOR_GRASP, edgecolor="black", linewidth=0.5)
    ax.bar(x_c + width/2, c_runtime["random-start"], width, label="Random-Start", color=COLOR_RANDOM, edgecolor="black", linewidth=0.5)

    for i in x_c:
        vg = c_runtime.loc[c_runtime.index[i], "grasp"]
        vr = c_runtime.loc[c_runtime.index[i], "random-start"]
        ax.annotate(f"{vg:.2f}s", (i - width/2, vg), textcoords="offset points", xytext=(0, 4), ha="center", fontsize=8)
        ax.annotate(f"{vr:.2f}s", (i + width/2, vr), textcoords="offset points", xytext=(0, 4), ha="center", fontsize=8)

    ax.set_title("Tiempo de Ejecución Promedio según Número de Clientes (c)", pad=12)
    ax.set_xlabel("Tamaño de Problema (c)")
    ax.set_ylabel("Tiempo Promedio (segundos)")
    ax.set_xticks(x_c)
    ax.set_xticklabels(c_labels)
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend()
    plt.tight_layout()
    fig.savefig(out_dir / "runtime_by_client_size.png")
    plt.close(fig)

    print(f"[✓] Gráficos de runtime completados en: {out_dir}")


def plot_boxplots(df_results, output_dir):
    """
    Caso B: Diagramas de Cajas y Violín (Boxplots / Violin Plots)
    Evalúa la dispersión y estabilidad estocástica de los resultados:
    1. Dispersión de semillas (05_seeds) comparando GRASP vs Random-Start.
    2. Dispersión de la brecha relativa (% Gap) para configuraciones de operadores (01 y 02).
    """
    out_dir = Path(output_dir) / "07_boxplots"
    out_dir.mkdir(parents=True, exist_ok=True)
    print("[*] Generando diagramas de cajas (Caso B: Boxplots)...")

    # Calcular Brecha Relativa (% Gap respecto al mejor por instancia)
    df = df_results.copy()
    best_per_inst = df.groupby("instance")["eval"].transform("min")
    df["gap_pct"] = ((df["eval"] - best_per_inst) / best_per_inst) * 100

    # B.1: Boxplot de 05_seeds comparando dispersión entre GRASP y Random-Start por tamaño c
    df_seeds = df[
        (df["n"] == 200) &
        (df["g"] == 250) &
        (np.isclose(df["p_mut"], 0.2)) &
        (np.isclose(df["p_cro"], 0.3)) &
        (df["seed"].isin([1, 2, 3, 4, 5]))
    ].copy()

    fig, ax = plt.subplots(figsize=(10, 6), dpi=200)
    c_sizes = [10, 50, 100, 200]
    positions = []
    data_to_plot = []
    colors = []

    pos = 1
    for c_val in c_sizes:
        sub_c = df_seeds[df_seeds["c"] == c_val]
        grasp_vals = sub_c[sub_c["init_type"] == "grasp"]["gap_pct"].dropna()
        rnd_vals = sub_c[sub_c["init_type"] == "random-start"]["gap_pct"].dropna()

        data_to_plot.append(grasp_vals)
        positions.append(pos)
        colors.append(COLOR_GRASP)

        data_to_plot.append(rnd_vals)
        positions.append(pos + 0.8)
        colors.append(COLOR_RANDOM)

        pos += 2.5

    bp = ax.boxplot(data_to_plot, positions=positions, widths=0.6, patch_artist=True, showmeans=True, meanline=True)

    for patch, col in zip(bp['boxes'], colors):
        patch.set_facecolor(col)
        patch.set_alpha(0.7)
    for median in bp['medians']:
        median.set_color("black")
        median.set_linewidth(1.5)
    for mean in bp['means']:
        mean.set_color("red")
        mean.set_linewidth(1.5)

    ax.set_title("Dispersión entre Semillas (05_seeds) según Tamaño de Instancia (Boxplot)\nComparación de Robustez: GRASP vs Random-Start", pad=12)
    ax.set_xlabel("Tamaño de Problema (c)")
    ax.set_ylabel("Brecha respecto al Mejor (% Gap)")
    xticks = [1.4 + i * 2.5 for i in range(len(c_sizes))]
    ax.set_xticks(xticks)
    ax.set_xticklabels([f"c = {c}" for c in c_sizes])
    ax.grid(axis="y", linestyle="--", alpha=0.4)

    # Leyenda personalizada
    from matplotlib.patches import Patch
    from matplotlib.lines import Line2D
    legend_elements = [
        Patch(facecolor=COLOR_GRASP, alpha=0.7, label="GRASP (seeds 1..5)"),
        Patch(facecolor=COLOR_RANDOM, alpha=0.7, label="Random-Start (seeds 1..5)"),
        Line2D([0], [0], color="black", lw=1.5, label="Mediana"),
        Line2D([0], [0], color="red", lw=1.5, linestyle="--", label="Media")
    ]
    ax.legend(handles=legend_elements, loc="upper left")
    plt.tight_layout()
    fig.savefig(out_dir / "boxplot_seeds_dispersion_by_c.png")
    plt.close(fig)

    # B.1.a y B.1.b: Gráficos de 05_seeds separados para GRASP y Random-Start
    for init_t, col, label_t, suffix in [
        ("grasp", COLOR_GRASP, "Inicio GRASP", "grasp"),
        ("random-start", COLOR_RANDOM, "Inicio Aleatorio", "randomstart")
    ]:
        fig, ax = plt.subplots(figsize=(8, 5), dpi=200)
        c_data = [df_seeds[(df_seeds["c"] == c) & (df_seeds["init_type"] == init_t)]["gap_pct"].dropna() for c in c_sizes]
        bp = ax.boxplot(c_data, positions=range(1, len(c_sizes) + 1), widths=0.5, patch_artist=True, showmeans=True, meanline=True)
        for patch in bp['boxes']:
            patch.set_facecolor(col)
            patch.set_alpha(0.7)
        for median in bp['medians']:
            median.set_color("black")
            median.set_linewidth(1.5)
        for mean in bp['means']:
            mean.set_color("red")
            mean.set_linewidth(1.5)

        ax.set_title(f"Dispersión entre Semillas (05_seeds) - {label_t}\n(n=200, g=250, mut=0.2, cro=0.3)", pad=12)
        ax.set_xlabel("Tamaño de Problema (c)")
        ax.set_ylabel("Brecha respecto al Mejor (% Gap)")
        ax.set_xticks(range(1, len(c_sizes) + 1))
        ax.set_xticklabels([f"c = {c}" for c in c_sizes])
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.legend(handles=legend_elements[2:], loc="upper right")
        plt.tight_layout()
        fig.savefig(out_dir / f"boxplot_seeds_dispersion_by_c_{suffix}.png")
        plt.close(fig)

    # B.2: Boxplot de Balance de Operadores (01_zero_types & 02_operator_balance)
    configs_ops = [
        {"mut": 0.3, "cro": 0.0, "label": "mut=0.3\ncro=0.0"},
        {"mut": 0.0, "cro": 0.6, "label": "mut=0.0\ncro=0.6"},
        {"mut": 0.9, "cro": 0.0, "label": "mut=0.9\ncro=0.0"},
        {"mut": 0.1, "cro": 0.7, "label": "mut=0.1\ncro=0.7"},
        {"mut": 0.2, "cro": 0.6, "label": "mut=0.2\ncro=0.6"},
        {"mut": 0.3, "cro": 0.3, "label": "mut=0.3\ncro=0.3"},
        {"mut": 0.6, "cro": 0.1, "label": "mut=0.6\ncro=0.1"},
    ]
    df_op_sub = []
    for cfg in configs_ops:
        mask = (
            (df["n"] == 200) &
            (df["g"] == 200) &
            (df["seed"] == -1) &
            (np.isclose(df["p_mut"], cfg["mut"])) &
            (np.isclose(df["p_cro"], cfg["cro"]))
        )
        m = df[mask].copy()
        m["param_label"] = cfg["label"]
        df_op_sub.append(m)

    df_op_sub = pd.concat(df_op_sub, ignore_index=True)

    fig, ax = plt.subplots(figsize=(14, 6), dpi=200)
    op_labels = [c["label"] for c in configs_ops]
    positions = []
    data_to_plot = []
    colors = []

    pos = 1
    for lbl in op_labels:
        sub_l = df_op_sub[df_op_sub["param_label"] == lbl]
        grasp_vals = sub_l[sub_l["init_type"] == "grasp"]["gap_pct"].dropna()
        rnd_vals = sub_l[sub_l["init_type"] == "random-start"]["gap_pct"].dropna()

        data_to_plot.append(grasp_vals)
        positions.append(pos)
        colors.append(COLOR_GRASP)

        data_to_plot.append(rnd_vals)
        positions.append(pos + 0.7)
        colors.append(COLOR_RANDOM)

        pos += 2.2

    bp = ax.boxplot(data_to_plot, positions=positions, widths=0.55, patch_artist=True, showmeans=True, meanline=True)
    for patch, col in zip(bp['boxes'], colors):
        patch.set_facecolor(col)
        patch.set_alpha(0.7)
    for median in bp['medians']:
        median.set_color("black")
        median.set_linewidth(1.5)
    for mean in bp['means']:
        mean.set_color("red")
        mean.set_linewidth(1.5)

    ax.set_title("Dispersión y Estabilidad de Operadores (01_zero_types & 02_operator_balance)\nDistribución de % Gap sobre las 20 Instancias", pad=12)
    ax.set_xlabel("Configuración de Operadores")
    ax.set_ylabel("Brecha respecto al Mejor (% Gap)")
    xticks = [1.35 + i * 2.2 for i in range(len(op_labels))]
    ax.set_xticks(xticks)
    ax.set_xticklabels(op_labels)
    ax.grid(axis="y", linestyle="--", alpha=0.4)
    ax.legend(handles=legend_elements[:2], loc="upper right")
    plt.tight_layout()
    fig.savefig(out_dir / "boxplot_operators_relative_gap.png")
    plt.close(fig)

    # B.2.a y B.2.b: Gráficos de operadores separados para GRASP y Random-Start
    for init_t, col, label_t, suffix in [
        ("grasp", COLOR_GRASP, "Inicio GRASP", "grasp"),
        ("random-start", COLOR_RANDOM, "Inicio Aleatorio", "randomstart")
    ]:
        fig, ax = plt.subplots(figsize=(10, 5), dpi=200)
        op_data = [df_op_sub[(df_op_sub["param_label"] == lbl) & (df_op_sub["init_type"] == init_t)]["gap_pct"].dropna() for lbl in op_labels]
        bp = ax.boxplot(op_data, positions=range(1, len(op_labels) + 1), widths=0.5, patch_artist=True, showmeans=True, meanline=True)
        for patch in bp['boxes']:
            patch.set_facecolor(col)
            patch.set_alpha(0.7)
        for median in bp['medians']:
            median.set_color("black")
            median.set_linewidth(1.5)
        for mean in bp['means']:
            mean.set_color("red")
            mean.set_linewidth(1.5)

        ax.set_title(f"Dispersión de Operadores (01 & 02) - {label_t}\nDistribución de % Gap sobre las 20 Instancias", pad=12)
        ax.set_xlabel("Configuración de Operadores")
        ax.set_ylabel("Brecha respecto al Mejor (% Gap)")
        ax.set_xticks(range(1, len(op_labels) + 1))
        ax.set_xticklabels(op_labels)
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.legend(handles=legend_elements[2:], loc="upper right")
        plt.tight_layout()
        fig.savefig(out_dir / f"boxplot_operators_relative_gap_{suffix}.png")
        plt.close(fig)

    print(f"[✓] Gráficos de Boxplots (Caso B) completados en: {out_dir}")


def plot_tradeoff_quality_runtime(df_results, output_dir):
    """
    Caso C: Frente de Trade-off (Calidad vs. Tiempo de Ejecución).
    Eje X: Runtime promedio (segundos, escala logarítmica).
    Eje Y: Calidad promedio (% Gap respecto al mejor por instancia).
    Traza el frente de Pareto de soluciones no dominadas.
    """
    out_dir = Path(output_dir) / "08_tradeoff_quality_runtime"
    out_dir.mkdir(parents=True, exist_ok=True)
    print("[*] Generando frentes de Trade-off Calidad vs Tiempo (Caso C)...")

    df = df_results.copy()
    best_per_inst = df.groupby("instance")["eval"].transform("min")
    df["gap_pct"] = ((df["eval"] - best_per_inst) / best_per_inst) * 100

    # C.1: Trade-off Global de todas las configuraciones evaluadas
    summary = df.groupby(["init_type", "n", "g", "p_mut", "p_cro", "seed"]).agg(
        mean_gap=("gap_pct", "mean"),
        mean_time=("runtime", "mean"),
    ).reset_index()

    fig, ax = plt.subplots(figsize=(11, 7), dpi=200)

    for init_type, color, marker, label_name in [
        ("grasp", COLOR_GRASP, "o", "GRASP"),
        ("random-start", COLOR_RANDOM, "s", "Random-Start")
    ]:
        sub = summary[summary["init_type"] == init_type].sort_values("mean_time")
        ax.scatter(sub["mean_time"], sub["mean_gap"], color=color, marker=marker, s=60, alpha=0.75, edgecolor="black", linewidth=0.5, label=f"Configuraciones {label_name}")

        # Calcular y trazar Frontera de Pareto
        pts = sub[["mean_time", "mean_gap"]].values
        pareto_x, pareto_y = [], []
        min_gap_seen = float("inf")
        for x_val, y_val in pts:
            if y_val < min_gap_seen:
                pareto_x.append(x_val)
                pareto_y.append(y_val)
                min_gap_seen = y_val

        if pareto_x:
            ax.step(pareto_x, pareto_y, where="post", color=color, linestyle="--", linewidth=1.8, alpha=0.9, label=f"Frontera Pareto ({label_name})")

    # Anotar configuraciones destacadas en la frontera Pareto de GRASP
    grasp_sub = summary[summary["init_type"] == "grasp"].sort_values("mean_time")
    min_g = float("inf")
    for _, row in grasp_sub.iterrows():
        if row["mean_gap"] < min_g:
            min_g = row["mean_gap"]
            cfg_lbl = f"n={int(row['n'])}, g={int(row['g'])}\n(m={row['p_mut']}, c={row['p_cro']})"
            ax.annotate(
                cfg_lbl,
                (row["mean_time"], row["mean_gap"]),
                xytext=(10, 8),
                textcoords="offset points",
                fontsize=7.5,
                fontweight="bold",
                bbox=dict(boxstyle="round,pad=0.2", fc="white", ec=COLOR_GRASP, lw=1),
                arrowprops=dict(arrowstyle="->", connectionstyle="arc3,rad=0", color=COLOR_GRASP)
            )

    ax.set_xscale("log")
    ax.set_title("Trade-off: Calidad vs. Tiempo de Cómputo (Frontera de Pareto)\nCompromiso entre Esfuerzo de CPU y Proximidad al Óptimo", pad=12)
    ax.set_xlabel("Tiempo Medio de Ejecución (segundos, escala logarítmica)")
    ax.set_ylabel("Brecha Promedio respecto al Mejor (% Gap)")
    ax.grid(True, which="both", linestyle="--", alpha=0.35)
    ax.legend(frameon=True, loc="upper right")
    plt.tight_layout()
    fig.savefig(out_dir / "pareto_tradeoff_all_configs.png")
    plt.close(fig)

    # C.1.a y C.1.b: Gráficos de Pareto individuales para GRASP y Random-Start
    for init_type, color, marker, label_name, suffix in [
        ("grasp", COLOR_GRASP, "o", "Inicio GRASP", "grasp"),
        ("random-start", COLOR_RANDOM, "s", "Inicio Aleatorio", "randomstart")
    ]:
        fig, ax = plt.subplots(figsize=(9, 6), dpi=200)
        sub = summary[summary["init_type"] == init_type].sort_values("mean_time")
        ax.scatter(sub["mean_time"], sub["mean_gap"], color=color, marker=marker, s=65, alpha=0.8, edgecolor="black", linewidth=0.5, label=f"Configuraciones ({label_name})")

        pts = sub[["mean_time", "mean_gap"]].values
        pareto_x, pareto_y = [], []
        min_gap_seen = float("inf")
        for x_val, y_val in pts:
            if y_val < min_gap_seen:
                pareto_x.append(x_val)
                pareto_y.append(y_val)
                min_gap_seen = y_val

        if pareto_x:
            ax.step(pareto_x, pareto_y, where="post", color=color, linestyle="--", linewidth=1.8, alpha=0.9, label=f"Frontera Pareto ({label_name})")

        min_g = float("inf")
        for _, row in sub.iterrows():
            if row["mean_gap"] < min_g:
                min_g = row["mean_gap"]
                cfg_lbl = f"n={int(row['n'])}, g={int(row['g'])}\n(m={row['p_mut']}, c={row['p_cro']})"
                ax.annotate(
                    cfg_lbl,
                    (row["mean_time"], row["mean_gap"]),
                    xytext=(10, 8),
                    textcoords="offset points",
                    fontsize=7.5,
                    fontweight="bold",
                    bbox=dict(boxstyle="round,pad=0.2", fc="white", ec=color, lw=1),
                    arrowprops=dict(arrowstyle="->", connectionstyle="arc3,rad=0", color=color)
                )

        ax.set_xscale("log")
        ax.set_title(f"Frente de Trade-off (Calidad vs. Tiempo) - {label_name}", pad=12)
        ax.set_xlabel("Tiempo Medio de Ejecución (segundos, escala logarítmica)")
        ax.set_ylabel("Brecha Promedio respecto al Mejor (% Gap)")
        ax.grid(True, which="both", linestyle="--", alpha=0.35)
        ax.legend(frameon=True, loc="upper right")
        plt.tight_layout()
        fig.savefig(out_dir / f"pareto_tradeoff_all_configs_{suffix}.png")
        plt.close(fig)

    # C.2: Trade-off enfocado en Escalamiento y Presupuesto
    df_budget = df[(df["seed"] == -1) & (np.isclose(df["p_mut"], 0.2)) & (np.isclose(df["p_cro"], 0.3))].copy()
    sum_budget = df_budget.groupby(["init_type", "n", "g"]).agg(
        mean_gap=("gap_pct", "mean"),
        mean_time=("runtime", "mean"),
    ).reset_index()

    fig, ax = plt.subplots(figsize=(10, 6), dpi=200)
    for init_type, color, marker, label_name in [
        ("grasp", COLOR_GRASP, "o", "GRASP"),
        ("random-start", COLOR_RANDOM, "s", "Random-Start")
    ]:
        sub = sum_budget[sum_budget["init_type"] == init_type].sort_values("mean_time")
        ax.plot(sub["mean_time"], sub["mean_gap"], marker=marker, color=color, linewidth=1.8, markersize=7, label=label_name)
        for _, row in sub.iterrows():
            ax.annotate(f"n={int(row['n'])}, g={int(row['g'])}", (row["mean_time"], row["mean_gap"]),
                        xytext=(5, 5), textcoords="offset points", fontsize=8)

    ax.set_title("Curva de Escalamiento: Tiempo de Cómputo vs Calidad (% Gap)\n(Parámetros de 03_n_g_differences y 04_scale_differences)", pad=12)
    ax.set_xlabel("Tiempo Promedio (segundos)")
    ax.set_ylabel("Brecha Promedio respecto al Mejor (% Gap)")
    ax.grid(True, linestyle="--", alpha=0.4)
    ax.legend()
    plt.tight_layout()
    fig.savefig(out_dir / "pareto_tradeoff_budget_scale.png")
    plt.close(fig)

    # C.2.a y C.2.b: Curva de escalamiento individual para GRASP y Random-Start
    for init_type, color, marker, label_name, suffix in [
        ("grasp", COLOR_GRASP, "o", "Inicio GRASP", "grasp"),
        ("random-start", COLOR_RANDOM, "s", "Inicio Aleatorio", "randomstart")
    ]:
        fig, ax = plt.subplots(figsize=(8, 5), dpi=200)
        sub = sum_budget[sum_budget["init_type"] == init_type].sort_values("mean_time")
        ax.plot(sub["mean_time"], sub["mean_gap"], marker=marker, color=color, linewidth=2.0, markersize=8, label=label_name)
        for _, row in sub.iterrows():
            ax.annotate(f"n={int(row['n'])}, g={int(row['g'])}", (row["mean_time"], row["mean_gap"]),
                        xytext=(5, 5), textcoords="offset points", fontsize=8)

        ax.set_title(f"Curva de Escalamiento (Tiempo vs Calidad) - {label_name}", pad=12)
        ax.set_xlabel("Tiempo Promedio (segundos)")
        ax.set_ylabel("Brecha Promedio respecto al Mejor (% Gap)")
        ax.grid(True, linestyle="--", alpha=0.4)
        ax.legend()
        plt.tight_layout()
        fig.savefig(out_dir / f"pareto_tradeoff_budget_scale_{suffix}.png")
        plt.close(fig)

    print(f"[✓] Gráficos de Trade-off (Caso C) completados en: {out_dir}")


def plot_heatmaps(df_results, output_dir):
    """
    Caso D: Mapas de Calor (Heatmaps) de Interacción de Parámetros.
    Genera matrices bidimensionales con código de color representando la brecha relativa promedio (% Gap):
    1. Interacción de Operadores (p_mut vs p_cro).
    2. Interacción de Tamaño de Población (N) vs Generaciones (G).
    """
    out_dir = Path(output_dir) / "09_heatmaps"
    out_dir.mkdir(parents=True, exist_ok=True)
    print("[*] Generando mapas de calor de parámetros (Caso D: Heatmaps)...")

    df = df_results.copy()
    best_per_inst = df.groupby("instance")["eval"].transform("min")
    df["gap_pct"] = ((df["eval"] - best_per_inst) / best_per_inst) * 100

    # D.1: Heatmap de Operadores (p_mut vs p_cro)
    df_ops = df[(df["n"] == 200) & (df["g"] == 200) & (df["seed"] == -1)].copy()

    mut_vals = sorted(df_ops["p_mut"].round(2).unique())
    cro_vals = sorted(df_ops["p_cro"].round(2).unique())

    fig, (ax_g, ax_r) = plt.subplots(1, 2, figsize=(15, 6), dpi=200)

    for init_type, ax, title in [
        ("grasp", ax_g, "Inicio GRASP (p_mut vs p_cro)"),
        ("random-start", ax_r, "Inicio Aleatorio (p_mut vs p_cro)")
    ]:
        sub = df_ops[df_ops["init_type"] == init_type]
        pivot = sub.pivot_table(index="p_mut", columns="p_cro", values="gap_pct", aggfunc="mean")
        pivot = pivot.reindex(index=mut_vals, columns=cro_vals)

        matrix_data = pivot.values
        masked_data = np.ma.masked_invalid(matrix_data)

        cmap = plt.get_cmap("YlGnBu_r").copy()
        cmap.set_bad(color="#eeeeee")

        im = ax.imshow(masked_data, cmap=cmap, aspect="auto", origin="lower",
                       vmin=0, vmax=np.nanmax(df_ops["gap_pct"]) * 0.7)

        ax.set_xticks(range(len(cro_vals)))
        ax.set_xticklabels([f"{c:.1f}" for c in cro_vals])
        ax.set_yticks(range(len(mut_vals)))
        ax.set_yticklabels([f"{m:.1f}" for m in mut_vals])
        ax.set_xlabel("Probabilidad de Cruce (p_cro)")
        ax.set_ylabel("Probabilidad de Mutación (p_mut)")
        ax.set_title(title, fontweight="bold", pad=10)

        for r in range(len(mut_vals)):
            for c in range(len(cro_vals)):
                val = matrix_data[r, c]
                if not np.isnan(val):
                    txt_color = "white" if val > 15 else "black"
                    ax.text(c, r, f"{val:.1f}%", ha="center", va="center", color=txt_color, fontsize=9, fontweight="bold")
                else:
                    ax.text(c, r, "-", ha="center", va="center", color="#999999")

        cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
        cbar.set_label("Brecha Promedio (% Gap)")

    fig.suptitle("Mapa de Calor: Interacción de Probabilidades de Mutación y Cruce\n(Menor % indica mejor rendimiento global)", fontsize=14, y=0.99)
    plt.tight_layout()
    fig.savefig(out_dir / "heatmap_operators_mut_vs_cro.png")
    plt.close(fig)

    # D.1.a y D.1.b: Heatmaps individuales de operadores para GRASP y Random-Start
    for init_type, title, suffix in [
        ("grasp", "Mapa de Calor Operadores (p_mut vs p_cro) - Inicio GRASP", "grasp"),
        ("random-start", "Mapa de Calor Operadores (p_mut vs p_cro) - Inicio Aleatorio", "randomstart")
    ]:
        sub = df_ops[df_ops["init_type"] == init_type]
        pivot = sub.pivot_table(index="p_mut", columns="p_cro", values="gap_pct", aggfunc="mean")
        pivot = pivot.reindex(index=mut_vals, columns=cro_vals)

        fig, ax = plt.subplots(figsize=(8, 6), dpi=200)
        matrix_data = pivot.values
        masked_data = np.ma.masked_invalid(matrix_data)

        cmap = plt.get_cmap("YlGnBu_r").copy()
        cmap.set_bad(color="#eeeeee")

        im = ax.imshow(masked_data, cmap=cmap, aspect="auto", origin="lower",
                       vmin=0, vmax=np.nanmax(df_ops["gap_pct"]) * 0.7)

        ax.set_xticks(range(len(cro_vals)))
        ax.set_xticklabels([f"{c:.1f}" for c in cro_vals])
        ax.set_yticks(range(len(mut_vals)))
        ax.set_yticklabels([f"{m:.1f}" for m in mut_vals])
        ax.set_xlabel("Probabilidad de Cruce (p_cro)")
        ax.set_ylabel("Probabilidad de Mutación (p_mut)")
        ax.set_title(title, fontweight="bold", pad=12)

        for r in range(len(mut_vals)):
            for c in range(len(cro_vals)):
                val = matrix_data[r, c]
                if not np.isnan(val):
                    txt_color = "white" if val > 15 else "black"
                    ax.text(c, r, f"{val:.1f}%", ha="center", va="center", color=txt_color, fontsize=9.5, fontweight="bold")
                else:
                    ax.text(c, r, "-", ha="center", va="center", color="#999999")

        cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
        cbar.set_label("Brecha Promedio (% Gap)")
        plt.tight_layout()
        fig.savefig(out_dir / f"heatmap_operators_mut_vs_cro_{suffix}.png")
        plt.close(fig)

    # D.2: Heatmap de N vs G (Escalamiento y Distribución de Presupuesto)
    df_ng = df[(df["seed"] == -1) & (np.isclose(df["p_mut"], 0.2)) & (np.isclose(df["p_cro"], 0.3))].copy()
    n_vals = sorted(df_ng["n"].unique())
    g_vals = sorted(df_ng["g"].unique())

    fig, (ax_g, ax_r) = plt.subplots(1, 2, figsize=(15, 6), dpi=200)

    for init_type, ax, title in [
        ("grasp", ax_g, "Inicio GRASP (Población N vs Generaciones G)"),
        ("random-start", ax_r, "Inicio Aleatorio (Población N vs Generaciones G)")
    ]:
        sub = df_ng[df_ng["init_type"] == init_type]
        pivot = sub.pivot_table(index="n", columns="g", values="gap_pct", aggfunc="mean")
        pivot = pivot.reindex(index=n_vals, columns=g_vals)

        matrix_data = pivot.values
        masked_data = np.ma.masked_invalid(matrix_data)

        cmap = plt.get_cmap("YlGnBu_r").copy()
        cmap.set_bad(color="#eeeeee")

        im = ax.imshow(masked_data, cmap=cmap, aspect="auto", origin="lower",
                       vmin=0, vmax=np.nanmax(df_ng["gap_pct"]) * 0.7)

        ax.set_xticks(range(len(g_vals)))
        ax.set_xticklabels([f"g={g}" for g in g_vals])
        ax.set_yticks(range(len(n_vals)))
        ax.set_yticklabels([f"n={n}" for n in n_vals])
        ax.set_xlabel("Generaciones (g - Explotación)")
        ax.set_ylabel("Población (n - Exploración)")
        ax.set_title(title, fontweight="bold", pad=10)

        for r in range(len(n_vals)):
            for c in range(len(g_vals)):
                val = matrix_data[r, c]
                if not np.isnan(val):
                    txt_color = "white" if val > 15 else "black"
                    ax.text(c, r, f"{val:.1f}%", ha="center", va="center", color=txt_color, fontsize=9, fontweight="bold")
                else:
                    ax.text(c, r, "-", ha="center", va="center", color="#999999")

        cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
        cbar.set_label("Brecha Promedio (% Gap)")

    fig.suptitle("Mapa de Calor: Configuración de Recursos (Población N vs Generaciones G)", fontsize=14, y=0.99)
    plt.tight_layout()
    fig.savefig(out_dir / "heatmap_n_vs_g_scale.png")
    plt.close(fig)

    # D.2.a y D.2.b: Heatmaps individuales de N vs G para GRASP y Random-Start
    for init_type, title, suffix in [
        ("grasp", "Mapa de Calor Recursos (Población N vs Generaciones G) - Inicio GRASP", "grasp"),
        ("random-start", "Mapa de Calor Recursos (Población N vs Generaciones G) - Inicio Aleatorio", "randomstart")
    ]:
        sub = df_ng[df_ng["init_type"] == init_type]
        pivot = sub.pivot_table(index="n", columns="g", values="gap_pct", aggfunc="mean")
        pivot = pivot.reindex(index=n_vals, columns=g_vals)

        fig, ax = plt.subplots(figsize=(8, 6), dpi=200)
        matrix_data = pivot.values
        masked_data = np.ma.masked_invalid(matrix_data)

        cmap = plt.get_cmap("YlGnBu_r").copy()
        cmap.set_bad(color="#eeeeee")

        im = ax.imshow(masked_data, cmap=cmap, aspect="auto", origin="lower",
                       vmin=0, vmax=np.nanmax(df_ng["gap_pct"]) * 0.7)

        ax.set_xticks(range(len(g_vals)))
        ax.set_xticklabels([f"g={g}" for g in g_vals])
        ax.set_yticks(range(len(n_vals)))
        ax.set_yticklabels([f"n={n}" for n in n_vals])
        ax.set_xlabel("Generaciones (g - Explotación)")
        ax.set_ylabel("Población (n - Exploración)")
        ax.set_title(title, fontweight="bold", pad=12)

        for r in range(len(n_vals)):
            for c in range(len(g_vals)):
                val = matrix_data[r, c]
                if not np.isnan(val):
                    txt_color = "white" if val > 15 else "black"
                    ax.text(c, r, f"{val:.1f}%", ha="center", va="center", color=txt_color, fontsize=9.5, fontweight="bold")
                else:
                    ax.text(c, r, "-", ha="center", va="center", color="#999999")

        cbar = fig.colorbar(im, ax=ax, fraction=0.046, pad=0.04)
        cbar.set_label("Brecha Promedio (% Gap)")
        plt.tight_layout()
        fig.savefig(out_dir / f"heatmap_n_vs_g_scale_{suffix}.png")
        plt.close(fig)

    print(f"[✓] Gráficos de Heatmaps (Caso D) completados en: {out_dir}")


def main():
    parser = argparse.ArgumentParser(
        description="Generación automatizada de gráficos para el algoritmo evolutivo TDPDP."
    )
    parser.add_argument(
        "--results-dir",
        type=str,
        default=None,
        help="Ruta al directorio de resultados (por defecto: code/data/results)"
    )
    parser.add_argument(
        "--output-dir",
        type=str,
        default=None,
        help="Ruta al directorio de gráficos (por defecto: code/data/plots)"
    )
    parser.add_argument(
        "--skip-iterations",
        action="store_true",
        help="Omite la carga y graficación de iteraciones (acelera la ejecución)"
    )
    parser.add_argument(
        "--only",
        choices=["all", "seeds", "operators", "ng", "scale", "iterations", "runtime", "boxplots", "tradeoff", "heatmaps"],
        default="all",
        help="Genera únicamente un tipo de gráfico específico (default: all)"
    )

    args = parser.parse_args()

    paths = resolve_paths()
    results_dir = Path(args.results_dir) if args.results_dir else paths["results_dir"]
    plots_dir = Path(args.output_dir) if args.output_dir else paths["plots_dir"]

    plots_dir.mkdir(parents=True, exist_ok=True)
    print(f"==================================================")
    print(f"[*] TDPDP - Generador de Gráficos y Consolidación")
    print(f"[*] Directorio de Datos:    {results_dir}")
    print(f"[*] Directorio de Gráficos: {plots_dir}")
    print(f"==================================================")

    # 1. Cargar y consolidar resultados finales
    df_results = load_and_combine_results(results_dir, save_combined=True)

    # 2. Cargar y consolidar resultados por iteración (si se requiere)
    df_iterations = None
    if args.only in ["all", "iterations"] and not args.skip_iterations:
        print("[*] Cargando archivos de iteraciones (puede demorar unos segundos)...")
        df_iterations = load_and_combine_iterations(results_dir, save_combined=True)

    # Generación de gráficos según argumento --only
    if args.only in ["all", "seeds"]:
        plot_seeds_by_instance(df_results, plots_dir)

    if args.only in ["all", "operators"]:
        plot_operator_balance(df_results, plots_dir)

    if args.only in ["all", "ng"]:
        plot_ng_differences(df_results, plots_dir)

    if args.only in ["all", "scale"]:
        plot_scale_differences(df_results, plots_dir)

    if args.only in ["all", "iterations"] and df_iterations is not None:
        plot_iterations_by_instance(df_iterations, plots_dir)

    if args.only in ["all", "runtime"]:
        plot_runtime(df_results, plots_dir)

    # Nuevos Casos B, C y D
    if args.only in ["all", "boxplots"]:
        plot_boxplots(df_results, plots_dir)

    if args.only in ["all", "tradeoff"]:
        plot_tradeoff_quality_runtime(df_results, plots_dir)

    if args.only in ["all", "heatmaps"]:
        plot_heatmaps(df_results, plots_dir)

    print(f"==================================================")
    print(f"[✓] Proceso completado exitosamente.")
    print(f"[✓] Los gráficos se han guardado de manera ordenada en: {plots_dir}")
    print(f"==================================================")


if __name__ == "__main__":
    main()
