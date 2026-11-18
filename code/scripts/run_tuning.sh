PARAMILS_EXEC="libs/paramils2.3.8-source/param_ils_2_3_run.rb"
PARAMILS_SCENARIO="data/parameters/scenario.txt"
NUM_RUNS=1

# Existencia de parametros
if [ ! -f "$PARAMILS_EXEC" ]; then
	echo "[-] Error: Ejecutable de paramILS no encontrado"
	exit -1
fi
if [ ! -f "$PARAMILS_SCENARIO" ]; then
	echo "[-] Error: Scenario para el tuning no encontrado"
	exit -1
fi

echo "[+] Inicio del tuning"
ruby "${PARAMILS_EXEC}" -scenariofile "${PARAMILS_SCENARIO}" -numRun 1 

