# Práctica 3 — Modularización (retardos no bloqueantes)

## Preguntas de reflexión

### ¿Es suficientemente clara la consigna 2 o da lugar a implementaciones con distinto comportamiento? 

La consigna 2 deja algo ambigua la interpretación de los tiempos de encendido y el duty cycle del 50%. Podría interpretarse que los tiempos representan el tiempo que el led tiene que estar encendido, y en consecuencia el que tienen que estar apagado (por ser 50%), o bien que los tiempos de encendido y apagado son el 50% de los valores del array de tiempos, es decir {250, 50, 50, 500}. En mi caso, utilize la primera interpretación.

---
### ¿Se puede cambiar el tiempo de encendido del led fácilmente en un solo lugar del código o éste está hardcodeado? ¿Hay números “mágicos” en el código?

Si, solo bastaría editar el arreglo de `TIEMPOS`. No considero que hayan quedado magic numbers en el código.

---
### ¿Qué bibliotecas estándar se debieron agregar a API_delay.h para que el código compile? Si las funcionalidades de una API propia crecieran, habría que pensar cuál sería el mejor lugar para incluir esas bibliotecas y algunos typedefs que se usen en la implementación, ¿Cuál sería el mejor lugar?.

En `API_delay.h` se tuvo que incluir:

- `<stdint.h>`: para tipos `uint32_t`
- `<stdbool.h>`: para `bool`

---
### ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos? ¿Se controla que estén dentro de los rangos esperados?

Considero que si. En las funciones que reciben un puntero se corrobora que el puntero no sea `NULL`, y en las funciones `delayInit` y `delayWrite` controlo que la duración provista no sea 0 (ya que la variable es de tipo `uint32_t`).
