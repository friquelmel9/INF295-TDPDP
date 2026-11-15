#!/usr/bin/env bash

set -u

# Ejecución desde la carpeta code

# Ejecución desde carpeta code (una superior)
SCRIT_DIR=".." 
INSTANCES_DIR="data/instancias"
EXEC="./exec"

# Encontrar ejecutable
if [ ! -x "$EXEC" ]; then
	if [ -f "makefile" ]; then
		make compile
	fi
	if [ ! -x "$EXEC" ]; then
		echo "[-] Error: No se pudo generar el ejecutable " >&2
		exit 1
	fi
fi

# Encontrar instancias
if [ ! -d "$INSTANCES_DIR" ]; then
	echo "[-] Error: Carpeta de instancias no encontrada"
fi


shopt -s nullglob
ALL_ENTRIES=("$INSTANCES_DIR"/*)
shopt -u nullglob

INSTANCES=("$INSTANCES_DIR"/*)
INSTANCES_FILES=()
TOTAL=0
for item in "${INSTANCES[@]}"; do
	if [ -f "$item" ]; then
		INSTANCES_FILES+=("$item")
		((TOTAL++))
	fi
done

ALL_INSTANCES=${INSTANCE_FILES[@]}
if [ "$TOTAL" -eq 0 ]; then
    echo "[-] Error. No hay instancias en '$INSTANCES_DIR'." >&2
    exit 1
fi

# Datos sobre la ejecución
SUCCESS_COUNT=0
FAIL_COUNT=0
START_ALL=$(date +%s)

INDEX=1
for file in "${INSTANCES_FILES[@]}"; do
    filename="$(basename "$file")"
    echo "[$INDEX/$TOTAL] Instancia Actual: $filename"
    
    t_start=$(date +%s)
    if "$EXEC" -i "$file" "$@"; then
        echo "    [✓] Instancia realizada"
        ((SUCCESS_COUNT++))
    else
        exit_code=$?
        echo "    [✗] Instancia fallada" >&2
        ((FAIL_COUNT++))
    fi
    ((INDEX++))
done

echo "[=] Resultados de ejecución"
echo "[=] Parámetros definidos:           ${@}"
echo "[=] Total de Instancias Ejecutadas: $SUCESS_COUNT"

if [ "$FAIL_COUNT" -gt 0 ]; then
    exit 1
fi

