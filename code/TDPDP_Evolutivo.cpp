#include <bits/stdc++.h>
#include "header.hpp"
#include "libs/CLI11.hpp"

// Archivos
#include "algorithms/utils.cpp"
#include "algorithms/evolutivos.cpp"
#include "algorithms/drones.cpp"

Individual evolutionaryTdpdp(const TDPDP &tdpdp, int n, int g, double p_mut, double p_cro, const bool print = false);

namespace fs = std::filesystem;

/* Función principal que obtiene una solución para el problema de TDPDP usando un algoritmo evolutivo.
 *
 * Parámetros:
 * - tdpdp (TDPDP): Instancia del problema a resolver.
 * - n (int): Cantidad de población a utilizar en el algoritmo.
 * - g (int): Cantidad de generaciones a realizar (iteraciones del evolutivo)
 * - p_mut (double 0.0-1.0): Probabilidad de mutuación
 * - p_cro (double 0.0-1.0): Probabilidad de cruzamiento
 * - print (bool): Imprime el estado del algoritmo por generación
 * - use_dp (bool): Utiliza programación dinámica (dpDrones) en vez de greedyDrones
 * */
Individual evolutionaryTdpdp(const TDPDP &tdpdp, int n, int g, double p_mut, double p_cro, const bool print){
	
	auto assign_drones = [&](Individual& indv){dpDrones(indv, tdpdp);};

	// Generar la población inicial y asignar drones
	std::vector<Individual> generation = generatePopulation(tdpdp, n);
	for(Individual& indv : generation){assign_drones(indv);}

	// Mejor individuo histórico
	Individual best_indv = generation[0];
	for(const Individual& indv : generation){
		if(indv.fit < best_indv.fit){best_indv = indv;}
	}

	// Distribuciones para selección aleatoria
	std::uniform_real_distribution<double> dist(0.0, 1.0); // Distribución de probabilidad [0.0, 1.0)
	std::uniform_int_distribution<int> dist_parent(0, n - 1); // Muestreo uniforme de candidatos para torneo
	const int mut_types[] = {MUT_TWOOPT, MUT_SWAP, MUT_SCRAMBLE}; // Tipos de mutación disponibles

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

		if(print){std::cout << "[%] Generación: (" << curr_g << "): Mejor evaluación actual = " << best_indv.fit << " [%] " << std::endl;}

		// Identificar al mejor y peor individuo de la generación actual para la adaptación
		int best_curr_idx = 0;
		int worst_curr_idx = 0;
		for(int j = 1; j < n; ++j){
			if(generation[j].fit < generation[best_curr_idx].fit){
				best_curr_idx = j;
			}
			if(generation[j].fit > generation[worst_curr_idx].fit){
				worst_curr_idx = j;
			}
		}

		// Actualizar el mejor global encontrado
		if(generation[best_curr_idx].fit < best_indv.fit){
			best_indv = generation[best_curr_idx];
		}

		// Rango de fitness para normalizar la cercanía del padre al óptimo actual
		double min_fit = generation[best_curr_idx].fit;
		double max_fit = generation[worst_curr_idx].fit;
		double fit_range = max_fit - min_fit;

		// Elitismo estricto: el mejor individuo pasa intacto a la siguiente generación
		next_gen[0] = generation[best_curr_idx];

		// Generar los restantes n - 1 individuos
		int i = 1;
		while(i < n){
			Prob prob = dist(get_rng());

			if (prob < p_mut) { // MUTACIÓN (adaptativa según cercanía al mejor individuo)
				int parent_idx = select_parent(generation);

				// Distancia relativa normalizada en [0.0, 1.0]: 0 = más cercano al mejor, 1 = más lejano
				double alpha = 0.0;
				if (fit_range > 1e-4) {
					alpha = (generation[parent_idx].fit - min_fit) / fit_range;
					alpha = std::clamp(alpha, 0.0, 1.0);
				}

				// Ponderación dinámica de operadores de mutación:
				Prob w_twoopt = (1.0 - alpha) * 0.80 + 0.10;
				Prob w_scramble = alpha * 0.60 + 0.05;
				Prob w_swap = 1.0 - (w_twoopt + w_scramble);
				if (w_swap < 0.05) w_swap = 0.05;

				std::discrete_distribution<int> adaptive_mut({w_twoopt, w_swap, w_scramble});
				int mut_type = mut_types[adaptive_mut(get_rng())];

				next_gen[i] = mutation(generation[parent_idx], tdpdp, mut_type);
				assign_drones(next_gen[i]);
			} else if (prob < p_mut + p_cro) { // CRUZAMIENTO (guiado por aptitud)
				int p1 = select_parent(generation);
				int p2 = select_parent(generation);
				if (n > 1) {
					while (p2 == p1) {
						p2 = dist_parent(get_rng());
					}
				}
				next_gen[i] = crossing(generation[p1], generation[p2], tdpdp);
				assign_drones(next_gen[i]);
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
		if(indv.fit < best_indv.fit){
			best_indv = indv;
		}
	}

	return best_indv;
}

int main(int argc, char** argv){

	CLI::App app{"T"};
	
	// Valores comunes para las variables
	int n=50;
	int g=50;
	Prob p_mut = 0.3;
	Prob p_cro = 0.4;
	std::string inst;
	int seed=-1;
	bool print=false;
	bool result=false;

	// Parámetrizados utilizados para usar paramILS
	app.add_option("-n,--n",n,"Tamaño de la Población (n)");
	app.add_option("-g,--g",g,"Cantidad de generaciones/iteraciones del evolutivo (g)");
	app.add_option("-m,--mut",p_mut,"Probabilidad de mutación (p_mut)");
	app.add_option("-c,--cro",p_cro,"Probabilidad de cruzamiento (p_cro)");
	app.add_option("-i,--inst",inst,"Archivo con instancias a generar");
	app.add_option("-s,--seed",seed,"Semilla a utilizar en la generación de números aleatorios");
	app.add_flag("-p,--print",print,"Imprime el estado actual del algoritmo");
	app.add_flag("-r,--result",result,"Imprime el resultado del algoritmo (para tunear usando ParamILS)");

	CLI11_PARSE(app, argc, argv);

	if(seed>=0){seed_rng(static_cast<unsigned int>(seed));}

	// Evaluación del algoritmo
	TDPDP tdpdp;
	read_file(inst, tdpdp);

	if(print){std::cout << "[+] Aplicando Algoritmo Evolutivo a Instancia " << inst << " [+] " << std::endl;}

	auto t_start = std::chrono::high_resolution_clock::now();
	Individual best = evolutionaryTdpdp(tdpdp, n, g, p_mut, p_cro, print);
	auto t_end = std::chrono::high_resolution_clock::now();
	double runtime = std::chrono::duration<double>(t_end - t_start).count();

	if(print){std::cout << "[-] Algoritmo Evolutivo Finalizado [-]" << std::endl;}
	if(print){std::cout << "[=] Resultado final: runtime = " << runtime << " (s), con resultado " << best.fit << "[=]" << std::endl;}
	if(result){std::cout << "Result for ParamILS: SAT, " << runtime << ", " << g << ", " << best.fit << ", " << seed << std::endl;}

	std::string results_file = fs::exists("data/results") ? "data/results/results.csv" : "code/data/results/results.csv";
	write_result(results_file, n, g, p_mut, p_cro, runtime, best.fit);

	return 0;
}
