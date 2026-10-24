#ifndef TDPDP_HEADER
#define TDPDP_HEADER

// Librerias
#include <stdio.h>

// Definición de nombres
using Client = int;
using Time = double;

// Constantes para camión
const bool TRUCK_DELIVERY = 1;
const bool DRON_DELIVERY = 0;

// Constantes para evaluación
const bool TO_CALC = 0;
const bool CALCULATED = 1;

// Constantes para dron
const int NO_DRON_CLIENTS = -1; // Usar para indicar que no hay más entregas de dron con las que revisar el tiempo máximo de recuperación

// Constantes para mutación
const int MUT_TWOOPT = 1;
const int MUT_SWAP = 2;
const int MUT_SCRAMBLE = 3;

/* Struct de camino del dron (Definición F)
 * */
struct DronFlight {
	Client i; // Nodo inicial del dron
	Client j; // Cliente atendido por el dron
	Client k; // Nodo final del dron (encuentro con camión)
};

/* Struct que representa una instancia de problema TDPDP
 * Contiene la información según el modelo utilizado en el formato de la instancia
 * */
struct TDPDP {
	bool isInstance = false; // Variable auxiliar para saber si la Instancia esta inicializada.

	Client c; // Cantidad de clientes
	Time SL; // Setup Launch Time
	Time SR; // Setup Recover Time
	Time E; // Tiempo de resistencia del Dron (Endurance)
	
	std::unordered_set<Client> Cdrones; // Conjunto de clientes atendibles por el dron
	
	std::vector<std::vector<Time>> tauTruck; // Tiempo que toma al camión para ir del nodo i al nodo j (tauTruck[i][j])
	std::vector<std::vector<Time>> tauDrone; // Tiempo que toma al dron para ir del nodo i alnodo j (tauTruck[i][j])
	
	int size_F; // Tamaño del conjunto F
	std::vector<DronFlight> F; // Conjunto de los posibles caminos para un dron
	
	/*
	 * Función auxiliar que imprime los datos de la instancia
	 * */
	void print(){
		if(!isInstance){
			std::cout << "Instancia no cargada" << std::endl;
			return;
		}
		std::cout << "==============================================" << std::endl;
		std::cout << "                   TDPDP                      " << std::endl;
		std::cout << "==============================================" << std::endl;
		std::cout << "                 Parámetros                   " << std::endl;
		std::cout << "c (#Clients): " << c << std::endl;
		std::cout << "SL (Time Setup Launch): " << SL << std::endl;
		std::cout << "SR (Time Setup Recover): " << SR << std::endl;
		std::cout << "E (Endurance): " << E << std::endl;
		std::cout << "                   Cdrones                    " << std::endl;
		for (Client client_cd : Cdrones){std::cout << client_cd << " ";}
		std::cout << std::endl;
		}
};


/* Struct que representa una solución del problema TDPDP (Algoritmo Evolutivo)
 * */
struct Individual {
	std::vector<Client> tsp_seq; // Secuencia de clientes del TSP del camión. Incluye todos los caminos, exceptuando el depot 0 y c+1, es decir, se asumen que ocurren pero no se encuentran insertos dentro de la secuencia.
	
	std::vector<Client> truck_seq; // Secuencia de clientes del TDPDP (tras usar greedyDrones o dpDrones). Incluye todos los caminos realizados por el camión, exceptuando el depot 0 y c+1, es decir, asumen que ocurren pero no se encuentran insertos dentro del vector.
	std::vector<DronFlight> dron_seq; // Secuencia de vuelos del TDPDP. Utilizado para revisar
	std::set<Client> dron_launch; // Set auxiliar en donde estan aquellos clientes desde donde el camión lanza el dron. Sirve par amantener un calculo mejor de las rutas.
	
	Time fit = 0; // Valor de evaluación a la secuencia actual
	bool is_calculated = TO_CALC; // Indica si se ha hecho la evaluación del fitness()

	/*
	 * Función auxiliar que imprime los datos del individuo
	 * */
	void print() const {
		std::cout << "==============================================" << std::endl;
		std::cout << "                  Individuo                   " << std::endl;
		std::cout << "==============================================" << std::endl;
		std::cout << "tsp_seq: ";
		for (Client c : tsp_seq) {
			std::cout << c << " ";
		}
		std::cout << std::endl;

		std::cout << "truck_seq: ";
		for (Client c : truck_seq) {
			std::cout << c << " ";
		}
		std::cout << std::endl;

		std::cout << "dron_seq: ";
		for (size_t idx = 0; idx < dron_seq.size(); ++idx) {
			std::cout << "{" << dron_seq[idx].i << ", " << dron_seq[idx].j << ", " << dron_seq[idx].k << "}";
			if (idx + 1 < dron_seq.size()) {
				std::cout << ", ";
			}
		}
		std::cout << std::endl;

		if (is_calculated == CALCULATED) {
			std::cout << "fit = " << fit << std::endl;
		} else {
			std::cout << "fit = (not calculated)" << std::endl;
		}
		std::cout << "==============================================" << std::endl;
	}
};

/* Conjunto de declaración de funciones para la implementación evolutiva
 * */

Time fitness(const Individual indv, const TDPDP tdpdp); // Función que evalua el individuo del TDPDP
Individual crossing(const Individual father1, const Individual father2, const TDPDP tdpdp); // Genera un hijo de dos padres
Individual mutation(Individual &indv, const TDPDP tdpdp, const int type); // Genera una mutuación dentro del individuo original
void repair(Individual &indv, const TDPDP tdpdp); // Repara algún individuo
std::vector<Individual> generatePopulation(const TDPDP tdpdp, const int n); // Genera la población inicial

/* Conjunto de declaración de funciones para la obtención optima del dron.
 * */

void dpDrones(Individual &indv, const TDPDP tdpdp); // Genera mediante un dp la asignación de los drones 
void greedyDrones(Individual &ind, const TDPDP tdpdp); // Genera mediante un greedy la asignación de los drones						

/* Utilidades 
 * */

void read_file(std::string filepath, TDPDP &inst); // Lectura de un archivo instancia

#endif
