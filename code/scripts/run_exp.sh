#!/usr/bin/env bash

set -u

# Si se ejecuta desde la raíz del repositorio, cambiamos a la carpeta code
if [ -d "code" ] && [ -f "code/makefile" ]; then
    cd code
fi

SCRIPT_RUN="scripts/run_all.sh"
BASE_PARAMS_DIR="data/parameters_code"

# Verificar existencia de dependencias
if [ ! -f "$SCRIPT_RUN" ]; then
    echo "[-] Error: Script '$SCRIPT_RUN' no encontrado." >&2
    exit 1
fi
if [ ! -d "$BASE_PARAMS_DIR" ]; then
    echo "[-] Error: Carpeta base de parámetros '$BASE_PARAMS_DIR' no encontrada." >&2
    exit 1
fi

# Permitir especificar un subdirectorio o archivo de parámetros como primer argumento
TARGET_PATH="$BASE_PARAMS_DIR"
EXTRA_ARGS=()

if [ "$#" -gt 0 ]; then
    if [ -f "$1" ]; then
        TARGET_PATH="$1"
        shift
        EXTRA_ARGS=("$@")
    elif [ -f "$BASE_PARAMS_DIR/$1" ]; then
        TARGET_PATH="$BASE_PARAMS_DIR/$1"
        shift
        EXTRA_ARGS=("$@")
    elif [ -d "$1" ]; then
        TARGET_PATH="$1"
        shift
        EXTRA_ARGS=("$@")
    elif [ -d "$BASE_PARAMS_DIR/$1" ]; then
        TARGET_PATH="$BASE_PARAMS_DIR/$1"
        shift
        EXTRA_ARGS=("$@")
    else
        EXTRA_ARGS=("$@")
    fi
fi

# Encontrar archivos .txt de parámetros de forma recursiva
PARAM_FILES=()
if [ -f "$TARGET_PATH" ]; then
    PARAM_FILES=("$TARGET_PATH")
elif [ -d "$TARGET_PATH" ]; then
    while IFS= read -r file; do
        PARAM_FILES+=("$file")
    done < <(find "$TARGET_PATH" -type f -name "*.txt" | sort)
fi

TOTAL_CONFIGS=${#PARAM_FILES[@]}
if [ "$TOTAL_CONFIGS" -eq 0 ]; then
    echo "[-] Error: No se encontraron archivos de parámetros (.txt) en '$TARGET_PATH'." >&2
    exit 1
fi

echo "[*] TDPDP - Ejecución de Experimentos Evolutivos"

SUCCESS_COUNT=0
FAIL_COUNT=0
CONFIG_INDEX=1

START_TIME=$(date +%s)

for param_file in "${PARAM_FILES[@]}"; do
    # Ruta relativa amigable para visualización
    rel_path="${param_file#$BASE_PARAMS_DIR/}"
    echo ""
    echo "[*][$CONFIG_INDEX/$TOTAL_CONFIGS] Configuración: $rel_path"

    # Leer parámetros del archivo (ignorando líneas vacías y comentarios)
    param_args=()
    while IFS= read -r line || [ -n "$line" ]; do
        line=$(echo "$line" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
        [[ -z "$line" || "$line" =~ ^# ]] && continue
        read -r -a words <<< "$line"
        param_args+=("${words[@]}")
    done < "$param_file"


    # Ejecutar run_all.sh con los parámetros leídos y argumentos adicionales
    if bash "$SCRIPT_RUN" "${param_args[@]}" "${EXTRA_ARGS[@]}"; then
        echo "[✓] Configuración '$rel_path' finalizada con éxito."
        ((SUCCESS_COUNT++))
    else
        echo "[✗] Configuración '$rel_path' falló durante la ejecución." >&2
        ((FAIL_COUNT++))
    fi

    ((CONFIG_INDEX++))
done

END_TIME=$(date +%s)
ELAPSED_TOTAL=$((END_TIME - START_TIME))
ELAPSED_MIN=$((ELAPSED_TOTAL / 60))
ELAPSED_SEC=$((ELAPSED_TOTAL % 60))

echo ""
echo "======================================================================"
echo "[*] RESUMEN DE EJECUCIÓN DE EXPERIMENTOS [*]"
echo "[=] Tiempo total transcurrido: ${ELAPSED_MIN}m ${ELAPSED_SEC}s (${ELAPSED_TOTAL}s)"
echo "[=] Configuraciones exitosas:  $SUCCESS_COUNT / $TOTAL_CONFIGS"
echo "[=] Configuraciones fallidas:  $FAIL_COUNT / $TOTAL_CONFIGS"
echo "======================================================================"

if [ "$FAIL_COUNT" -gt 0 ]; then
    exit 1
fi

exit 0
