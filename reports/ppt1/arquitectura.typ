#import "@preview/cetz:0.5.3"
#import "@preview/codly:2.0.0" as codly
#show raw.where(block: true): codly.new

== Testcases

#show raw.where(block: true): set text(size: 5pt)

#let read = [
```cpp
void read_file(std::string filepath, TDPDP &inst){
  inst.isInstance = false;
  std::ifstream file(filepath);
  ...
  file >> inst.c; file >> inst.SL; file >> inst.SR; file >> inst.E;
  // Lectura de los Cdrones
  std::string line;
  std::getline(file >> std::ws, line);
  std::stringstream ss(line);
  Client cdron;
  while(ss >> cdron){inst.Cdrones.insert(cdron);}
  // Tau
  int num_nodes = inst.c + 2;
  // Camión
  inst.tauTruck.assign(num_nodes, std::vector<Time>(num_nodes, 0.0));
  for(int i=0; i<num_nodes; ++i){for(int j=0; j<num_nodes; ++j){file >> inst.tauTruck[i][j];}}
  // Dronesk
  inst.tauDrone.assign(num_nodes, std::vector<Time>(num_nodes, 0.0));
  for(int i=0; i<num_nodes; ++i){ for(int j=0; j<num_nodes; ++j){ file >> inst.tauDrone[i][j];}}
  // Conjunto F
  file >> inst.size_F;
  inst.F.resize(inst.size_F);
  for(int i=0; i<inst.size_F; ++i){file >> inst.F[i].i >> inst.F[i].j >> inst.F[i].k;}
  file.close();
  inst.isInstance = true; // Función auxiliar para saber que el tdpdp se encuentra cargado
}
```
]

#align(horizon,
[
Los testcases fueron entregados en `Instancias.zip`. Contienen la información mencionada en el struct de TDPDP.

Estos contienen los datos descritos en el modelo (c, SL, SR, E, F, $tau_T$, $tau_D$, Cdrones)

Se leen mediante la función `read_file()`
]
)

#align(center + horizon,
  figure(
    read,
    caption: [Lectura de Testcases `read_file()`]
  )
)

== Estructura Main

La estructura del archivo está en el main, archivo `TDPDP_Evolutivo.cpp`. Realizará los siguientes pasos:

+ Lee todos los archivos dentro de `data/instancias`
+ Por cada instancia, recibe los datos mediante `read_file()`
+ Se realiza el algoritmo evolutivo.

La implementación concreta del evolutivo está en desarrollo. Se está buscando encontrar...
- Tamaños recomendados para $n$ (Cantidad de individuos en la población)
- Porcentajes para la `mutation`, `crossing`, y elitismo
- Revisión del `dpDrones` y `greedyDrones`, para saber cuál realiza mejor mejora y con cuánto tiempo de diferencia.


