#import "@preview/diatypst:0.9.3": *
#import "@preview/cetz:0.5.3"

// Configuración de colores institucionales (USM)
#let usm-blue = rgb("#003c71")
#let usm-gold = rgb("#f2a900")

#show: slides.with(
  title: "TDPDP: Pickup and Delivery Problem with Drone",
  subtitle: "Algoritmo Evolutivo Híbrido: Enfoque Truck-First, Drone-Second",
  date: "Octubre 2024",
  authors: ("Facundo Riquelme"),
  layout: "medium",
  ratio: 16/9,
  title-color: usm-blue,
  first-slide: false,
  toc: false,
  count: "dot",
  footer-title: context {
    if here().page() > 1 [
      #grid(
        columns: (auto, auto),
        gutter: 8pt,
        align: horizon,
        image("assets/logo_usm.pdf", height: 1.15em),
        text(weight: "bold", size: 0.85em, fill: usm-blue)[TDPDP | INF295 - Inteligencia Artificial]
      )
    ]
  },
  footer-subtitle: context {
    if here().page() > 1 {
      let page-num = here().page()
      let sections = query(selector(heading.where(level: 1)))
      let current-sec = sections.rev().find(x => x.location().page() <= page-num)
      let sec-title = if current-sec != none { current-sec.body } else [Presentación]
      grid(
        columns: (1fr, auto),
        gutter: 8pt,
        align: horizon,
        text(style: "italic", size: 0.8em, fill: luma(80))[#sec-title],
        text(weight: "bold", size: 0.85em, fill: usm-blue)[#page-num]
      )
    }
  }
)

// Desactivar numeración automática para títulos personalizados
#set heading(numbering: none)

// =========================================================================
// PORTADA
// =========================================================================

#align(top)[
  #grid(
    columns: (1fr, auto),
    align: (horizon + left, horizon + right),
    [
      #text(1.05em, weight: "bold", fill: usm-blue)[Universidad Técnica Federico Santa María] \
      #text(0.85em, fill: luma(90))[Departamento de Informática · Casa Central Valparaíso]
    ],
    image("assets/logo_usm.pdf", height: 1.6cm)
  )

  #v(0.3cm)
  #line(length: 100%, stroke: 1.5pt + usm-blue)
  #v(0.4cm)

  #text(1.7em, weight: "bold", fill: usm-blue)[Time-Dependent Pickup and Delivery Problem with Drone (TDPDP)] \
  #v(0.2cm)
  #text(1.15em, weight: "medium", fill: usm-blue.lighten(25%))[Algoritmo Evolutivo Híbrido: Enfoque Truck-First, Drone-Second]

  #v(1.0cm)
  #grid(
    columns: (1fr, 1fr),
    [
      #text(0.75em, fill: luma(100))[Autor:] \
      #text(1.0em, weight: "bold", fill: usm-blue)[Facundo Riquelme]
    ],
    align(right)[
      #text(0.75em, fill: luma(100))[Curso / Contexto:] \
      #text(1.0em, weight: "bold", fill: usm-blue)[INF295 Inteligencia Artificial]
    ]
  )
]

#pagebreak()

== Tabla de Contenidos

#v(0.2cm)
#columns(2, gutter: 20pt)[
  #outline(title: none, target: heading.where(level: 1), indent: auto)
]

= 1. Introducción y Contexto

#include "introduccion.typ"

= 2. Representación y Factibilidad

#include "representacion.typ"

= 3. Algoritmo Evolutivo Híbrido

#include "evolutivo.typ"

= 4. Arquitectura

#include "arquitectura.typ"

= 5. Resultados Preliminares

#include "resultados.typ"

= 6. Conclusiones y Trabajo Futuro

#include "conclusiones.typ"

= 7. Referencias y Anexos

#include "referencias.typ"
