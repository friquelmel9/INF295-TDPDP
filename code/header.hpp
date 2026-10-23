#ifndef TDPDP_HEADER
#define TDPDP_HEADER

// Librerias
#include <stdio.h>

// Definición de nombres
using Client = int;
using Time = double;
using Eval = double;

const bool TRUCK_DELIVERY = 1;
const bool DRON_DELIVERY = 0;

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
	std::vector<Path> sequence; // Secuencia de viajes. Esto indica que viajes se hicieron sin incluir el 0. Por ejemplo, sequence = [path1,path2,path3] seria decir que va de path0->path1, path1->path2, path2->path3 y path3->path0
	
	Eval eval; // Evaluación del individuo actual
	bool valid = false; // Indica si la secuencia actual es valida. Default falso hasta comprobar contrario.
};

/*
 * FUNCIONES EVOLUTIVAS
 * (Declaraciones para el algoritmo)
 * */

Eval fitness(const Individual indv, const InstanceTDPDP tdpdp); // Función que evalua el individuo del TDPDP
Individual crossing(const Individual father1, const Individual father2, const InstanceTDPDP tdpdp); // Genera un hijo de dos padres
Individual mutation(Individual &indv, const InstanceTDPDP tdpdp); // Genera una mutuación dentro del individuo irignal
void repair(Individual &indv, const InstanceTDPDP tdpdp); // Repara algún individuo

#endif
