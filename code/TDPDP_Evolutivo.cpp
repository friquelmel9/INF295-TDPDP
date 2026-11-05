#include <bits/stdc++.h>
#include "header.hpp"

// Archivos
#include "algorithms/utils.cpp"
#include "algorithms/evolutivos.cpp"
#include "algorithms/drones.cpp"

/*
 * Función de testing
 * */
void test(){
	std::cout << "================= TEST 1 ======================" << std::endl;


	std::string filepath = "data/instancias/Instance_010.1_new.txt";
	TDPDP tdpdp; 
	read_file(filepath, tdpdp);
	tdpdp.print();

	std::cout << "================= TEST 2 ======================" << std::endl;

	int n=1;
	std::vector<Individual> population = generatePopulation(tdpdp, 1);
	population[0].print();

	std::cout << "================= TEST 3 ======================" << std::endl;
	greedyDrones(population[0], tdpdp);
	population[0].print();
}

namespace fs = std::filesystem;

/* Función que procesa los datos de problemas TDPDP
 * */
void processData(const std::string dirpath){
	if (!fs::exists(dirpath) || !fs::is_directory(dirpath)) {
		std::cerr << "Error: Invalid directory path.\n";
		return;
	}

	for (const auto& entry : fs::directory_iterator(dirpath)) {
		if (fs::is_regular_file(entry.status())) {
			std::cout << "Processing file: " << entry.path() << '\n';

			std::string filepath = entry.path().string();
			
			// Llamada de los datos
			TDPDP instTdpdp;
			read_file(filepath, instTdpdp);
		} 
	}
}

/* Función principal que obtiene una solución para el problema de TDPDP usando un algoritmo evolutivo.
 *
 * Parámetros:
 * - tdpdp (TDPDP): Instancia del problema a resolver.
 * - n: Cantidad de población a utilizar en el algoritmo.
 * */
void evolutionaryTdpdp(TDPDP &tdpdp, int n){
	
}

int main(){
	test();
	return 0;
}
