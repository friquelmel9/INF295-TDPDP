#!/usr/bin/env bash

set -u

# Ejecución desde la carpeta code
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

INSTANCES_DIR="${INSTANCES_DIR:-data/instancias}"
EXEC="./exec"

# Mensaje de ayuda (Gemini)
if [[ "$*" == *"-h"* ]] || [[ "$*" == *"--help"* ]]; then
    echo "Uso: $0 [options]"
    echo ""
    echo "Realiza todas las instancias guardadas en:  '$INSTANCES_DIR'."
    echo "Opciones extras son dadas por el ejecutable."
    echo ""
    echo "Ejemplos"
    echo "  $0                  # Corre todas las instancias con los parametros comunes"
    echo "  $0 -p               # Corre las instancias printeando su desarrollo"
    echo "  $0 -n 20 -g 10      # Corre las instancias cambiando el n y el g"
    echo ""
fi

# Ejecutar ejecutable
if [ ! -x "$EXEC" ]; then
    echo "[!] Executable '$EXEC' not found. Attempting compilation..."
    if [ -f "makefile" ]; then
        make compile
    else
        g++ -std=c++17 -O2 TDPDP_Evolutivo.cpp -Icode -o exec
    fi
    if [ ! -x "$EXEC" ]; then
        echo "[-] Error: Could not build or find '$EXEC'." >&2
        exit 1
    fi
    echo "[+] Compilation successful."
fi

# Verificación de instancias
if [ ! -d "$INSTANCES_DIR" ]; then
    echo "[-] Error: Directory '$INSTANCES_DIR' not found." >&2
    exit 1
fi

shopt -s nullglob
ALL_ENTRIES=("$INSTANCES_DIR"/*)
shopt -u nullglob

INSTANCE_FILES=()
for item in "${ALL_ENTRIES[@]}"; do
    if [ -f "$item" ]; then
        INSTANCE_FILES+=("$item")
    fi
done

TOTAL=${#INSTANCE_FILES[@]}

if [ "$TOTAL" -eq 0 ]; then
    echo "[-] Error. No hay instancias en '$INSTANCES_DIR'." >&2
    exit 1
fi

echo "============================================================"
echo "Iniciando ejecución de `TDPDP_Evolutivo.cpp`"
echo "============================================================"

SUCCESS_COUNT=0
FAIL_COUNT=0
START_ALL=$(date +%s)

INDEX=1
for file in "${INSTANCE_FILES[@]}"; do
    filename="$(basename "$file")"
    echo "[$INDEX/$TOTAL] Instancia Actual: $filename"
    
    t_start=$(date +%s)
    if "$EXEC" -i "$file" "$@"; then
        t_end=$(date +%s)
        echo "    [✓] Instancia realizada en $((t_end - t_start))s"
        ((SUCCESS_COUNT++))
    else
        exit_code=$?
        t_end=$(date +%s)
        echo "    [✗] Instancia fallada $exit_code in $((t_end - t_start))s" >&2
        ((FAIL_COUNT++))
    fi
    ((INDEX++))
done

END_ALL=$(date +%s)
TOTAL_TIME=$((END_ALL - START_ALL))

echo "============================================================"
echo "Resumen de ejecución"
echo "Tolta Instancias: $TOTAL"
echo "Ejecutadas:       $SUCCESS_COUNT"
echo "Fallos:           $FAIL_COUNT"
echo "Duración Final:   ${TOTAL_TIME}s"
echo "============================================================"

if [ "$FAIL_COUNT" -gt 0 ]; then
    exit 1
fi

exit 0
