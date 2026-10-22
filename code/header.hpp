#ifndef TDPDP_HEADER
#define TDPDP_HEADER

// Librerias
#include <stdio.h>

// Definición de nombres
using Client = int;
using Time = double;
using Eval = double;

/* Struct de camino del dron (Definición F)
 * */
struct DronFlight{
	Client i; // Nodo inicial del dron
	Client j; // Cliente atendido por el dron
	Client k; // Nodo final del dron (encuentro con camión)
};

/* Struct indica el camino para una sequence
 * */
struct Path{
	Client client; // Nodo al que se dirigio
	bool type; // Tipo. 1 == CAMION, 0 == DRON
};

/* Struct que representa una instancia de problema TDPDP
 * Contiene la información según el modelo utilizado en el formato de la instancia
 * */
struct InstanceTDPDP {
	bool is_instance = false; // Variable auxiliar para saber si la Instancia esta inicializada.

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
		if(!is_instance){
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

/* Strucut que representa una solución a un problema TDPDP (Individuo en lo que corresponde a algoritmo evolutivo)
 * Contiene la información unica de la solución, no incluye información del tipo de problema
 * */
struct Individual {
	std::vector<Path> sequence; // Secuencia de viajes.
};

/*
 * FUNCIONES EVOLUTIVAS
 * (Declaraciones para el algoritmo)
 * */

Eval fitness(const Individual individual);
Individual crossing(const Individual father1, const Individual father2);
Individual mutation(Individual &original);
void repair(Individual &individual);

#endif
