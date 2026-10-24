#import "@preview/cetz:0.5.3"

== Definición TDPDP

#align(center + horizon,
  figure(
    image("assets/amazon-prime.jpg"),
    caption: [Entrega de Pedidos con Drones (Amazon Air)]
  )
)

== Representación Grafica TDPDP

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
      caption: [Problema TDPDP sin Asignación]
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
      caption: [Problema VRP (Con Parcel)]
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
      caption: [Problema TDPDP]
    )
  )
)

