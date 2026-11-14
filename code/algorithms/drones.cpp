/* Función que asigna los drones utilizando algoritmo de Programación Dinámica
 *
 * Parámetros:
 * - indv (Individual): Ubicación de memoria de un individuo
 * - tdpdp (TDPDP): Datos del problema tdpdp
 *
 * Retorno:
 * - Guarda la asignación óptima de dron en indv.seq, indv.dron_seq y indv.dron_launch,
 *   y actualiza indv.fit e indv.is_calculated.
 * */
void dpDrones(Individual &indv, const TDPDP &tdpdp){
	if (indv.tsp_seq.empty()) {
		return;
	}

	const int c = tdpdp.c;
	const Client start_depot = 0;
	const Client end_depot = c + 1;

	// Vector V con la secuencia completa incluyendo depósitos: 0, s_1, s_2, ..., s_c, c+1
	const int n_nodes = c + 2;
	std::vector<Client> V(n_nodes);
	V[0] = start_depot;
	for (int idx = 0; idx < c; ++idx) {V[idx + 1] = indv.tsp_seq[idx];}
	V[c + 1] = end_depot;

	// Precalcular sumas prefijas de distancias del camión para la secuencia V
	// prefix_truck[k] es la distancia de recorrer V[0] -> V[1] -> ... -> V[k]
	std::vector<Time> prefix_truck(n_nodes, 0.0);
	for (int k = 1; k < n_nodes; ++k) { prefix_truck[k] = prefix_truck[k - 1] + tdpdp.tauTruck[V[k - 1]][V[k]];}

	// dp[k]: tiempo mínimo acumulado para atender a todos los clientes hasta el nodo V[k], estando el camión en V[k] con el dron disponible.
	const Time INF = std::numeric_limits<Time>::infinity();
	std::vector<Time> dp(n_nodes, INF);
	std::vector<int> prev_node(n_nodes, -1);
	std::vector<Client> drone_customer(n_nodes, NO_DRON_CLIENTS);

	dp[0] = 0.0;

	for (int k = 1; k < n_nodes; ++k) {
		// Opción 1: Paso directo del camión desde V[k-1] hasta V[k] (sin salida de dron)
		if (dp[k - 1] < INF) {
			Time cost = dp[k - 1] + tdpdp.tauTruck[V[k - 1]][V[k]];
			if (cost < dp[k]) {
				dp[k] = cost;
				prev_node[k] = k - 1;
				drone_customer[k] = NO_DRON_CLIENTS;
			}
		}

		// Opción 2: Salida de dron lanzada en V[i] (i < k - 1) que atiende a V[j] (i < j < k) y se reúne con el camión en V[k]. El camión atiende a todos los nodos intermedios entre V[i] y V[k] excepto V[j].
		for (int i = 0; i < k - 1; ++i) {
			if (dp[i] == INF) continue;

			for (int j = i + 1; j < k; ++j) {
				Client j_node = V[j];

				// Si j es atendible por dron
				if (tdpdp.Cdrones.find(j_node) == tdpdp.Cdrones.end()) {continue;}

				// Autonomía de vuelo del dron (tauDrone(i, j) + tauDrone(j, k) <= E)
				Time drone_flight_time = tdpdp.tauDrone[V[i]][j_node] + tdpdp.tauDrone[j_node][V[k]];
				if (drone_flight_time > tdpdp.E) {continue;}

				// Tiempo de viaje del camión de V[i] a V[k] sin pasar por V[j] en O(1)
				Time truck_travel_time = 0.0;
				if (k == i + 2) {
					// Solo estaba j entre i y k; el camión va directo V[i] -> V[k]
					truck_travel_time = tdpdp.tauTruck[V[i]][V[k]];
				} else if (j == i + 1) {
					// j es el primer nodo después de i; el camión va V[i] -> V[i+2] -> ... -> V[k]
					truck_travel_time = tdpdp.tauTruck[V[i]][V[i + 2]] + (prefix_truck[k] - prefix_truck[i + 2]);
				} else if (j == k - 1) {
					// j es el último nodo antes de k; el camión va V[i] -> ... -> V[k-2] -> V[k]
					truck_travel_time = (prefix_truck[k - 2] - prefix_truck[i]) + tdpdp.tauTruck[V[k - 2]][V[k]];
				} else {
					// j está en el interior; camión va V[i] -> ... -> V[j-1] -> V[j+1] -> ... -> V[k]
					truck_travel_time = (prefix_truck[j - 1] - prefix_truck[i])
					                  + tdpdp.tauTruck[V[j - 1]][V[j + 1]]
					                  + (prefix_truck[k] - prefix_truck[j + 1]);
				}

				// Llegada coordinada a V[k]:
				Time arrival_time = dp[i] + tdpdp.SL + std::max(truck_travel_time, drone_flight_time) + tdpdp.SR;

				if (arrival_time < dp[k]) {
					dp[k] = arrival_time;
					prev_node[k] = i;
					drone_customer[k] = j_node;
				}
			}
		}
	}

	// Reconstrucción de la solución óptima recorriendo los predecesores
	indv.seq.assign(c, TRUCK_DELIVERY);
	indv.dron_seq.clear();
	indv.dron_launch.clear();

	std::vector<DronFlight> flights;
	int curr = c + 1;
	while (curr > 0) {
		int p = prev_node[curr];
		Client d = drone_customer[curr];
		if (d != NO_DRON_CLIENTS) {
			flights.push_back({V[p], d, V[curr]});
			indv.dron_launch.insert(V[p]);
		}
		curr = p;
	}

	// Las salidas de dron se invierten para guardarlas en orden cronológico
	std::reverse(flights.begin(), flights.end());
	indv.dron_seq = std::move(flights);

	// Marcar en seq los clientes atendidos por el dron (DRON_DELIVERY)
	std::unordered_set<Client> dron_clients;
	for (const auto& flight : indv.dron_seq) {dron_clients.insert(flight.j);}
	for (int idx = 0; idx < c; ++idx) {
		if (dron_clients.find(indv.tsp_seq[idx]) != dron_clients.end()) {
			indv.seq[idx] = DRON_DELIVERY;
		} else {
			indv.seq[idx] = TRUCK_DELIVERY;
		}
	}

	indv.fit = fitness(indv, tdpdp);
	indv.is_calculated = CALCULATED;
}

