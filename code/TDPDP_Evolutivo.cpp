#include <bits/stdc++.h>
#include "header.hpp"

// Archivos
#include "algorithms/utils.cpp"
#include "algorithms/evolutivos.cpp"
#include "algorithms/drones.cpp"

Individual evolutionaryTdpdp(TDPDP &tdpdp, int n, int g, double p_mut, double p_cro);

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
	std::vector<Individual> population = generatePopulation(tdpdp, n);
	population[0].print();

	std::cout << "================= TEST 3 ======================" << std::endl;
	greedyDrones(population[0], tdpdp);
	population[0].print();

	std::cout << "================= TEST 4 ======================" << std::endl;
	Individual evo_best = evolutionaryTdpdp(tdpdp, 10, 5, 0.2, 0.7);
	std::cout << "Evolucion completada:" << std::endl;
	evo_best.print();
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
 * - n (int): Cantidad de población a utilizar en el algoritmo.
 * - g (int): Cantidad de generaciones a realizar (iteraciones del evolutivo)
 * - p_mut (double 0.0-1.0): Probabilidad de mutuación
 * - p_cro (double 0.0-1.0): Probabilidad de cruzamiento
 * */
Individual evolutionaryTdpdp(TDPDP &tdpdp, int n, int g, double p_mut, double p_cro){
	
	// Generar la población inicial y asignar drones greedy
	std::vector<Individual> generation = generatePopulation(tdpdp, n);
	for(Individual& indv : generation){greedyDrones(indv, tdpdp);}

	// Mejor individuo histórico
	Individual best_indv = generation[0];
	for(const Individual& indv : generation){
		if(indv.fit < best_indv.fit){best_indv = indv;}
	}

	// Distribuciones para selección aleatoria
	std::uniform_real_distribution<double> dist(0.0, 1.0); // Distribución de probabilidad [0.0, 1.0)
	std::uniform_int_distribution<int> dist_parent(0, n - 1); // Muestreo uniforme de candidatos para torneo
	const int mut_types[] = {MUT_TWOOPT, MUT_SWAP, MUT_SCRAMBLE}; // Tipos de mutación disponibles
	std::discrete_distribution<int> dist_mut_weighted({0.70, 0.20, 0.10}); // Probabilidades ponderadas para operadores de mutación: 70% 2-opt, 20% swap, 10% scramble

	// Función auxiliar de selección por torneo binario (k=2) guiada por aptitud
	auto select_parent = [&](const std::vector<Individual>& pop) -> int {
		if (n <= 1) return 0;
		int c1 = dist_parent(get_rng());
		int c2 = dist_parent(get_rng());
		return (pop[c1].fit <= pop[c2].fit) ? c1 : c2;
	};

	// Buffer pre-asignado para la siguiente generación
	std::vector<Individual> next_gen(n);

	for(int curr_g = 0; curr_g < g; curr_g++){

		// Identificar al mejor individuo de la generación actual
		int best_curr_idx = 0;
		for(int j = 1; j < n; ++j){
			if(generation[j].fit < generation[best_curr_idx].fit){
				best_curr_idx = j;
			}
		}

		// Actualizar el mejor global encontrado
		if(generation[best_curr_idx].fit < best_indv.fit){
			best_indv = generation[best_curr_idx];
		}

		// Elitismo estricto: el mejor individuo pasa intacto a la siguiente generación
		next_gen[0] = generation[best_curr_idx];

		// Generar los restantes n - 1 individuos
		int i = 1;
		while(i < n){
			double prob = dist(get_rng());

			if (prob < p_mut) { // MUTACIÓN (guiada por aptitud)
				int parent_idx = select_parent(generation);
				int mut_type = mut_types[dist_mut_weighted(get_rng())];
				next_gen[i] = mutation(generation[parent_idx], tdpdp, mut_type);
				greedyDrones(next_gen[i], tdpdp);
			} else if (prob < p_mut + p_cro) { // CRUZAMIENTO (guiado por aptitud)
				int p1 = select_parent(generation);
				int p2 = select_parent(generation);
				if (n > 1) {
					while (p2 == p1) {
						p2 = dist_parent(get_rng());
					}
				}
				next_gen[i] = crossing(generation[p1], generation[p2], tdpdp);
				greedyDrones(next_gen[i], tdpdp);
			} else { // Selección / Supervivencia por torneo
				int selected_idx = select_parent(generation);
				next_gen[i] = generation[selected_idx];
			}

			i++;
		}

		// Actualizar generación para la siguiente iteración intercambiando buffers
		std::swap(generation, next_gen);
	}

	// Revisar si algún individuo de la última generación supera al mejor histórico
	for(const Individual& indv : generation){
		if(indv.fit < best_indv.fit){best_indv = indv;}
	}

	return best_indv;
}

int main(){
	test();
	return 0;
}
