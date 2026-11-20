# INF295 - TDPDP code

Integrante: Facundo Riquelme Lucero
Rol: 202373046-7
Proyecto: Truck Dron Parcel Delivery Problem
Entrega: Implementación (2)

## Instrucciones de ejecución

> [!note] 
> Se entiende que para todos los siguientes comandos estos se ejecutan desde la carpeta code (Carpeta donde se incluye todo el codigo.
>
> En el caso de la entrega, esta corresponde a donde se encuentra el README.md, TDPDP_evolutvo y las otras carpetas.

El programa se puede compilar mediante el siguiente comando:

```bash
make
```

Para ejecutar una instancia se debe de utilizar el ejecutable que entrega, llamado `exec`. Luego se ejecuta mediante:

```bash
./exec <parametros>
```

Donde `<parametros>` corresponden a los parámetros a utilizar. Para saber la lista de parámetros utiliza el comando help incluido desde el propio ejecutable de la siguiente manera:

```bash
./exec --help
```

El comando retorna la siguiente lista de opciones:
```
  -h,     --help              Print this help message and exit
  -n,     --n INT             Tamaño de la Población (n)
  -g,     --g INT             Cantidad de generaciones/iteraciones del evolutivo (g)
  -m,     --mut FLOAT         Probabilidad de mutación (p_mut)
  -c,     --cro FLOAT         Probabilidad de cruzamiento (p_cro)
  -i,     --inst TEXT         Archivo con instancias a generar
  -s,     --seed INT          Semilla a utilizar en la generación de números aleatorios
  -p,     --print             Imprime el estado actual del algoritmo
  -r,     --result            Imprime el resultado del algoritmo (para tunear usando ParamILS)
          --grasp             Indica si generatePopulation utiliza GRASP o shuffle aleatorio (random-start)
```

Para ejecutar una instancia se debe de indicar como mínimo algún archivo mediante `-i <file-path>`. Las instancias se encuentran dentro de la carpeta `data/instancias`.

Se tienen comandos en el make que permiten ciertas tipo de ejecuciones. El siguiente comando ejecuta la realización de tuning para paramILS.

```bash
make tune
```

El paramILS toma sus parámetros desde la carpeta `data/parameters`. Se tienen los siguientes archivos:
- `params.txt`: Indica que parámetros se va a probar en el formato `<option> {<values>}[<default value>]`, donde option son las opciones mencionadas anteriormente, `<values>` una lista de posibles valores que podria tomar la opción (Ejemplo, {30,50,100} indicaria que puede tener valores 30, 50 o 100) y `<default value>` es el valor por defecto a usar
- `scenario.txt` contiene confguraciones para el tuneo. Hay varias configuraciones que no se deben tocar, sin embargo hay dos utiles en caso de querer modificar el paramILS. 
  - La primera es `tunerTimeout = <tiempo(s)>` que indica cuanto tiempo estara paramILS tuneando. El entero indica segundos. Actualmente se encuentra en 8hrs = $28800$ s.
  - La segunda es `cutoff_tiem = <tiempo(s)>`que indica cuanto tiempo estara como máximo ejecutandose una instancia del paramILS hasta considerar que fue demasiado. En esos casos se penaliza. Actualmente se encuentra en $30$s.
- `tuning_inst.txt` indica que instancias se utilizaran para tunear el algoritmo mediante su ruta relativa desde `code`. En este caso se configuro para tunear con las instancias de tamaño $c=10$ y $c=50$.
- `test_inst.txt` indica que instancias se utilizaran para probar la configuración del algoritmo. En este caso se utiilizan las instancias de tamaño $c=100$.

Se tienen scripts de bash que permiten la automatización de ejecución de instancias. El siguiente comando ejecuta todas las instancias con una configuración especifica. Se encarga además de recompilar en codigo en caso de ser necesario.

```bash
make run-all
```

En el make solo se ejecuta con la configuración default de $n=200$, $g=200$, $p_{mut}=0.3$ y $p_{cro}=0.3$. Si se quiere ejecutar con parámetros personalizados utilizar el bash directamente.

```bash
bash ./scripts/run_all.sh -n <valor-n> -g <valor-g> --mut <valor-mut> --cro <valor-cro> --seed <valor-seed> <print> <grasp>
```

En el caso especifico de print, si se indica en print la opción `-p`, se imprimira por pantalla el fitness del mejor individuo. Si se agrega `--grasp` se cambiara a que generen individuos aleatorios mediante GRASP.

El siguiente script realiza la ejecución de todas las instancias con cada uno de los parámetros a probar en `data/parameters_code`, los cuales son referenciados en el informe. Este comando es el que se uso para ejecutar el informe directamente.

```bash
make run-exp
```

Se tiene los siguientes comandos para generar y limpiar las graficas:

```bash
make plots // Generar graficas
make clean-plots // Limpiar graficas
```

Cuando se ejecutan los algoritmos, estos iran guardando los resultados en results.csv, independiente si habian resultados de otras ejecuciones previas. Para borrar todas las instancias guardadas usar el siguiente comando:

```bash
make clean-results
```

Para eliminar el resultado de compilar usar el siguiente comando:

```bash
make clean
```

## Estructura

Por si se quiere revisar en detalle, se tiene la siguiente estructura de archivos:

```
code (carpeta actual)
| - README.md
| - TDPDP_Evolutivo.cpp
| - makefile
| - libs/
|   | - CLI11.hpp
|   | - paramILS/
| - algorithms/
|   | - drones.cpp
|   | - evolutivos.cpp
|   | - plots.py
|   | - process_tuned_results.py
|   | - utils.cpp
| - data/
|   | - instancias/
|   | - output/
|   | - parameters/
|   | - parameters_code/
|   | - plots/
|   | - results/
```

Se explicaran algunos algoritmos especificos:

### Librerias

Se deben de indicar en nuestro caso que librerias se usaron. En este trabajo usamos dos:
- `ParamILS`: Libreria utilizada para tunear [Source](https://www.cs.ubc.ca/labs/algorithms/Projects/ParamILS/)
- `CLI11.hpp`: Libreria para C++ para usar parámetros de mejor forma [Source: Github](https://github.com/CLIUtils/CLI11/tree/main)

Estas se guardaron en la carpeta `libs/`, por lo que no es necesario descargarlas

### Algoritmo

El algoritmo principal tiene el siguiente flujo y los siguientes archivos:
+ `header.hpp`: Declaraciones de structs, constantes y funciones a realizar por todo el algoritmo. Se asegura también que la semilla se use en todos los otros archivos.
+ `utils.cpp`: Funciones de escritura y lectura
+ `evolutivos.cpp`: Implementaciones especificas del evolutvo, como generar población inicial (`generatePopulation()`, el vecino más random cercano (`random_nearest_neighborhood()`), mutuación (`mutuation()`), cruzamiento (`crossing()`) y función de fitness (evaluación) (`fitness()`)
+ `drones.cpp`: Implementación del DP para la función SPLIT. Incluye un greedy que se utilizo en entrega pasada.
+ `TDPDP_Evolutivo.cpp`: Implemetación de los ciclos del evolutivo. Toma los parámetros y los aplica al algoritmo. Escribe los resultados en `results.csv` y `results_per_iteration.csv` en `data/results`

### Plots

Para los plots se tiene los siguientes `.py` que usan los datos guardados en `data/results`:

- `plots.py`: Genera las graficas necesarias
- `process_tuned_results.py`: Calcula los datos resultantes de promedio, diferencia de GAP, etc.

Los plots quedaran guardados en `data/plots`.

> [!warning] Exageramiento de plots
> Se tiene un monton más de graficas que las utilizadas en el informe. Esto no es un error, si no que se utilizo para poder analizar de mejor manera.
> Dado que se mejoro un plot_generator tomado de un curso anterior (Algoco) para esta tarea, hay varios graficos que se generaron y no se usaron. Además, esta parte de las imagenes fue automatizada por IA, el generador fue mejorado mediante esta.

### Data

Se tienen otros datas que ya se explicaron previamente:

- `instancias/`: Guarda las instancias entregadas en el enunciado
- `output`: Guarda los resultados del tuneo de _ParamILS_
- `parameters`: Guarda los parámetros para _ParamILS_
- `parameters_code`: Guarda los parámetros a utilizar en la ejecución del algoritmo con todas las posibilidades indicadas en el informe. Separadas también por tipo.


