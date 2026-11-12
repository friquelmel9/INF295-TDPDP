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
 * - n (int): Cantidad de población a utilizar en el algoritmo.
 * - g (int): Cantidad de generaciones a realizar (iteraciones del evolutivo)
 * - p_mut (double 0.0-1.0): Probabilidad de mutuación
 * - p_cro (double 0.0-1.0): Probabilidad de cruzamiento
 * */
Individual evolutionaryTdpdp(TDPDP &tdpdp, int n, int g, double p_mut, double p_cro){
	
	// Generar la población inicial 
	std::vector<Individual> generation = generatePopulation(tdpdp, n);
	for(Individual& indv : generation){greedyDrones(indv, tdpdp);}

	// Mejor individuo. Primera generación asume que es el primero.
	Individual best_indv = generation[0];

	// Distribuciones para selección aleatoria
	std::uniform_real_distribution<double> dist(0.0, 1.0); // Distribución probabilidad 0-1
	std::uniform_int_distribution<int> dist_parent(0, n - 1); // Distribución para los padres
	const int mut_types[] = {MUT_TWOOPT, MUT_SWAP, MUT_SCRAMBLE}; // Utilizada para escoger la mutación
	std::uniform_int_distribution<int> dist_mut_idx(0, 2);

	// Iterar durante las g generaciones
	for(int curr_g = 0; curr_g < g; curr_g++){

		// Evaluar cada individuo de la generación
		for(Individual& indv : generation){
			indv.fit = fitness(indv, tdpdp);
			indv.is_calculated = CALCULATED;
			if(best_indv.fit > indv.fit){best_indv = indv;}
		}

		// Nueva generación
		std::vector<Individual> next_gen(n);

		// Calculo para Elitismo (Ruleta para minimización)
		std::vector<double> roulette(n, 0.0);

		// Pesos inversamente proporcionales al fit (asumiendo fit > 0 sin números negativos)
		std::vector<double> inv_fit(n, 0.0);
		double sum_inv_fit = 0.0;
		for (int j = 0; j < n; ++j) {
			inv_fit[j] = 1.0 / generation[j].fit;
			sum_inv_fit += inv_fit[j];
		}

		roulette[0] = inv_fit[0] / sum_inv_fit;
		for (int j = 1; j < n-1; ++j) {roulette[j] = (inv_fit[j] / sum_inv_fit) + roulette[j - 1];}
		roulette[n - 1] = 1.0;

		// Calculo para probabilidad de elitismo.
		double p_sup = (double)(1.0) - p_mut - p_cro;
		std::vector<double> roulette_new = {p_mut, p_mut + p_cro, 1.0};

		// Ciclo para cada individuo
		int i = 0;
		while(i < n){
			double prob = dist(get_rng());

			if (prob < roulette_new[0]) { // MUTACIÓN
				int parent_idx = dist_parent(get_rng());
				int mut_type = mut_types[dist_mut_idx(get_rng())];
				next_gen[i] = mutation(generation[parent_idx], tdpdp, mut_type);
				greedyDrones(next_gen[i], tdpdp);
			} else if (prob < roulette_new[1]) { 
				int p1 = dist_parent(get_rng());
				int p2 = dist_parent(get_rng());
				if (n > 1) {
					while (p2 == p1) {
						p2 = dist_parent(get_rng());
					}
				}
				next_gen[i] = crossing(generation[p1], generation[p2], tdpdp);
				greedyDrones(next_gen[i], tdpdp);
			} else {
				// Elitismo (basado en el vector roulette)
				double r_elite = dist(get_rng());
				int selected_idx = 0;
				while (selected_idx < n - 1 && r_elite > roulette[selected_idx]) {
					selected_idx++;
				}
				next_gen[i] = generation[selected_idx];
			}

			i++;
		}

		// Actualizar generación para la siguiente iteración
		generation = std::move(next_gen);
	}

	// Revisar si algún individuo de la última generación supera al mejor histórico
	for(Individual& indv : generation){
		if(best_indv.fit > indv.fit){best_indv = indv;}
	}
}

int main(){
	test();
	return 0;
}
