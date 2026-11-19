#!/usr/bin/env python3
"""
process_tuned_results.py - Procesa y consolida resultados de ejecuciones con parámetros sintonizados.

Este script filtra los resultados de 'results_combined.csv' y 'results_per_iteration_combined.csv'
para la configuración sintonizada (n=200, g=250, p_mut=0.2, p_cro=0.3).
Calcula:
  - nombre instancia
  - tiempo promedio (s)
  - grasp/random? (tipo de inicialización)
  - % de mejora respecto a la generación inicial (eval_0 -> eval_249)
  - solucion final promedio

Guarda el archivo resultante en:
  'code/data/results/results_tuned.csv'
y genera el código LaTeX de la tabla correspondiente.
"""

import sys
from pathlib import Path
import numpy as np
import pandas as pd


def get_tuned_summary(repo_root: Path) -> pd.DataFrame:
    results_dir = repo_root / "code" / "data" / "results"
    comb_path = results_dir / "results_combined.csv"
    iter_path = results_dir / "results_per_iteration_combined.csv"

    if not comb_path.exists():
        raise FileNotFoundError(f"No se encontró: {comb_path}")
    if not iter_path.exists():
        raise FileNotFoundError(f"No se encontró: {iter_path}")

    df_comb = pd.read_csv(comb_path)
    df_iter = pd.read_csv(iter_path)

    # Filtrar por parámetros sintonizados: n=200, g=250, p_mut=0.2, p_cro=0.3
    mask_comb = (
        (df_comb["n"] == 200) &
        (df_comb["g"] == 250) &
        (np.isclose(df_comb["p_mut"], 0.2)) &
        (np.isclose(df_comb["p_cro"], 0.3))
    )
    df_comb_tuned = df_comb[mask_comb].drop_duplicates(subset=["instance", "init_type", "seed"]).copy()

    mask_iter = (
        (df_iter["n"] == 200) &
        (df_iter["g"] == 250) &
        (np.isclose(df_iter["p_mut"], 0.2)) &
        (np.isclose(df_iter["p_cro"], 0.3))
    )
    df_iter_tuned = df_iter[mask_iter].drop_duplicates(subset=["instance", "init_type", "seed", "i"]).copy()

    # Obtener evaluación inicial (i=0) y final (i=249)
    i0 = df_iter_tuned[df_iter_tuned["i"] == 0][["instance", "init_type", "seed", "eval"]].rename(
        columns={"eval": "eval_init"}
    )
    i_last = df_iter_tuned[df_iter_tuned["i"] == 249][["instance", "init_type", "seed", "eval"]].rename(
        columns={"eval": "eval_final"}
    )

    merged_iter = pd.merge(i0, i_last, on=["instance", "init_type", "seed"])
    merged_iter["mejora_pct"] = (
        (merged_iter["eval_init"] - merged_iter["eval_final"]) / merged_iter["eval_init"]
    ) * 100.0

    # Combinar con runtime de df_comb_tuned
    full = pd.merge(
        df_comb_tuned[["instance", "init_type", "seed", "runtime", "eval"]],
        merged_iter[["instance", "init_type", "seed", "eval_init", "mejora_pct"]],
        on=["instance", "init_type", "seed"]
    )

    # Agrupar por instancia e init_type promediando sobre las semillas
    summary = full.groupby(["instance", "init_type"]).agg(
        tiempo_promedio=("runtime", "mean"),
        porcentaje_mejora=("mejora_pct", "mean"),
        solucion_final=("eval", "mean")
    ).reset_index()

    # Renombrar columnas para consistencia con el requerimiento del usuario
    summary = summary.rename(columns={
        "instance": "nombre_instancia",
        "init_type": "grasp_random"
    })

    # Reordenar columnas exactamente como solicitó el usuario:
    # nombre instancia ; tiempo promedio ; grasp/random? ; % de mejora ; solucion final
    summary = summary[["nombre_instancia", "tiempo_promedio", "grasp_random", "porcentaje_mejora", "solucion_final"]]

    # Ordenar por clientes c y variante de instancia
    summary["c"] = summary["nombre_instancia"].str.extract(r"Instance_(\d+)").astype(int)
    summary["var"] = summary["nombre_instancia"].str.extract(r"Instance_\d+\.(\d+)").astype(int)
    summary["init_order"] = summary["grasp_random"].map({"grasp": 0, "random-start": 1})
    summary = summary.sort_values(by=["c", "var", "init_order"]).drop(columns=["c", "var", "init_order"])


    return summary


