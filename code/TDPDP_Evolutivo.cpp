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
	// TEST 1: Lectura de un archivo
	std::cout << "================= TEST 1 ======================" << std::endl;


	std::string filepath = "data/instancias/Instance_010.1_new.txt";
	TDPDP tdpdp; 
	read_file(filepath, tdpdp);
	tdpdp.print();

	// TEST 2: Solución inicial generada de manera aleatoria
	std::cout << "================= TEST 2 ======================" << std::endl;

	int n=1;
	std::vector<Individual> population = generatePopulation(tdpdp, 1);
	population[0].print();

	// TEST 3: Evaluación de una solución inicial
	std::cout << "================= TEST 3 ======================" << std::endl;

	population[0].fit = fitness(population[0], tdpdp);
	population[0].is_calculated = true;
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

	// Iterate through the folder contents one by one
	for (const auto& entry : fs::directory_iterator(dirpath)) {
		// entry.path() returns a fs::path object
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
