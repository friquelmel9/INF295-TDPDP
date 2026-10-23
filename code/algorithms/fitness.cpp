#include <iostream>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include "header.hpp"

/*
 * Función que realiza la evaluación de un individuo
 *
 * Parámetros:
 * - indv (Individual): individuo a evaluar (su sequence no incluye depósito inicial ni final)
 * - tdpdp (InstanceTDPDP): instancia
 *
 * Retorno:
 * - Eval: tiempo total de retorno de camión y dron al depósito final (parcel)
 * */
Eval fitness(const Individual indv, const InstanceTDPDP tdpdp){
	const Client start_depot = 0;
	const Client end_depot = tdpdp.c + 1;

	// Si no hay paradas en la secuencia, el camión viaja directo del depósito inicial al final
	if (indv.sequence.empty()) {
		return tdpdp.tauTruck[start_depot][end_depot];
	}

	Time current_time = 0.0;
	Client current_truck = start_depot;
	size_t idx = 0;
	const size_t n = indv.sequence.size();

	// Si la secuencia explícitamente incluyera el depósito 0 al inicio, se salta
	if (n > 0 && indv.sequence[0].client == start_depot && indv.sequence[0].type == TRUCK_DELIVERY) {
		idx++;
	}

	while (idx < n) {
		if (indv.sequence[idx].type == TRUCK_DELIVERY) {
			// Entrega atendida por el camión
			Client next_truck = indv.sequence[idx].client;
			current_time += tdpdp.tauTruck[current_truck][next_truck];
			current_truck = next_truck;
			idx++;
		} else { // indv.sequence[idx].type == DRON_DELIVERY
			// El dron se lanza desde la posición actual del camión (i)
			Client launch_node = current_truck;
			Time launch_time = current_time;

			// Acumular tiempo de vuelo del dron a los clientes asignados
			Time drone_flight_time = 0.0;
			Client prev_drone_node = launch_node;

			while (idx < n && indv.sequence[idx].type == DRON_DELIVERY) {
				Client drone_client = indv.sequence[idx].client;
				drone_flight_time += tdpdp.tauDrone[prev_drone_node][drone_client];
				prev_drone_node = drone_client;
				idx++;
			}

			// Nodo de encuentro k (recuperación del dron):
			// Es la siguiente parada de camión en la secuencia, o el depósito final si no quedan paradas
			Client recovery_node = end_depot;
			bool recovery_in_sequence = false;

			if (idx < n && indv.sequence[idx].type == TRUCK_DELIVERY) {
				recovery_node = indv.sequence[idx].client;
				recovery_in_sequence = true;
			}

			// Tramo de vuelo del dron hasta el nodo de reencuentro k
			drone_flight_time += tdpdp.tauDrone[prev_drone_node][recovery_node];

			// Tiempos de llegada al nodo k
			Time drone_arrival = launch_time + tdpdp.SL + drone_flight_time;
			Time truck_arrival = launch_time + tdpdp.SL + tdpdp.tauTruck[launch_node][recovery_node];

			// En el nodo k se sincronizan: máximo entre llegada de camión y dron, más tiempo de recuperación SR
			current_time = std::max(drone_arrival, truck_arrival) + tdpdp.SR;
			current_truck = recovery_node;

			// Si el nodo k estaba en la secuencia como parada de camión, ya fue visitado
			if (recovery_in_sequence) {
				idx++;
			}
		}
	}

	// Si el camión aún no ha retornado al depósito final (parcel)
	if (current_truck != end_depot) {
		current_time += tdpdp.tauTruck[current_truck][end_depot];
		current_truck = end_depot;
	}

	return current_time;
}
