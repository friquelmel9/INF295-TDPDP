#!/usr/bin/env bash
# ==============================================================================
# ParamILS Target Algorithm Wrapper for TDPDP Evolutionary Algorithm
#
# Arguments passed by ParamILS:
#   $1: instance_relname   (path to instance file)
#   $2: instance_specifics (extra instance info or "")
#   $3: cutoff_time        (time limit in seconds)
#   $4: cutoff_length      (step/iteration limit or max)
#   $5: seed               (random seed integer)
#   $6...: algorithmic parameters (e.g. -n 50 -g 50 -mut 0.3 -cro 0.4)
# ==============================================================================

set -u

# Check minimum number of arguments
if [ "$#" -lt 5 ]; then
    echo "Usage: $0 <instance_file> <instance_specifics> <cutoff_time> <cutoff_length> <seed> [parameters...]" >&2
    exit 1
fi

INSTANCE_IN="$1"
SPECIFICS="$2"
CUTOFF_TIME="$3"
CUTOFF_LENGTH="$4"
SEED="$5"
shift 5

# Determine directory paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CODE_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
ROOT_DIR="$(cd "$CODE_DIR/.." && pwd)"

# Locate executable
EXEC="$CODE_DIR/exec"
if [ ! -x "$EXEC" ]; then
    # Try building if not present
    if [ -f "$CODE_DIR/makefile" ]; then
        make -C "$CODE_DIR" compile > /dev/null 2>&1
    fi
fi

if [ ! -x "$EXEC" ]; then
    echo "Result for ParamILS: CRASHED, 0.0, 0, 999999999, $SEED"
    echo "Error: Executable not found or not executable at $EXEC" >&2
    exit 0
fi

# Resolve instance file path
INSTANCE=""
if [ -f "$INSTANCE_IN" ]; then
    INSTANCE="$INSTANCE_IN"
elif [ -f "$ROOT_DIR/$INSTANCE_IN" ]; then
    INSTANCE="$ROOT_DIR/$INSTANCE_IN"
elif [ -f "$CODE_DIR/$INSTANCE_IN" ]; then
    INSTANCE="$CODE_DIR/$INSTANCE_IN"
elif [ -f "$CODE_DIR/data/instancias/$(basename "$INSTANCE_IN")" ]; then
    INSTANCE="$CODE_DIR/data/instancias/$(basename "$INSTANCE_IN")"
elif [ -f "$ROOT_DIR/code/data/instancias/$(basename "$INSTANCE_IN")" ]; then
    INSTANCE="$ROOT_DIR/code/data/instancias/$(basename "$INSTANCE_IN")"
else
    echo "Result for ParamILS: CRASHED, 0.0, 0, 999999999, $SEED"
    echo "Error: Instance file not found: $INSTANCE_IN" >&2
    exit 0
fi

# Convert parameters into CLI11-friendly arguments
# Maps -mut -> --mut and -cro -> --cro so CLI11 parses them properly
CLI_ARGS=()
while [ "$#" -gt 0 ]; do
    case "$1" in
        -n|--n)
            CLI_ARGS+=("-n" "$2")
            shift 2
            ;;
        -g|--g)
            CLI_ARGS+=("-g" "$2")
            shift 2
            ;;
        -mut|--mut|-m)
            CLI_ARGS+=("--mut" "$2")
            shift 2
            ;;
        -cro|--cro|-c)
            CLI_ARGS+=("--cro" "$2")
            shift 2
            ;;
        *)
            if [[ "$1" =~ ^-[a-zA-Z0-9_]{2,}$ ]]; then
                CLI_ARGS+=("-$1")
            else
                CLI_ARGS+=("$1")
            fi
            if [ "$#" -gt 1 ] && [[ ! "$2" =~ ^- ]]; then
                CLI_ARGS+=("$2")
                shift 2
            else
                shift 1
            fi
            ;;
    esac
done

# Timeout handling
TIMEOUT_SEC="${CUTOFF_TIME:-30}"
# If CUTOFF_TIME is "max" or non-numeric, default to 3600
if ! [[ "$TIMEOUT_SEC" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    TIMEOUT_SEC=3600
fi

# Run executable with timeout
# Seed is passed via -s, instance via -i, and -r requests ParamILS output format
CMD=("$EXEC" "-i" "$INSTANCE" "-s" "$SEED" "-r" "${CLI_ARGS[@]}")

START_TIME=$(date +%s%N 2>/dev/null || date +%s)
OUTPUT=$(timeout "$TIMEOUT_SEC" "${CMD[@]}" 2>&1)
EXIT_CODE=$?
END_TIME=$(date +%s%N 2>/dev/null || date +%s)

# Calculate elapsed time in seconds as fallback
if [ "$START_TIME" -ne "$END_TIME" ] && [ "${#START_TIME}" -ge 10 ]; then
    ELAPSED=$(awk -v s="$START_TIME" -v e="$END_TIME" 'BEGIN { printf "%.4f", (e - s) / 1000000000 }')
else
    ELAPSED="0.0"
fi

# Check execution status
if [ "$EXIT_CODE" -eq 124 ]; then
    # Timeout occurred
    echo "Result for ParamILS: TIMEOUT, $TIMEOUT_SEC, 0, 999999999, $SEED"
    exit 0
elif [ "$EXIT_CODE" -ne 0 ]; then
    # Execution crashed
    echo "Result for ParamILS: CRASHED, $ELAPSED, 0, 999999999, $SEED"
    echo "$OUTPUT" >&2
    exit 0
fi

# Search for the standard ParamILS output in the algorithm response
RESULT_LINE=$(echo "$OUTPUT" | grep -m 1 "Result for ParamILS:")

if [ -n "$RESULT_LINE" ]; then
    echo "$RESULT_LINE"
else
    # Fallback if binary printed just a numeric value
    FITNESS=$(echo "$OUTPUT" | tail -n 1 | tr -d '\r\n')
    if [[ "$FITNESS" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        echo "Result for ParamILS: SAT, $ELAPSED, 0, $FITNESS, $SEED"
    else
        echo "Result for ParamILS: CRASHED, $ELAPSED, 0, 999999999, $SEED"
        echo "$OUTPUT" >&2
    fi
fi

exit 0
