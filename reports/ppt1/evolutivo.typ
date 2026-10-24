#import "@preview/cetz:0.5.3"
#import "@preview/codly:2.0.0" as codly
#show raw.where(block: true): codly.new

== Individuo (TruckFirst, DroneSecond)

+ Representación Camión ( _TSP_ ): Encargada de la sección de TSP, nodos y viajes.
+ Representacion Drone ( _TDPDP_ ): Encarga de la sección especifica de TDPDP, restricciones de dron.

#align(center + horizon,
  figure(
    cetz.canvas({
      import cetz.draw: *
      let dist = 1.5
      let lgray = gray.lighten(50%)
      let lblue = blue.lighten(75%)

      for (c, t, f) in (
        ((0, 0), "0", lgray),
        ((1 * dist, 1 * dist), "1", none),
        ((2 * dist, 1 * dist), "2", none),
        ((2 * dist, 0), "3", none),
        ((1 * dist, -1 * dist), "4", none),
      ) {
        circle(c, name: t, radius: 0.4, fill: f)
        content(t, [#t])
      }
      line("0","1")
      line("1","2")
      line("2","3")
      line("3","4")
      line("4","0")

      // Intermedio
      line((4,0),(5,0))
      line((6,0),(7,0))
      rect((5,-0.5), (rel: (1,1)), radius: 0)
      content((5.5,0), [*split*])

      let r = 8
      for (c, t, f) in (
        ((r, 0), "5", lgray),
        ((r+1 * dist, 1 * dist), "6", none),
        ((r+2 * dist, 1 * dist), "7", lblue),
        ((r+2 * dist, 0), "8", none),
        ((r+1 * dist, -1 * dist), "9", none),
      ) {
        circle(c, name: t, radius: 0.4, fill: f)
        content(t, [#(str(int(t) - 5))])
      }
      line("5","6")
      line("6","7",stroke: (dash: "dotted"))
      line("6","8")
      line("7","8",stroke: (dash: "dotted"))
      line("8","9")
      line("9","5")
    })
  )
)

#let genotipo = [
```cpp
struct Individual {
  std::vector<Client> tsp_seq; // Genotipo
  std::vector<Client> truck_seq; // Fenotipo
  std::vector<DronFlight> dron_seq; // Fenotipo
  // Auxiliar
  std::vector<DronFlight> dron_seq; 
  std::set<Client> dron_launch; 
  // Evaluación
  Time fit = 0; 
  bool is_calculated = TO_CALC; 
  void print()...
}
```
]

#align(center + horizon,
  figure(
    genotipo,
    caption: [Genotipo y Fenotipo para TDPDP]
  )
)

== Evaluación

#let eval = [
```cpp
...
for (Client next_node : truck_stops) {
  // Tiempo del camión para trasladarse al siguiente cliente
  current_time += tdpdp.tauTruck[current_node][next_node];
  current_node = next_node;
  // Si el dron se reúne con el camión en current_node: Se calcula el máximo entre la llegada del camión y la del dron, más tiempo de recuperación SR
  if (drone_in_flight && drone_recovery_node == current_node) {
    current_time = std::max(current_time, drone_arrival_time) + tdpdp.SR;
    drone_in_flight = false;
    drone_recovery_node = NO_DRON_CLIENTS;
  }
  // Si se lanza un dron desde current_node (incluyendo caso de re-lanzamiento en el mismo nodo): Se agrega el tiempo de setup de lanzamiento SL
  if (!drone_in_flight && launch_map.find(current_node) != launch_map.end()) {
    const auto& flight = launch_map.at(current_node);
    current_time += tdpdp.SL;
    drone_in_flight = true;
    drone_recovery_node = flight.k;
    drone_arrival_time = current_time + tdpdp.tauDrone[flight.i][flight.j] + tdpdp.tauDrone[flight.j][flight.k];
  }}
...
```
]

#show raw.where(block: true): set text(size: 6pt)

#align(horizon,
[
La evaluación corresponde al tiempo máximo. El algoritmo considera:
- Tiempo del camión de ir un nodo a otro
- En caso de que se lanze un dron, se incluye el tiempo de lanzamiento ($"SL"$)
- En caso de que se reciba un dron, toma el máximo entre el tiempo de llegada del dron y el tiempo de llegada del camión. Luego le suma el tiempo de recuperación del dron ($"SR"$)
]
)

