# Práctica 4 — Debounce

### ¿Es adecuado el control de los parámetros pasados por el usuario que se hace en las funciones implementadas? ¿Se controla que sean valores válidos? ¿Se controla que estén dentro de los rangos correctos?
Considero que sí. En esta pŕactica casi no hay parámetros de entrada adicionales.

### ¿Se nota una mejora en la detección de las pulsaciones respecto a la práctica 0? ¿Se pierden pulsaciones? ¿Hay falsos positivos?
No logré que se perdieran pulsaciones ni que ocurrieran falsos positivos. El pulsador es bastante sólido al tacto.

### ¿Es adecuada la temporización con la que se llama a debounceFSM_update()? ¿Y a readKey()? ¿Qué pasaría si se llamara con un tiempo mucho más grande? ¿Y mucho más corto?
Si, se están llamando en cada iteración del while principal tanto `debounceFSM_update` como `readKey`. 

- Si se llama con un periodo mucho más grande, la máquina de estados podría detectar tarde, o comportarse erráticamente.
- Si se llama con un período mucho más corto, debería mejorar la resolución (acotada por el CPU).
