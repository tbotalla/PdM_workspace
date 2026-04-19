# Trabajo final — monitor térmico (STM32F446RE + HAL)

Firmware para un **sistema de vigilancia de temperatura** con alarma local y consola por UART.

## Qué hace

- Lee **temperatura** por **I2C** desde un sensor **BME280** (solo se usa la lectura compensada en °C).
- Ejecuta una **máquina de estados** (`api_thermal`): umbrales, histéresis, tiempo máximo de alarma y recuperación ante
  fallo del sensor.
- Activa un **buzzer** en GPIO cuando la temperatura supera el umbral; lo apaga al bajar por histéresis o al vencer el
  tiempo de alarma.
- Expone una **CLI por USART2**: consulta de estado, parámetros, reporte periódico opcional, etc.

## Hardware / periféricos

| Recurso    | Uso                     |
|------------|-------------------------|
| **I2C1**   | Comunicación con BME280 |
| **USART2** | Consola / comandos      |
| **GPIO**   | Buzzer                  |

MCU: **STM32F446RE** con **STM32 HAL** (proyecto generado desde `trabajo-final.ioc`).

## Módulos de aplicación (carpeta `Drivers/`)

| Módulo               | Rol                                                       |
|----------------------|-----------------------------------------------------------|
| **`Drivers/API`**    | FSM térmica: umbrales, alarma, timeout, estado del sensor |
| **`Drivers/BME280`** | Inicialización y lectura de temperatura vía I2C           |
| **`Drivers/BUZZER`** | Control del zumbador                                      |
| **`Drivers/UART`**   | Inicialización UART, CLI y reportes periódicos            |

El bucle principal (`Core/Src/main.c`) llama en cada vuelta a `thermal_fsm_update(HAL_GetTick())` y `uart_cli_poll()`.

## Consola UART (resumen)

Comandos típicos: `HELP`, `STATUS`, `SET` / `GET` para umbrales e histéresis, `REPORT ON` / `REPORT OFF` para el envío
periódico de líneas de temperatura/estado. Detalle exacto: mensaje de ayuda al conectar o con `HELP`.

## Estructura del proyecto

- **`Core/`** — `main`, `stm32f4xx_hal_msp`, interrupciones, configuración de reloj.
- **`Drivers/STM32F4xx_HAL_Driver/`** y **`Drivers/CMSIS/`** — HAL y capa CMSIS de ST.

Compilación: proyecto **STM32CubeIDE** (`.cproject`, `trabajo-final.ioc`).
