# INF295 - TDPDP code

Integrante: Facundo Riquelme Lucero
Rol: 202373046-7
Proyecto: Truck Dron Parcel Delivery Problem
Entrega: Presentación Arte (1)

## Consideraciones:

Por el momento, el codigo realiza tres tests unicos:
- Revisa que pueda leer instancias (Especificamente la primera instancia de las entregadas)
- Revisa que se puedan generar instancias de manera aleatoria
- Revisa que se pueda evaluar (realizando un `greedyDrones` de manera previa

Pese a que se tenga los demás comandos, no se incluyen dentro del main (`main` corresponde a `TDPDP_Evolutivo.cpp`) dado que no se han probado con certeza. Además, no se consideran parte de la entrega del estado de Arte.

## Ejecución

Para ejecutar el programa, debe primero de compilarlo mediante `make`

Tras eso, se puede ejecutar mediante `./sol` (nombre provisto)

Para eliminar el residuo (ejecutable), utilizar `make clean`

Todo esto debe de realizarse desde la carpeta code

## Estructura

Se tienen los siguientes archivos:

```
header.hpp
TDPDP_Evolutivo.cpp
algorithms
|  - evolutivo.cpp
|  - utils.cpp
|  - drones.cpp
data
|  - instancias
|  |  - INSTANCIA_010_new.txt
|  |  - ...
```

Los archivos contienen los siguientes datos:

- `header.hpp`: Contiene las declaraciones iniciales del problema. Incluye los struct de `TDPDP`, `Individuo`, constantes, y otros. Además se tiene las declaraciones de las funciones dentro de `algorithms`

- `evolutivo.cpp`: Contiene todas las funciones con respecto al evolutivo del TSP (Ver presentación). Estas incluyen:
  - `fitness()`: Calcula la evaluación del individuo (Esta especificamente evalua el TSP o el TDPDP dependiendo de los datos)
  - `crossing()`: Realiza un hijo entre dos padres
  - `mutation()`: Genera una mutuación de algún individuo
  - `repair()`: Repara algún individuo (No calculado aún)
  - `generatePopulation`: Genera una cantidad $n$ de individuos particulares para una población. Todos son TSP automaticamente generados.

- `drones.cpp`: Incluye las funciones que transforman al TSP a TDPDP.
  - `dpDrones()`: Genera mediante un dp la asignación de los drones (Por programar)
  - `greedyDrones()`: Genera mediante un greedy la asignación de los drones

- `utils.cpp`: Utilidades varias
  - `read_file`: Lee la instancia de algún filepath especifico

- `data/instanncias/`: Incluye las instancias entregadas en el enunciado
- `data/instanncias/`: Incluye las instancias entregadasen el enunciado

- `TDPDP_Evolutivo`: Incluye todos los archivos anteriores y realiza el TDPDP_Evolutivo