#pagebreak()

#align(center + horizon,
  figure(
    eval,
    caption: [Función de Evaluación `fitness()`]
  )
)

== Población Inicial

#show raw.where(block: true): set text(size: 7pt)

#let poblacion = [
```cpp
std::vector<Individual> generatePopulation(const TDPDP tdpdp, const int n){
    std::vector<Individual> population;
    population.reserve(n);

    std::vector<Client> base_tour(tdpdp.c);
    for(Client i=1; i<=tdpdp.c; ++i){base_tour[i-1] = i;}

    for(int j=0; j<n; ++j){
        std::vector<Client> tour = base_tour;
        std::shuffle(tour.begin(), tour.end(), rng);
        Individual ind;
        ind.tsp_seq = tour;
        population.push_back(ind);
    }

    return population;
}
```
]

#align(center + horizon,
  figure(
    poblacion,
    caption: [Algoritmo de Población Inicial (`generatePopulation`)]
  )
)

== Cruzamiento (OX Crossover)

#show raw.where(block: true): set text(size:5pt)
#align(center + horizon,
  figure(
    grid(
      columns: (1fr, 1fr),
      gutter: 10pt,
      align: top,
      
      // Columna Izquierda (Parte 1 del código)
      codly.new(
        ```cpp
        Individual crossing(const Individual father1, const Individual father2, const TDPDP tdpdp){
          Individual child;
          const int n = static_cast<int>(father1.tsp_seq.size());
          ...
          std::uniform_int_distribution<int> dist(0, n - 1);
          int cut1 = dist(rng);
          int cut2 = dist(rng);
          if (cut1 > cut2) {
                std::swap(cut1, cut2);
          }
          if (n > 2) {
            while (cut1 == cut2 || (cut1 == 0 && cut2 == n - 1)) {
              cut1 = dist(rng);
              cut2 = dist(rng);
              if (cut1 > cut2) {
                std::swap(cut1, cut2);
              }
            }
          } else if (n == 2) {
              cut1 = dist(rng);
              cut2 = cut1;
          }
          ...
        ```,
        number-enabled: false,
      ),

      // Columna Derecha (Parte 2 del código)
      codly.new(
        ```cpp
          // 1. Copiar subsecuencia continua del Padre 1
          for (int i = cut1; i <= cut2; ++i) {
                Client c = father1.tsp_seq[i];
                child.tsp_seq[i] = c;
                in_child.insert(c);
          }
          // 2. Completar los huecos restantes del Padre 2
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
          ...
          return child;
        }
        ```,
        number-enabled: false,
      )
    ),
    caption: [Algoritmo de Cruzamiento (`crossing`)]
  )
)

== Mutuación (2-opt, Swap, Scramble)

#align(horizon,
[
- Swap (Intercambio): Selecciona aleatoriamente dos posiciones $i$ y $j$ en la permutación e intercambia las posiciones de los dos clientes .
- Inversion (2-opt): Selecciona dos puntos de corte aleatorios e invierte el orden del bloque de clientes comprendido entre ellos (por ejemplo, $[1, 2, {3, 4, 5}, 6] -> [1, 2, {5, 4, 3}, 6]$). 
- Scramble. Tomar dos puntos $i$ y $j$, dejando $i$ al inicio de su ruta y a $j$ al final. Todas las ciudades entremedio son randomizadas.
])

#pagebreak()

Con función `Individual mutation(Individual &indv, const TDPDP tdpdp, const int type)`

