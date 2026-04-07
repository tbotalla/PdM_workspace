# Práctica 5 — UART + Command Parser

### Punto 1
Se implementó la capa `API_uart` con inicialización de USART2 y funciones básicas de envío/recepción en polling:

- `uartInit()`
- `uartSendString()`
- `uartSendStringSize()`
- `uartReceiveStringSize()`

Además, en la inicialización se imprime un mensaje con la configuración de la UART.

### Punto 2
Se implementó `API_cmdparser` con una máquina de estados para recibir comandos por UART:

- Lectura carácter a carácter.
- Armado de línea hasta `\r` o `\n`.
- Tokenización y validación básica.
- Ejecución de comandos obligatorios.

### Comandos obligatorios soportados
- `HELP`
- `LED ON`
- `LED OFF`
- `LED TOGGLE`
- `STATUS`

### Manejo de errores
Se reportan errores simples por UART:

- `ERROR: line too long`
- `ERROR: unknown command`
- `ERROR: bad arguments`
