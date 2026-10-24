#import "@preview/cetz:0.5.3"
#import "@preview/codly:2.0.0" as codly
#show raw.where(block: true): codly.new

#show raw.where(block: true): set text(size: 7pt)

== Representación Grafica

#align(center + horizon,
  grid(
    columns: (1fr, 1fr, 1fr),
    gutter: 15pt,
    align: center + horizon,

    // Primer dibujo (TSP)
    figure(
      cetz.canvas({
        import cetz.draw: *
        let dist = 1.5
        let lgray = gray.lighten(50%)

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

        line("0", "1")
        line("0", "2")
        line("0", "3")
        line("0", "4")
        line("1", "2")
        line("1", "3")
        line("1", "4")
        line("2", "3")
        line("2", "4")
        line("3", "4")
      }),
      caption: [Problema TSP]
    ),

    // Segundo dibujo (VRP)
    figure(
      cetz.canvas({
        import cetz.draw: *
        let dist = 1.5
        let lgray = gray.lighten(50%)

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
      }),
      caption: [Problema VRP (Con Parcel) Instanciado [9]]
    ),

    // Tercer dibujo (TDPDP)
    figure(
      cetz.canvas({
        import cetz.draw: *
        let dist = 1.5
        let lgray = gray.lighten(50%)
        let lblue = blue.lighten(75%)

        for (c, t, f) in (
          ((0, 0), "0", lgray),
          ((1 * dist, 1 * dist), "1", none),
          ((2 * dist, 1 * dist), "2", lblue),
          ((2 * dist, 0), "3", none),
          ((1 * dist, -1 * dist), "4", none),
        ) {
          circle(c, name: t, radius: 0.4, fill: f)
          content(t, [#t])
        }
        line("0","1")
        line("1","2",stroke: (dash: "dotted"))
        line("1","3")
        line("2","3",stroke: (dash: "dotted"))
        line("3","4")
        line("4","0")
      }),
      caption: [Problema TDPDP [9]]
    )
  )
)

== Modelo

#align(horizon,
[
El modelo nace de las instancias, en donde se encuentran los siguientes elementos: [3]
- $"c"$: Cantidad de clientes
- $"SL"$: Tiempo que tarda en lanzarse un dron
- $"SR"$: Tiempo que tarda en recuperarse un dron
- $"E"$: Resistencia del dron


Se incluyen los siguientes datos:
- $"Cdrones"$: Conjunto de clientes que pueden recibir pedidos por dron
- $tau_(t)$: Matriz que indica tiempo de viaje (camión) de $i$ a $j$
- $tau_(d)$: Matriz que indica tiempo de viaje (dron) de $i$ a $j$
- $"F"$: Conjunto de viajes validos para el dron ${i,j,k}$ [1]

#pagebreak()

Se tienen las restricciones: [2] [3]
- Se deben realizar la entrega a todos los clientes
- Un dron debe ser enviado en una posición $i$, ir a $j$ y encontrarse con el camión en $k$
- Los viajes de dron deben de pertener a $"F"$. Si pertenece a $"F"$, indirectamente cumple al condición de que se entrege a $"Cdrones"$.
- Condiciones fisicas (un camión solo puede ir de $i$ a $j$, no a varios clientes a la vez, el viaje del dron debe de soportar la distancia máxima de su resistencia $"E"$)

La función objetivo es minimzar el tiempo máximo donde el dron y el camión vuelven al deposito inicial.
]
)


== Implementación Representación TDPDP

#let tdpdp = [
```
struct TDPDP {
  ...
  Client c;
  Time SL;
  Time SR;
  Time E; 
  
  std::unordered_set<Client> Cdrones;
  
  std::vector<std::vector<Time>> tauTruck;
  std::vector<std::vector<Time>> tauDrone;
  
  int size_F; 
  std::vector<DronFlight> F;
  ...
```
]

#align(center+horizon,
  figure(
    tdpdp,
    caption: [Representación en C++]
  )
)


