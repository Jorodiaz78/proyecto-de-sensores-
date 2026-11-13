# Proyecto 8: Control de iluminación industrial según luz natural y presencia

Asignatura: Sensores y Actuadores, Corte 3
Integrantes: [nombre 1] y [nombre 2]

## Objetivo
Sistema de control de iluminación con ESP32 que mide la luz natural (BH1750) y detecta presencia (PIR HC-SR501) para encender, regular o apagar una tira LED de 12 V mediante un MOSFET con PWM (LEDC), usando una máquina de estados con millis().

## Materiales
- ESP32 DevKit V1
- Sensor de luz BH1750 (I2C)
- Sensor PIR HC-SR501
- MOSFET de canal N + resistencia de 1 kΩ
- Tira LED de 12 V y fuente externa de 12 V
- Módulo relé de 1 canal
- Protoboard, cables y multímetro

## Conexiones
| Componente | Señal | GPIO ESP32 |
|---|---|---|
| BH1750 | SDA | 21 |
| BH1750 | SCL | 22 |
| PIR | OUT | 27 |
| MOSFET | G (vía 1 kΩ) | 25 |

## Lógica de control
| Presencia | Luz natural | Acción |
|---|---|---|
| Sí | < 100 lux | LED encendido con duty proporcional |
| Sí | ≥ 100 lux | LED apagado |
| No | Insuficiente | LED apagado tras 30 s de gracia |
| No | Suficiente | LED apagado inmediato |

## Contenido del repositorio
- `codigo/`: código fuente (.ino)
- `diagramas/`: diagrama de conexiones y de flujo
- `fotos/`: fotos del montaje

## Calibración
[Aquí va la tabla con tus mediciones de lux y el umbral final]

## Video de la demostración
[Enlace al video]
