#!/usr/bin/env bash

set -u

# Si se ejecuta desde la raíz del repositorio, cambiamos a la carpeta code
if [ -d "code" ] && [ -f "code/makefile" ]; then
    cd code
fi

SCRIPT_RUN="scripts/run_all.sh"
PARAMS_DIR="data/parameters_code"

# Verificar existencia de variables
if [ ! -f "$SCRIPT_RUN" ]; then
    echo "[-] Error: Script '$SCRIPT_RUN' no encontrado." >&2
    exit 1
fi
if [ ! -d "$PARAMS_DIR" ]; then
    echo "[-] Error: Carpeta de parámetros '$PARAMS_DIR' no encontrada." >&2
    exit 1
fi

# Encontrar archivos .txt de parámetros
shopt -s nullglob
PARAM_FILES=("$PARAMS_DIR"/*.txt)
shopt -u nullglob

TOTAL_CONFIGS=${#PARAM_FILES[@]}
if [ "$TOTAL_CONFIGS" -eq 0 ]; then
    echo "[-] Error: No se encontraron archivos de parámetros (.txt) en '$PARAMS_DIR'." >&2
    exit 1
fi

SUCCESS_COUNT=0
FAIL_COUNT=0
CONFIG_INDEX=1

START_TIME=$(date +%s)

for param_file in "${PARAM_FILES[@]}"; do
    param_filename="$(basename "$param_file")"
    echo "[*][$CONFIG_INDEX/$TOTAL_CONFIGS] Ejecutando con configuración: $param_filename [*]"

    echo "<M-C-D-Space>"

    # Leer parámetros del archivo (ignorando líneas vacías y comentarios)
    param_args=()
    while IFS= read -r line || [ -n "$line" ]; do
        line=$(echo "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        [[ -z "$line" || "$line" =~ ^# ]] && continue
        read -r -a words <<< "$line"
        param_args+=("${words[@]}")
    done < "$param_file"

    echo "[*] Con parámetros leidos: ${param_args[*]} [*]"

    # Ejecutar run_all.sh con los parámetros leídos
    if bash "$SCRIPT_RUN" "${param_args[@]}" "$@"; then
        echo "[✓] Configuración $param_filename finalizada con éxito."
        ((SUCCESS_COUNT++))
    else
        echo "[✗] Configuración $param_filename falló durante la ejecución." >&2
        ((FAIL_COUNT++))
    fi

    ((CONFIG_INDEX++))
done

END_TIME=$(date +%s)
ELAPSED_TOTAL=$((END_TIME - START_TIME))

echo "[*] FINALIZACIÓN DE EJECUCIÓN DE LOS ALGORITMOS [*]"
