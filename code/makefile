all: compile

compile: # Compilación
	g++ -std=c++17 -O2 TDPDP_Evolutivo.cpp -I. -Icode -o exec

test-wrapper: compile
	bash libs/paramILS/wrapper.sh data/instancias/Instance_010.1_new.txt "" 10 max 42 -n 200 -g 100 -mut 0.2 -cro 0.3

tune: compile # Ejecutar el tuneo
	./scripts/run_tuning.sh

run-all: compile
	bash ./scripts/run_all.sh

run-exp: compile
	bash ./scripts/run_exp.sh

clean:
	rm -rf exec