/* Función que asigna los drones utilizando algoritmo de Greedy
 *
 * Parámetros:
 * - indv (Individual): Ubicación de memoria de un individuo
 * - tdpdp (TDPDP): Datos del problema tdpdp
 *
 * Retorno:
 * - No realiza un retorno, pero guarda el viaje optimo en indv.seq, indv.dron_seq y indv.dron_launch.
 * */
void greedyDrones(Individual &indv, const TDPDP &tdpdp){
    if (indv.tsp_seq.empty()) {
        return;
    }

    // Inicialización del camión y dron a partir de tsp_seq
    indv.seq.assign(indv.tsp_seq.size(), TRUCK_DELIVERY);
    indv.dron_seq.clear();
    indv.dron_launch.clear();

    const Client start_depot = 0;
    const Client end_depot = tdpdp.c + 1;

    // Proceso iterativo greedy
    while (true) {
        Time current_fit = fitness(indv, tdpdp);
        Time best_saving = 0.0;
        bool found_better = false;

        Individual best_candidate;

        // Construir la ruta completa del camión actual incluyendo depósitos: 0, ..., c+1
        std::vector<Client> full_route;
        full_route.reserve(indv.tsp_seq.size() + 2);
        full_route.push_back(start_depot);
        for (size_t p = 0; p < indv.tsp_seq.size(); ++p) {
            if (indv.seq[p] == TRUCK_DELIVERY) {
                full_route.push_back(indv.tsp_seq[p]);
            }
        }
        full_route.push_back(end_depot);

        // Mapear cada nodo a su índice en la ruta completa actual
        std::unordered_map<Client, int> node_pos;
        for (size_t p = 0; p < full_route.size(); ++p) {
            node_pos[full_route[p]] = static_cast<int>(p);
        }

        // Identificar nodos que ya tienen asignado un dron (despegue o reencuentro)
        std::unordered_set<Client> used_launch;
        std::unordered_set<Client> used_recovery;
        struct FlightInterval {
            int start_idx;
            int end_idx;
        };
        std::vector<FlightInterval> existing_intervals;
        for (const auto& flight : indv.dron_seq) {
            used_launch.insert(flight.i);
            used_recovery.insert(flight.k);
            existing_intervals.push_back({node_pos[flight.i], node_pos[flight.k]});
        }

        // Paso 1: Evaluar cada cliente j elegible para dron visitado por el camión
        for (size_t j_idx = 0; j_idx < indv.tsp_seq.size(); ++j_idx) {
            if (indv.seq[j_idx] != TRUCK_DELIVERY) {
                continue;
            }
            Client j = indv.tsp_seq[j_idx];

            // Comprobar que j sea elegible para ser atendido por el dron
            if (tdpdp.Cdrones.find(j) == tdpdp.Cdrones.end()) {
                continue;
            }

            // Un nodo que ya actúa como punto de despegue o recuperación no puede ser atendido por dron
            if (used_launch.find(j) != used_launch.end() || used_recovery.find(j) != used_recovery.end()) {
                continue;
            }

            // Generar ruta de camión tentativa sin el cliente j
            std::vector<Client> cand_full_route;
            cand_full_route.reserve(full_route.size() - 1);
            for (Client c : full_route) {
                if (c != j) {
                    cand_full_route.push_back(c);
                }
            }

            std::unordered_map<Client, int> cand_node_pos;
            for (size_t p = 0; p < cand_full_route.size(); ++p) {
                cand_node_pos[cand_full_route[p]] = static_cast<int>(p);
            }

            // Mapear los intervalos de los vuelos existentes en la nueva ruta
            std::vector<FlightInterval> cand_existing_intervals;
            for (const auto& flight : indv.dron_seq) {
                cand_existing_intervals.push_back({cand_node_pos[flight.i], cand_node_pos[flight.k]});
            }

            int cand_route_len = static_cast<int>(cand_full_route.size());

            Individual cand_ind;
            cand_ind.tsp_seq = indv.tsp_seq;
            cand_ind.seq = indv.seq;
            cand_ind.seq[j_idx] = DRON_DELIVERY;

            // Paso 2: Explorar todos los pares de despegue i y reencuentro k (donde i precede a k)
            for (int idx_i = 0; idx_i < cand_route_len - 1; ++idx_i) {
                Client i_node = cand_full_route[idx_i];

                // i no debe tener aún asignado un dron (despegue o reencuentro)
                if (used_launch.find(i_node) != used_launch.end() ||
                    used_recovery.find(i_node) != used_recovery.end()) {
                    continue;
                }

                for (int idx_k = idx_i + 1; idx_k < cand_route_len; ++idx_k) {
                    Client k_node = cand_full_route[idx_k];

                    // k no debe tener aún asignado un dron (despegue o reencuentro)
                    if (used_recovery.find(k_node) != used_recovery.end() ||
                        used_launch.find(k_node) != used_launch.end()) {
                        continue;
                    }

                    // 1. Comprobar que la duración del vuelo no supere el límite de energía E
                    Time drone_flight_time = tdpdp.tauDrone[i_node][j] + tdpdp.tauDrone[j][k_node];
                    if (drone_flight_time > tdpdp.E) {
                        continue;
                    }

                    // Comprobar no solapamiento con salidas de dron existentes (un único dron disponible)
                    bool overlaps = false;
                    for (const auto& interval : cand_existing_intervals) {
                        if (!(idx_k <= interval.start_idx || idx_i >= interval.end_idx)) {
                            overlaps = true;
                            break;
                        }
                    }
                    if (overlaps) {
                        continue;
                    }

                    // Configurar el candidato incorporando el nuevo vuelo en orden cronológico
                    cand_ind.dron_seq = indv.dron_seq;
                    DronFlight new_flight = {i_node, j, k_node};
                    cand_ind.dron_seq.push_back(new_flight);
                    std::sort(cand_ind.dron_seq.begin(), cand_ind.dron_seq.end(),
                              [&cand_node_pos](const DronFlight& a, const DronFlight& b) {
                                  return cand_node_pos[a.i] < cand_node_pos[b.i];
                              });

                    cand_ind.dron_launch = indv.dron_launch;
                    cand_ind.dron_launch.insert(i_node);

                    // Paso 3: Evaluar beneficio neto: AhorroNeto = current_fit - cand_fit
                    Time cand_fit = fitness(cand_ind, tdpdp);
                    Time saving = current_fit - cand_fit;

                    // Seleccionar la combinación que entregue el máximo ahorro neto estrictamente positivo
                    if (saving > best_saving && saving > 1e-6) {
                        best_saving = saving;
                        best_candidate = cand_ind;
                        found_better = true;
                    }
                }
            }
        }

        // Si se encontró una reasignación con ahorro positivo, aplicar el cambio y repetir
        if (found_better) {
            indv = best_candidate;
        } else {
            // El proceso finaliza cuando ninguna reasignación logre un ahorro neto positivo
            break;
        }
    }

    // Actualizar el valor de evaluación final del individuo
    indv.fit = fitness(indv, tdpdp);
    indv.is_calculated = CALCULATED;
}
