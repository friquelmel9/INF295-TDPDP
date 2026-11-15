all: compile

help: # Comando de ayuda general
	

compile: # 
	g++ -std=c++17 -O2 TDPDP_Evolutivo.cpp -Icode -o exec

test-wrapper: compile
	bash libs/paramILS/wrapper.sh data/instancias/Instance_010.1_new.txt "" 10 max 42 -n 20 -g 10 -mut 0.3 -cro 0.4

tune: compile
	ruby libs/paramils2.3.8-source/param_ils_2_3_run.rb -scenariofile data/parameters/scenario.txt -numRun 1 

run-all: compile
	bash run_all.sh

clean:
	rm -rf exec
