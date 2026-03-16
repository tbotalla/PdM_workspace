#### Practica 2
La resolución de la práctica se encuentra dentro de `Core/Src/main.c` y `Core/Inc/main.h`.
Las diferentes implementaciones del punto 2 y punto 3 se encuentran separadas entre comentarios del tipo:
```
/* Inicio Punto 2 */
/* Fin Punto 2 */
/* Inicio Punto 3 */
/* Fin Punto 3 */
```

##### 1. ¿Se pueden cambiar los tiempos de encendido fácilmente en un solo lugar o están hardcodeados?
Sí, se pueden cambiar en un solo lugar. Los tiempos no están repartidos por el código: están todos en el arreglo `tick_patterns[]` en `main.c`. 
Cada entrada tiene `period_ms`, `count` y `duty_cycle`. Para cambiar períodos, cantidad de parpadeos o ciclo de trabajo basta con editar ese arreglo. 

##### 2. ¿Qué bibliotecas estándar se debieron agregar para que compile? ¿Dónde convendría ponerlas y los typedefs si el código crece?
Agregadas explícitamente: en `main.h` solo está `#include <stdbool.h>` (para `bool_t`).
Convendría ponerlas en un único lugar común, por ejemplo en `main.h` (o en un header de tipos del proyecto) y que el resto de archivos incluya ese header en lugar de repetir includes.

##### 3. ¿Es adecuado el control de los parámetros? ¿Se comprueba que sean válidos y estén en rango?
Considero que es aceptable. Se valida que el puntero `delay_t*` no sea nulo y se validan que los valores de `duration` sean positivos.

##### 4. ¿Cuán reutilizable es el código implementado?
No demasiado. Para que sea reutilizable habría que extraer la lógica principal a una función que reciba todos los parámetros necesarios en vez de tener todo en `main.c`.

##### 5. ¿Cuán sencillo es cambiar el patrón de tiempos de parpadeo?
Fácil, basta con editar el arreglo `tick_patterns[]`: agregar o quitar filas, o cambiar `period_ms`, `count` o `duty_cycle`. 
No hace falta tocar el while ni recalcular manualmente el número de patrones porque `total_tick_patterns` se deriva del tamaño del arreglo.