def generate_latex_table(df: pd.DataFrame) -> str:
    lines = []
    lines.append(r"\begin{table}[htbp]")
    lines.append(r"\centering")
    lines.append(r"\footnotesize")
    lines.append(r"\setlength{\tabcolsep}{4.5pt}")
    lines.append(r"\caption{Resultados obtenidos con parámetros sintonizados ($n=200, g=250, p_{mut}=0.2, p_{cro}=0.3$) promediados sobre las semillas.}")
    lines.append(r"\label{tab:resultados_tuneados}")
    lines.append(r"\begin{tabular}{lrrrrrr}")
    lines.append(r"\toprule")
    lines.append(r"\multirow{2}{*}{\textbf{Instancia}} & \multicolumn{2}{c}{\textbf{Tiempo Promedio (s)}} & \multicolumn{2}{c}{\textbf{\% de Mejora}} & \multicolumn{2}{c}{\textbf{Solución Final}} \\")
    lines.append(r"\cmidrule(lr){2-3} \cmidrule(lr){4-5} \cmidrule(lr){6-7}")
    lines.append(r" & \textbf{Random-Start} & \textbf{GRASP} & \textbf{Random-Start} & \textbf{GRASP} & \textbf{Random-Start} & \textbf{GRASP} \\")
    lines.append(r"\midrule")

    instances = df["nombre_instancia"].unique()
    for idx_inst, inst in enumerate(instances):
        sub = df[df["nombre_instancia"] == inst]
        clean_name = inst.replace("_new.txt", "").replace("_", r"\_")

        row_rnd = sub[sub["grasp_random"] == "random-start"]
        row_grasp = sub[sub["grasp_random"] == "grasp"]

        t_r = row_rnd["tiempo_promedio"].values[0] if not row_rnd.empty else 0.0
        t_g = row_grasp["tiempo_promedio"].values[0] if not row_grasp.empty else 0.0

        m_r = row_rnd["porcentaje_mejora"].values[0] if not row_rnd.empty else 0.0
        m_g = row_grasp["porcentaje_mejora"].values[0] if not row_grasp.empty else 0.0

        s_r = row_rnd["solucion_final"].values[0] if not row_rnd.empty else 0.0
        s_g = row_grasp["solucion_final"].values[0] if not row_grasp.empty else 0.0

        lines.append(
            rf"{clean_name} & {t_r:.2f} & {t_g:.2f} & {m_r:.2f}\% & {m_g:.2f}\% & {s_r:.2f} & {s_g:.2f} \\"
        )

        if idx_inst < len(instances) - 1:
            if (idx_inst + 1) % 5 == 0:
                lines.append(r"\midrule")
            else:
                lines.append(r"\addlinespace[1.5pt]")

    lines.append(r"\bottomrule")
    lines.append(r"\end{tabular}")
    lines.append(r"\end{table}")

    return "\n".join(lines)




def main():
    repo_root = Path(__file__).resolve().parents[2]
    out_csv = repo_root / "code" / "data" / "results" / "results_tuned.csv"

    print(f"[*] Procesando resultados sintonizados desde {repo_root}...")
    df_summary = get_tuned_summary(repo_root)

    # Redondear valores para el CSV
    df_to_save = df_summary.copy()
    df_to_save["tiempo_promedio"] = df_to_save["tiempo_promedio"].round(4)
    df_to_save["porcentaje_mejora"] = df_to_save["porcentaje_mejora"].round(2)
    df_to_save["solucion_final"] = df_to_save["solucion_final"].round(2)

    df_to_save.to_csv(out_csv, index=False)
    print(f"[✓] Archivo CSV generado exitosamente: {out_csv}")
    print(f"    Filas: {len(df_to_save)}")

    latex_code = generate_latex_table(df_summary)
    print("\n--- Código LaTeX generado ---")
    print(latex_code[:300] + "\n...\n" + latex_code[-200:])


if __name__ == "__main__":
    main()
