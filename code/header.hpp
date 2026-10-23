#ifndef TDPDP_HEADER
#define TDPDP_HEADER

// Librerias
#include <stdio.h>

// Definición de nombres
using Client = int;
using Time = double;
using Eval = double;

// Constantes para camión
const bool TRUCK_DELIVERY = 1;
const bool DRON_DELIVERY = 0;

// Constantes para evaluación
const bool TO_CALC = 0;
const bool CALCULATED = 1;

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
		std::cout << "                 Parámetros                   " << std::endl;
		std::cout << "==============================================" << std::endl;
		std::cout << "c (#Clients): " << c << std::endl;
		std::cout << "SL (Time Setup Launch): " << SL << std::endl;
		std::cout << "SR (Time Setup Recover): " << SR << std::endl;
		std::cout << "E (Endurance): " << E << std::endl;
		std::cout << "==============================================" << std::endl;
		std::cout << "                   Cdrones                    " << std::endl;
		std::cout << "==============================================" << std::endl;
		for (Client client_cd : Cdrones){std::cout << client_cd << " ";}
		std::cout << std::endl;
		}
};


/* Struct que representa una solución del problema TSP (Algoritmo Evolutivo)
 * */
struct Individual {
	std::vector<Client> sequence; // Secuencia de clientes. Se asume que el vector no incluye el depot 0 y c+1, si no que incluye unicamente los c clientes.
	std::vector<DronFlight> dronFlights; // Secuencia de drones utilizadas en el TSP.
	
	Eval fit = 0; // Valor de evaluación a la secuencia actual
	bool is_calculated = TO_CALC; // Indica si se ha hecho la evaluación del fitness()
};

/* Conjunto de declaración de funciones para la implementación evolutiva
 * */

Eval fitness(const Individual indv, const TDPDP tdpdp); // Función que evalua el individuo del TDPDP
Individual crossing(const Individual father1, const Individual father2, const TDPDP tdpdp); // Genera un hijo de dos padres
Individual mutation(Individual &indv, const TDPDP tdpdp); // Genera una mutuación dentro del individuo irignal
void repair(Individual &indv, const TDPDP tdpdp); // Repara algún individuo

#endif
