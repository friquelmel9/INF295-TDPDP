/* Función que realiza la evaluación de la instancia actual del tpdpd
 *
 * Parámetros:
 * - indv (Individual): Instancia del problema tpdpd
 * - tdpdp (TDPDP): Datos del problema tdpdp asociado a la instancia
 *
 * Retorno:
 * - Retorna la evaluación (Time) de la instancia. No modifica el valor guardado dentro de indv
 * */
Time fitness(const Individual indv, const TDPDP tdpdp){
	
	const Client start_depot = 0;
	const Client end_depot = tdpdp.c + 1;

	// Mapear los vuelos de dron según su nodo de despegue (i)
	std::unordered_map<Client, DronFlight> launch_map;
	for (const auto& flight : indv.dron_seq) {
		launch_map[flight.i] = flight;
	}

	// Secuencia de clientes visitados por el camión (excluyendo depósitos)
	const std::vector<Client>& truck_stops = !indv.truck_seq.empty() ? indv.truck_seq : indv.tsp_seq;

	Time current_time = 0.0;
	Client current_node = start_depot;

	bool drone_in_flight = false;
	Client drone_recovery_node = NO_DRON_CLIENTS;
	Time drone_arrival_time = 0.0;

	// 1. Nodo de inicio: depósito inicial (0)
	// Si se lanza un dron desde el depósito inicial, se agrega tiempo de setup SL
	if (launch_map.find(current_node) != launch_map.end()) {
		const auto& flight = launch_map.at(current_node);
		current_time += tdpdp.SL;
		drone_in_flight = true;
		drone_recovery_node = flight.k;
		drone_arrival_time = current_time + tdpdp.tauDrone[flight.i][flight.j] + tdpdp.tauDrone[flight.j][flight.k];
	}

	// 2. Paradas intermedias del camión
	for (Client next_node : truck_stops) {
		// Tiempo del camión para trasladarse al siguiente cliente
		current_time += tdpdp.tauTruck[current_node][next_node];
		current_node = next_node;

		// Si el dron se reúne con el camión en current_node:
		// Se calcula el máximo entre la llegada del camión y la del dron, más tiempo de recuperación SR
		if (drone_in_flight && drone_recovery_node == current_node) {
			current_time = std::max(current_time, drone_arrival_time) + tdpdp.SR;
			drone_in_flight = false;
			drone_recovery_node = NO_DRON_CLIENTS;
		}

		// Si se lanza un dron desde current_node (incluyendo caso de re-lanzamiento en el mismo nodo):
		// Se agrega el tiempo de setup de lanzamiento SL
		if (!drone_in_flight && launch_map.find(current_node) != launch_map.end()) {
			const auto& flight = launch_map.at(current_node);
			current_time += tdpdp.SL;
			drone_in_flight = true;
			drone_recovery_node = flight.k;
			drone_arrival_time = current_time + tdpdp.tauDrone[flight.i][flight.j] + tdpdp.tauDrone[flight.j][flight.k];
		}
	}

	// 3. Retorno al depósito final (c + 1)
	current_time += tdpdp.tauTruck[current_node][end_depot];
	current_node = end_depot;

	// Si el dron se reúne con el camión en el depósito final:
	if (drone_in_flight && drone_recovery_node == current_node) {
		current_time = std::max(current_time, drone_arrival_time) + tdpdp.SR;
		drone_in_flight = false;
		drone_recovery_node = NO_DRON_CLIENTS;
	} else if (drone_in_flight) {
		// Resguardo para vuelos que no alcanzaron su punto de reunión antes del final
		current_time = std::max(current_time, drone_arrival_time) + tdpdp.SR;
		drone_in_flight = false;
		drone_recovery_node = NO_DRON_CLIENTS;
	}

	return current_time;
}

/* Función que realiza el cruzamiento (crossing) de dos padres
 * El cruzamiento considera unicamente los caminos (TSPSP). Luego, los drones se asignan usando las función de dpDrones o greedyDrones.
 *
 * Parámetros:
 * - father1 (Individual): Instancia del problema tdpdp. Padre 1
 * - father2 (Individual): Instancia del problema tdpdp. Padre 2
 * - tdpdp (TDPDP): Datos del problema tdpdp asociado a las instancias
 *
 * Retorno
 * - Retorna una nueva instancia del problema con el cruzamiento de ambos padres. 
 * */
Individual crossing(const Individual father1, const Individual father2, const TDPDP tdpdp){
	return father1;
}

/* Función que genera una mutuación dentro del individuo original
 *
 * Parámetros:
 * - indv (Individual): Instancia del problema tdpdp
 * - tdpdp (TDPDP): Datos del problema tdpdp asociado a las instancias
 *
 * Retorno
 * - Retorna una nueva instancia del problema con una mutuación respecto al individuo original (indv)
 * */
Individual mutation(Individual &indv, const TDPDP tdpdp){
	return indv;
}

/* Función que realiza la reparación de algún individuo
 * La reparación unicamente considera el TSP 
 * NOTA: Probablemente esta función no tenga uso en el largo plazo, pero la dejare por si acaso.
 * */
void repair(Individual &indv, const TDPDP tdpdp){

}
