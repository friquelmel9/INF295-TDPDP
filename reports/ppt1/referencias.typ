#import "@preview/cetz:0.5.3"
#import "@preview/codly:2.0.0" as codly
#show raw.where(block: true): codly.new

== Bibliografía principal

+ *Murray, C. C., & Chu, C. W. (2015).* The flying sidekick traveling salesman problem: Optimization of drone-assisted parcel delivery. Transportation Research Part C: Emerging Technologies, 54, 86-109.
+ *Boysen, N., Briskorn, D., Fedtke, S., & Schwerdfeger, S. (2018)*. _Drone delivery from trucks: Drone scheduling for given truck routes. Networks, 72(4), 506-527._
+ *Ponza, A. (2016)*. _Optimization of drone-assisted parcel delivery with vehicle routing problem_. Master's thesis, Università degli Studi di Padova.
+ *Toth, P., & Vigo, D. (Eds.). (2014)*. _Vehicle Routing: Problems, Methods, and Applications (2nd ed.). MOS-SIAM Series on Optimization_, SIAM.
+ *Moshref-Javadi, M., & Winkenbach, M. (2021)*. _Applications and research avenues for drone-based models in logistics: A classification and review_. Expert Systems with Applications, 174, 114854.
+ *Freitas, J. C., Penna, P. H., & Toffolo, T. A. (2021)*. _A tighter formulation and a hybrid heuristic for the Flying Sidekick Traveling Salesman Problem_. arXiv preprint arXiv:2108.01996.
+ *Thomas, T., Srinivas, S., & Rajendran, C. (2024)*. _Collaborative truck multi-drone delivery system considering drone scheduling and en route operations_. Annals of Operations Research, 339(1), 693-739.
+ *Zieher, S., Olcay, E., Kefferpütz, K., Salamat, B., Olzem, S., Elsbacher, G., & Meeß, H. (2024)*. _Drones for automated parcel delivery: Use case identification and derivation of technical requirements_. Transportation Research Interdisciplinary Perspectives, 28, 101253.
+ *Prins, C. (2004)*. _A simple and effective evolutionary algorithm for the vehicle routing problem_. Computers & Operations Research, 31(12), 1985-2002.
+ *An Evolutionary Algorithm for VRPD (2021).* _An evolutionary algorithm for the vehicle routing problem with drones and interceptions_. Research article on last-mile drone delivery optimization.
+ *Evolutionary Algorithms for VRPTW (2014)*. _Evolutionary algorithms for the vehicle routing problem with time windows_. Review of genetic algorithms and local search hybrids for VRP.

== Uso de IA

#align(horizon,
[
Se utilizaron las siguientes IA: Gemini + NotebookLM
- NotebookLM se utilizó para referenciar las fuentes
- Gemini se utilizó en apoyo del código (Bugfixes y tipado)
])

== Anexo

=== Constantes definidas

#show raw.where(block: true): set text(size:5pt)

#let const = [
```cpp
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
```
]

#align(center + horizon,
  figure(
    const,
    caption: [Constantes Definidas]
  )
)

#pagebreak()