#show raw.where(block: true): set text(size:6pt)
#align(center + horizon,
  figure(
    grid(
      columns: (1fr, 1fr, 1fr),
      gutter: 10pt,
      align: top,
      
      ```cpp
        ...
        if (type == MUT_TWOOPT) {
          int i = dist(rng);
          int j = dist(rng);
          while (i == j) {
            j = dist(rng);
          }
          if (i > j) {
            std::swap(i, j);
          }
          std::reverse(mutated.tsp_seq.begin() + i, mutated.tsp_seq.begin() + j + 1);
        }
        ...
      ```,

      ```cpp
        ...
        if (type == MUT_SWAP) {
          int i = dist(rng);
          int j = dist(rng);
          while (i == j) {
            j = dist(rng);
          }
          std::swap(mutated.tsp_seq[i], mutated.tsp_seq[j]);
        ...
      }
      ```,
      ```cpp
        ...
        } else if (type == MUT_SCRAMBLE) {
          int i = dist(rng);
          int j = dist(rng);
          while (i == j) {
            j = dist(rng);
          }
          if (i > j) {
            std::swap(i, j);
          }
          std::shuffle(mutated.tsp_seq.begin() + i, mutated.tsp_seq.begin() + j + 1, rng);
        }

      ```
    ),
    caption: [Algoritmo de Mutación (`mutation`)]
  )
)

== Asignación Vuelo: Greedy v/s DP

#align(horizon,
[
La asignación del vuelo puede ser programada mediante dos algoritmos:
- *Greedy*: Realizar una asignación Greedy del problema
- *DP*: Programación Dinámica del problema. Knapsack modificado donde se decide si tomar o no un vuelo perteneciente a $F$

Greedy se encuentra implementado en `greedyDrones`, mientras que DP se encuentra en desarrollo (`dpDrone`)

El siguiente codigo tiene firma `greedyDrones(Individual &indv, const TDPDP tdpdp)`
]
)

#pagebreak()

#let criterio = [
```cpp
  ...
  // Candidato a dron
  cand_ind.dron_seq = indv.dron_seq;
  DronFlight new_flight = {i_node, j, k_node};
  cand_ind.dron_seq.push_back(new_flight);
  std::sort(cand_ind.dron_seq.begin(), cand_ind.dron_seq.end(), [&cand_node_pos](const DronFlight& a, const DronFlight& b) { return cand_node_pos[a.i] < cand_node_pos[b.i];});
  cand_ind.dron_launch = indv.dron_launch; 
  cand_ind.dron_launch.insert(i_node);
  // Evaluar beneficio neto
  Time cand_fit = fitness(cand_ind, tdpdp);
  Time saving = current_fit - cand_fit;
  // Seleccionar la combinación con el máximo ahorro neto
  if (saving > best_saving && saving > 1e-6) {
    best_saving = saving;
    best_candidate = cand_ind;
    found_better = true;
  }
```
]

#let selectj = [
```cpp
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
```
]

#let pairs = [
```cpp
// Explorar las combinaciones del camión lanzando en i.
for (int idx_i = 0; idx_i < cand_route_len - 1; ++idx_i) {
  // Revisar que desde el nodo actual no se haya lanzado o recuperado
  Client i_node = cand_full_route[idx_i];
  if (used_launch.find(i_node) != used_launch.end() ||
    used_recovery.find(i_node) != used_recovery.end()) {continue;}
  // Candidatos a agregar ruta
  for (int idx_k = idx_i + 1; idx_k < cand_route_len; ++idx_k) {
    Client k_node = cand_full_route[idx_k];
    if (used_recovery.find(k_node) != used_recovery.end() ||
      used_launch.find(k_node) != used_launch.end()) {
      continue;
    }
    // 1. Revisar que el dron pueda resistir
    Time drone_flight_time = tdpdp.tauDrone[i_node][j] + tdpdp.tauDrone[j][k_node];
    if (drone_flight_time > tdpdp.E) {
      continue;
    }
    // 2. Comprobar no solapamiento con salidas de dron existentes
    bool overlaps = false;
    for (const auto& interval : cand_existing_intervals) {
      if (!(idx_k <= interval.start_idx || idx_i >= interval.end_idx)) {overlaps = true; break;}
    }
    if (overlaps) {continue;} // No sirve
```
]


#show raw.where(block: true): set text(size:4pt)
#align(center + horizon,
  figure(
    grid(
      columns: (1fr, 1fr, 1fr),
      gutter: 10pt,
      align: top,
      selectj,
      pairs,
      criterio
    ),
    caption: [Algoritmo de Asignación Greedy)]
  )
)

