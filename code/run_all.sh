#!/usr/bin/env bash
# ==============================================================================
# Script to execute TDPDP Evolutionary Algorithm for all instances
# in data/instancias/.
#
# Assumed location: inside the 'code' folder.
# ==============================================================================

set -u

# Ensure execution from the directory where this script is located (the 'code' folder)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Directory containing instance files
INSTANCES_DIR="${INSTANCES_DIR:-data/instancias}"

# Executable name
EXEC="./exec"

# Show help message if requested
if [[ "$*" == *"-h"* ]] || [[ "$*" == *"--help"* ]]; then
    echo "Usage: $0 [options]"
    echo ""
    echo "Executes TDPDP evolutionary algorithm for all files in '$INSTANCES_DIR'."
    echo "Any extra options provided are forwarded directly to the executable."
    echo ""
    echo "Examples:"
    echo "  $0                  # Run all instances with default parameters"
    echo "  $0 -p               # Run all instances with generation progress output"
    echo "  $0 -n 20 -g 10      # Custom population size and generation count"
    echo ""
    echo "Underlying executable options:"
    if [ -x "$EXEC" ]; then
        "$EXEC" --help
    else
        echo "  (Compile with 'make compile' to see executable help)"
    fi
    exit 0
fi

# Check if executable exists; attempt compilation if not present
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

# Verify instances directory exists
if [ ! -d "$INSTANCES_DIR" ]; then
    echo "[-] Error: Directory '$INSTANCES_DIR' not found." >&2
    exit 1
fi

# Collect all files inside the directory
shopt -s nullglob
ALL_ENTRIES=("$INSTANCES_DIR"/*)
shopt -u nullglob

# Filter only regular files
INSTANCE_FILES=()
for item in "${ALL_ENTRIES[@]}"; do
    if [ -f "$item" ]; then
        INSTANCE_FILES+=("$item")
    fi
done

TOTAL=${#INSTANCE_FILES[@]}

if [ "$TOTAL" -eq 0 ]; then
    echo "[-] Error: No files found inside '$INSTANCES_DIR'." >&2
    exit 1
fi

echo "============================================================"
echo "Starting TDPDP Evolutionary Algorithm execution"
echo "Folder:    $INSTANCES_DIR"
echo "Instances: $TOTAL file(s) found"
if [ "$#" -gt 0 ]; then
    echo "Arguments: $*"
fi
echo "============================================================"

SUCCESS_COUNT=0
FAIL_COUNT=0
START_ALL=$(date +%s)

INDEX=1
for file in "${INSTANCE_FILES[@]}"; do
    filename="$(basename "$file")"
    echo "[$INDEX/$TOTAL] Processing: $filename"
    
    t_start=$(date +%s)
    if "$EXEC" -i "$file" "$@"; then
        t_end=$(date +%s)
        echo "    [✓] Done in $((t_end - t_start))s"
        ((SUCCESS_COUNT++))
    else
        exit_code=$?
        t_end=$(date +%s)
        echo "    [✗] Failed with exit code $exit_code in $((t_end - t_start))s" >&2
        ((FAIL_COUNT++))
    fi
    ((INDEX++))
done

END_ALL=$(date +%s)
TOTAL_TIME=$((END_ALL - START_ALL))

echo "============================================================"
echo "Execution Summary"
echo "Total instances: $TOTAL"
echo "Successful:      $SUCCESS_COUNT"
echo "Failed:          $FAIL_COUNT"
echo "Total duration:  ${TOTAL_TIME}s"
echo "============================================================"

if [ "$FAIL_COUNT" -gt 0 ]; then
    exit 1
fi

exit 0
