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
