/* Función que asigna los drones utilizando algoritmo de Programación Dinámica
 *
 * Parámetros:
 * - indv (Individual): Ubicación de memoria de un individuo
 * - tdpdp (TDPDP): Datos del problema tdpdp
 *
 * Retorno:
 * - No realiza un retorno, pero guarda el viaje optimo en indv.dronFlights.
 * */
void dpDrones(Individual &indv, const TDPDP tdpdp){

}

/* Función que asigna los drones utilizando algoritmo de Greedy
 *
 * Parámetros:
 * - indv (Individual): Ubicación de memoria de un individuo
 * - tdpdp (TDPDP): Datos del problema tdpdp
 *
 * Retorno:
 * - No realiza un retorno, pero guarda el viaje optimo en indv.truck_seq, indv.dron_seq y indv.dron_launch.
 * */
void greedyDrones(Individual &indv, const TDPDP tdpdp){
    if (indv.tsp_seq.empty()) {
        return;
    }

    // Inicialización del camión y dron a partir de tsp_seq
    indv.truck_seq = indv.tsp_seq;
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
        full_route.reserve(indv.truck_seq.size() + 2);
        full_route.push_back(start_depot);
        for (Client c : indv.truck_seq) {
            full_route.push_back(c);
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
        for (size_t j_idx = 0; j_idx < indv.truck_seq.size(); ++j_idx) {
            Client j = indv.truck_seq[j_idx];

            // Comprobar que j sea elegible para ser atendido por el dron
            if (tdpdp.Cdrones.find(j) == tdpdp.Cdrones.end()) {
                continue;
            }

            // Generar ruta de camión tentativa sin el cliente j
            std::vector<Client> cand_truck_seq;
            cand_truck_seq.reserve(indv.truck_seq.size() - 1);
            for (size_t p = 0; p < indv.truck_seq.size(); ++p) {
                if (p != j_idx) {
                    cand_truck_seq.push_back(indv.truck_seq[p]);
                }
            }

            std::vector<Client> cand_full_route;
            cand_full_route.reserve(cand_truck_seq.size() + 2);
            cand_full_route.push_back(start_depot);
            for (Client c : cand_truck_seq) {
                cand_full_route.push_back(c);
            }
            cand_full_route.push_back(end_depot);

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
            cand_ind.truck_seq = cand_truck_seq;

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
