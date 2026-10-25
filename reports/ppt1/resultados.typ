#import "@preview/cetz:0.5.3"
#import "@preview/codly:2.0.0" as codly
#show raw.where(block: true): codly.new

== Lectura de Datos

Se puede realizar lectura de las instancias iniciales

#align(center+horizon,
  figure(
    image("assets/result1.png", width: 80%),
    caption: [Lectura de Datos Instancia 1]
  )
)

== Obtención de Solución Inicial

Se genera una solución inicial (mediante TSP) utilizando una secuencia aleatoria

#align(center+horizon,
  figure(
    image("assets/result2.png", width: 80%),
    caption: [Generar Instancia Inicial (TSP)]
  )
)


== Evaluación

Se utiliza el greedy para obtener la instancia de los drones. También se calcula el fitness de la función (calculado por el propio greedy)

#align(center+horizon,
  figure(
    image("assets/result3.png", width: 80%),
    caption: [Resultado Greedy + Fitness]
  )
)

#pagebreak()

#let lgray = gray.lighten(50%)
#let lblue = blue.lighten(75%)

#align(center,
  figure(
    cetz.canvas({
      import cetz.draw: *
      let dist = 1.5
      let tsp = ("0", "7", "5", "3", "4", "8", "9", "10", "1", "6", "2")

      for i in range(tsp.len()) {
        let t = tsp.at(i)
        let x = i * dist
        let f = if t == "0" { lgray } else { none }
        circle((x, 0), radius: 0.35, fill: f, name: "tsp_" + str(i))
        content((x, 0), [#t])
      }

      for i in range(tsp.len() - 1) {
        line("tsp_" + str(i), "tsp_" + str(i+1), mark: (end: ">"))
      }
    }),
    caption: [Problema TSP - Test 3 (Secuencia lineal)]
  )
)

#v(20pt) 

#align(center,
  figure(
    cetz.canvas({
      import cetz.draw: *

      let coords = (
        // Eje central (Camión: y = 0)
        "0": (0.0, 0),
        "5": (1.5, 0),
        "3": (3.0, 0),
        "4": (4.5, 0),
        "8": (6.0, 0),
        "10": (7.5, 0),
        "1": (9.0, 0),
        "6": (10.5, 0),
        "11": (12.0, 0),
        
        "7": (1.5, 1.5),   // Arriba (para tándem {0, 7, 3})
        "9": (5.25, 1.5),  // Arriba (para tándem {4, 9, 10})
        "2": (9.75, -1.5)  // Abajo (para tándem {1, 2, 11})
      )

      let drone-nodes = ("0")
      let true-drone-nodes = ("7","9","2")

      for (t, c) in coords {
        let f = if drone-nodes.contains(t) { lgray } else { none }
        let f = if true-drone-nodes.contains(t) { lblue } else { none }
        circle(c, name: t, radius: 0.35, fill: f)
        content(t, [#t])
      }

      let truck = ("0", "5", "3", "4", "8", "10", "1", "6", "11")
      for i in range(truck.len() - 1) {
        line(truck.at(i), truck.at(i+1), mark: (end: ">"))
      }

      let drones = (
        ("0", "7", "3"),
        ("4", "9", "10"),
        ("1", "2", "11")
      )
      for path in drones {
        for i in range(path.len() - 1) {
          line(path.at(i), path.at(i+1), stroke: (dash: "dotted"), mark: (end: ">"))
        }
      }
    }),
    caption: [Problema TDPDP - Test 3 (Camión y Drones en ejes separados)]
  )
)
