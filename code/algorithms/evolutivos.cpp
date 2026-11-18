/* Función que realiza la evaluación de la instancia actual del tpdpd
 *
 * Parámetros:
 * - indv (Individual): Instancia del problema tpdpd
 * - tdpdp (TDPDP): Datos del problema tdpdp asociado a la instancia
 *
 * Retorno:
 * - Retorna la evaluación (Time) de la instancia. No modifica el valor guardado dentro de indv
 * */
Time fitness(const Individual &indv, const TDPDP &tdpdp){
	
	const Client start_depot = 0;
	const Client end_depot = tdpdp.c + 1;

	// Mapear los vuelos de dron según su nodo de despegue (i)
	std::unordered_map<Client, DronFlight> launch_map;
	for (const auto& flight : indv.dron_seq) {
		launch_map[flight.i] = flight;
	}

	// Secuencia de clientes visitados por el camión (excluyendo depósitos)
	std::vector<Client> truck_stops;
	if (!indv.seq.empty()) {
		truck_stops.reserve(indv.tsp_seq.size());
		for (size_t p = 0; p < indv.tsp_seq.size(); ++p) {
			if (indv.seq[p] == TRUCK_DELIVERY) {
				truck_stops.push_back(indv.tsp_seq[p]);
			}
		}
	}
	const std::vector<Client>& actual_truck_stops = !indv.seq.empty() ? truck_stops : indv.tsp_seq;

	// Calculo del tiempo en
	
	Time current_time = 0.0;
	Client current_node = start_depot;

	Time penalty = 0.0;
	Time drone_departure_time = 0.0;
	DronFlight active_flight;

	bool drone_in_flight = false;
	Client drone_recovery_node = NO_DRON_CLIENTS;
	Time drone_arrival_time = 0.0;

	// 1. Nodo de inicio: depósito inicial (0)
	// Si se lanza un dron desde el depósito inicial, se agrega tiempo de setup SL
	if (launch_map.find(current_node) != launch_map.end()) {
		active_flight = launch_map.at(current_node);
		if (!tdpdp.is_valid_flight(active_flight.i, active_flight.j, active_flight.k)) {
			penalty += 1e5;
		}
		current_time += tdpdp.SL;
		drone_departure_time = current_time;
		drone_in_flight = true;
		drone_recovery_node = active_flight.k;
		drone_arrival_time = current_time + tdpdp.tauDrone[active_flight.i][active_flight.j] + tdpdp.tauDrone[active_flight.j][active_flight.k];
	}

	// 2. Paradas intermedias del camión
	for (Client next_node : actual_truck_stops) {
		// Tiempo del camión para trasladarse al siguiente cliente
		current_time += tdpdp.tauTruck[current_node][next_node];
		current_node = next_node;

		// Si el dron se reúne con el camión en current_node:
		// Se calcula el máximo entre la llegada del camión y la del dron, más tiempo de recuperación SR
		if (drone_in_flight && drone_recovery_node == current_node) {
			Time truck_arrival_time = current_time;
			Time sortie_time = std::max(truck_arrival_time, drone_arrival_time) - drone_departure_time;
			if (sortie_time > tdpdp.E + 1e-6) {
				penalty += 1e5 + 1000.0 * (sortie_time - tdpdp.E);
			}
			current_time = std::max(truck_arrival_time, drone_arrival_time) + tdpdp.SR;
			drone_in_flight = false;
			drone_recovery_node = NO_DRON_CLIENTS;
		}

		// Si se lanza un dron desde current_node (incluyendo caso de re-lanzamiento en el mismo nodo):
		// Se agrega el tiempo de setup de lanzamiento SL
		if (!drone_in_flight && launch_map.find(current_node) != launch_map.end()) {
			active_flight = launch_map.at(current_node);
			if (!tdpdp.is_valid_flight(active_flight.i, active_flight.j, active_flight.k)) {
				penalty += 1e5;
			}
			current_time += tdpdp.SL;
			drone_departure_time = current_time;
			drone_in_flight = true;
			drone_recovery_node = active_flight.k;
			drone_arrival_time = current_time + tdpdp.tauDrone[active_flight.i][active_flight.j] + tdpdp.tauDrone[active_flight.j][active_flight.k];
		}
	}

	// 3. Retorno al depósito final (c + 1)
	current_time += tdpdp.tauTruck[current_node][end_depot];
	current_node = end_depot;

	// Si el dron se reúne con el camión en el depósito final:
	if (drone_in_flight && drone_recovery_node == current_node) {
		Time truck_arrival_time = current_time;
		Time sortie_time = std::max(truck_arrival_time, drone_arrival_time) - drone_departure_time;
		if (sortie_time > tdpdp.E + 1e-6) {
			penalty += 1e5 + 1000.0 * (sortie_time - tdpdp.E);
		}
		current_time = std::max(truck_arrival_time, drone_arrival_time) + tdpdp.SR;
		drone_in_flight = false;
		drone_recovery_node = NO_DRON_CLIENTS;
	} else if (drone_in_flight) {
		// Resguardo para vuelos que no alcanzaron su punto de reunión antes del final
		penalty += 1e6;
		current_time = std::max(current_time, drone_arrival_time) + tdpdp.SR;
		drone_in_flight = false;
		drone_recovery_node = NO_DRON_CLIENTS;
	}

	return current_time + penalty;
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
Individual crossing(const Individual &father1, const Individual &father2, const TDPDP &tdpdp){
	Individual child;
	const int n = static_cast<int>(father1.tsp_seq.size());

	// Casos base o secuencias vacías / unarias
	if (n == 0) {
		return child;
	}
	if (n == 1 || father2.tsp_seq.size() != static_cast<size_t>(n)) {
		child.tsp_seq = father1.tsp_seq;
		return child;
	}

	// Selección de puntos de corte para la subsecuencia continua del Padre 1
	std::uniform_int_distribution<int> dist(0, n - 1);
	int cut1 = dist(get_rng());
	int cut2 = dist(get_rng());
	if (cut1 > cut2) {
		std::swap(cut1, cut2);
	}

	// Asegurar que la subsecuencia no abarque toda la secuencia si n > 1 para permitir aporte del Padre 2
	if (n > 2) {
		while (cut1 == cut2 || (cut1 == 0 && cut2 == n - 1)) {
			cut1 = dist(get_rng());
			cut2 = dist(get_rng());
			if (cut1 > cut2) {
				std::swap(cut1, cut2);
			}
		}
	} else if (n == 2) {
		cut1 = dist(get_rng());
		cut2 = cut1;
	}

	// Inicializar la secuencia del hijo con valores no válidos (-1)
	child.tsp_seq.assign(n, -1);
	std::unordered_set<Client> in_child;
	in_child.reserve(n);

	// 1. Copiar subsecuencia continua del Padre 1 en las mismas posiciones
	for (int i = cut1; i <= cut2; ++i) {
		Client c = father1.tsp_seq[i];
		child.tsp_seq[i] = c;
		in_child.insert(c);
	}

	// 2. Completar los huecos restantes leyendo circularmente del Padre 2 comenzando en cut2 + 1
	int child_idx = (cut2 + 1) % n;
	for (int step = 0; step < n; ++step) {
		int p2_idx = (cut2 + 1 + step) % n;
		Client candidate = father2.tsp_seq[p2_idx];

		if (in_child.find(candidate) == in_child.end()) {
			while (child.tsp_seq[child_idx] != -1) {
				child_idx = (child_idx + 1) % n;
			}
			child.tsp_seq[child_idx] = candidate;
			in_child.insert(candidate);
			child_idx = (child_idx + 1) % n;
		}
	}

	// Resguardo en caso de que alguna secuencia no contenga todos los clientes
	if (in_child.size() < static_cast<size_t>(n)) {
		for (Client c = 1; c <= tdpdp.c && in_child.size() < static_cast<size_t>(n); ++c) {
			if (in_child.find(c) == in_child.end()) {
				while (child.tsp_seq[child_idx] != -1) {
					child_idx = (child_idx + 1) % n;
				}
				child.tsp_seq[child_idx] = c;
				in_child.insert(c);
				child_idx = (child_idx + 1) % n;
			}
		}
	}

	return child;
}

/* Función que genera una mutuación dentro del individuo original
 *
 * Parámetros:
 * - indv (Individual): Instancia del problema tdpdp
 * - tdpdp (TDPDP): Datos del problema tdpdp asociado a las instancias
 * - type (int): Tipo de mutación a aplicar (MUT_TWOOPT, MUT_SWAP, MUT_SCRAMBLE)
 *
 * Retorno
 * - Retorna una nueva instancia del problema con una mutuación respecto al individuo original (indv)
 * */
Individual mutation(const Individual &indv, const TDPDP &tdpdp, const int type){
	Individual mutated;
	mutated.tsp_seq = indv.tsp_seq;

	const int n = static_cast<int>(mutated.tsp_seq.size());
	if (n < 2) {
		return mutated;
	}

	std::uniform_int_distribution<int> dist(0, n - 1);

	if (type == MUT_TWOOPT) {
		int i = dist(get_rng());
		int j = dist(get_rng());
		while (i == j) {
			j = dist(get_rng());
		}
		if (i > j) {
			std::swap(i, j);
		}
		std::reverse(mutated.tsp_seq.begin() + i, mutated.tsp_seq.begin() + j + 1);
	} else if (type == MUT_SWAP) {
		int i = dist(get_rng());
		int j = dist(get_rng());
		while (i == j) {
			j = dist(get_rng());
		}
		std::swap(mutated.tsp_seq[i], mutated.tsp_seq[j]);
	} else if (type == MUT_SCRAMBLE) {
		int i = dist(get_rng());
		int j = dist(get_rng());
		while (i == j) {
			j = dist(get_rng());
		}
		if (i > j) {
			std::swap(i, j);
		}
		std::shuffle(mutated.tsp_seq.begin() + i, mutated.tsp_seq.begin() + j + 1, get_rng());
	}

	return mutated;
}

/* Función que realiza la reparación de algún individuo
 * La reparación unicamente considera el TSP 
 * NOTA: Probablemente esta función no tenga uso en el largo plazo, pero la dejare por si acaso.
 * */
void repair(Individual &indv, const TDPDP &tdpdp){

}

/* Función que genera una instancia inicial para el TDPDP mediante GRASP (Randomized Nearest Neighbor)
 *
 * Parámetros:
 * - tdpdp (TDPDP): Datos del problema TDPDP asociado a las instancias
 * - k (int): Cantidad de mejores vecinos más cercanos a considerar en la Lista Restringida de Candidatos (RCL)
 *
 * Retorno:
 * - Retorna un individuo con los datos de tsp_seq generados mediante la heurística constructiva aleatorizada.
 * */
Individual random_nearest_neighborhood(const TDPDP &tdpdp, int k){
	Individual ind;
	if (tdpdp.c <= 0) {
		return ind;
	}

	const int c = tdpdp.c;
	std::vector<Client> tour;
	tour.reserve(c);

	std::vector<bool> visited(c + 1, false);
	Client current = 0; // Se inicia la ruta desde el depósito inicial (nodo 0)

	std::vector<std::pair<Time, Client>> candidates;
	candidates.reserve(c);

	for(int step = 0; step < c; ++step){
		candidates.clear();
		for(Client cand = 1; cand <= c; ++cand){
			if(!visited[cand]){
				candidates.push_back({tdpdp.tauTruck[current][cand], cand});
			}
		}

		int cand_count = static_cast<int>(candidates.size());
		int rcl_size = std::min(cand_count, std::max(1, k));

		std::partial_sort(candidates.begin(), candidates.begin() + rcl_size, candidates.end(),
			[](const std::pair<Time, Client>& a, const std::pair<Time, Client>& b){
				return a.first < b.first;
			}
		);

		// Selección con probabilidad uniforme entre los rcl_size mejores candidatos
		std::uniform_int_distribution<int> dist_rcl(0, rcl_size - 1);
		int chosen_idx = dist_rcl(get_rng());
		Client next_client = candidates[chosen_idx].second;

		tour.push_back(next_client);
		visited[next_client] = true;
		current = next_client;
	}

	ind.tsp_seq = std::move(tour);
	return ind;
}

/* Función que genera instancias iniciales para el tdpdp mediante GRASP (Randomized Nearest Neighbor)
 *
 * Parámetros
 * - tdpdp (TDPDP): Datos del problema tdpdp asociado a las instancias
 * - n (int): Cantidad de individuos en la población
 *
 * Retorno:
 * - Retorna una población donde cada individuo viene con los datos del tsp_seq completados.
 * */
std::vector<Individual> generatePopulation(const TDPDP &tdpdp, const int n){
	std::vector<Individual> population;
	if (n <= 0) return population;
	population.reserve(n);

	for(int j=0; j<n; ++j){
		population.push_back(random_nearest_neighborhood(tdpdp, 5));
	}

	return population;
}

