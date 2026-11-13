/*
 * PROYECTO 8 - FASE 4
 * Integración BH1750 + PIR + Máquina de Estados
 * Plataforma: ESP32 DevKit V1
 *
 * Conexiones:
 * - BH1750: SDA=21, SCL=22, VCC=3.3V, GND=GND, ADDR=GND
 * - PIR:    OUT=27, VCC=5V (VIN), GND=GND
 * - MOSFET: G=GPIO25 (vía 1kΩ), S=GND, D=negativo tira LED
 *
 * Lógica:
 * - Presencia + oscuridad  -> LED ON con duty proporcional
 * - Presencia + luz OK     -> LED OFF
 * - Ausencia (oscuridad)   -> LED OFF tras 30 s de gracia
 * - Ausencia + luz OK      -> LED OFF inmediato
 */

#include <Wire.h>
#include <BH1750.h>

// ─────────────── Constantes de configuración ───────────────
#define PIN_MOSFET      25
#define PIN_PIR         27
#define LUX_OSCURO      100    // Umbral de oscuridad (lux)
#define LUX_MIN_REG     50     // Inicio de regulación proporcional
#define LUX_MAX_REG     300    // Fin de regulación proporcional
#define TIEMPO_GRACIA   30000  // 30 segundos en milisegundos

// ─────────────── Estados de la FSM ───────────────
enum Estado {
  IDLE,        // Sin presencia, LED apagado
  ACTIVO,      // Presencia + oscuridad, LED modulado
  GRACIA,      // Sin presencia pero LED aún encendido (temporizador)
  LUZ_OK       // Luz natural suficiente, LED apagado
};

// ─────────────── Variables globales ───────────────
Estado estadoActual = IDLE;
unsigned long tiempoInicioGracia = 0;
unsigned long ultimaLectura = 0;
const unsigned long INTERVALO_LECTURA = 500; // ms

BH1750 lightMeter;

// ─────────────── Prototipos ───────────────
int calcularDuty(float lux);
const char* nombreEstado(Estado e);

// ─────────────── Setup ───────────────
void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);

  Wire.begin(21, 22);
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("ERROR: BH1750 no detectado.");
    while (1) { delay(100); }
  }

  ledcAttach(PIN_MOSFET, 5000, 8);  // 5 kHz, 8 bits
  ledcWrite(PIN_MOSFET, 0);

  Serial.println("Sistema FASE 4 inicializado.");
  Serial.println("Estado\tPIR\tLux\tDuty\tEvento");
}

// ─────────────── Loop principal ───────────────
void loop() {
  unsigned long ahora = millis();

  // Lectura no bloqueante cada INTERVALO_LECTURA ms
  if (ahora - ultimaLectura >= INTERVALO_LECTURA) {
    ultimaLectura = ahora;

    bool presencia = digitalRead(PIN_PIR);
    float lux = lightMeter.readLightLevel();
    int duty = 0;
    const char* evento = "";

    // ─── Máquina de estados ───
    switch (estadoActual) {

      case IDLE:
        if (presencia && lux < LUX_OSCURO) {
          estadoActual = ACTIVO;
          evento = "IDLE->ACTIVO (presencia+oscuridad)";
        } else if (lux >= LUX_OSCURO) {
          estadoActual = LUZ_OK;
          evento = "IDLE->LUZ_OK (luz suficiente)";
        }
        break;

      case ACTIVO:
        duty = calcularDuty(lux);
        if (lux >= LUX_OSCURO) {
          estadoActual = LUZ_OK;
          duty = 0;
          evento = "ACTIVO->LUZ_OK (luz suficiente)";
        } else if (!presencia) {
          estadoActual = GRACIA;
          tiempoInicioGracia = ahora;
          evento = "ACTIVO->GRACIA (sin presencia)";
        }
        break;

      case GRACIA:
        duty = calcularDuty(lux);
        if (presencia) {
          estadoActual = ACTIVO;
          evento = "GRACIA->ACTIVO (presencia reaparece)";
        } else if (lux >= LUX_OSCURO) {
          estadoActual = LUZ_OK;
          duty = 0;
          evento = "GRACIA->LUZ_OK (luz suficiente)";
        } else if (ahora - tiempoInicioGracia >= TIEMPO_GRACIA) {
          estadoActual = IDLE;
          duty = 0;
          evento = "GRACIA->IDLE (tiempo agotado)";
        }
        break;

      case LUZ_OK:
        duty = 0;
        if (presencia && lux < LUX_OSCURO) {
          estadoActual = ACTIVO;
          evento = "LUZ_OK->ACTIVO (presencia+oscuridad)";
        }
        break;
    }

    // ─── Aplicar duty al MOSFET ───
    if (estadoActual == ACTIVO || estadoActual == GRACIA) {
      ledcWrite(PIN_MOSFET, duty);
    } else {
      ledcWrite(PIN_MOSFET, 0);
    }

    // ─── Log en monitor serial ───
    Serial.print(nombreEstado(estadoActual));
    Serial.print("\t");
    Serial.print(presencia ? "1" : "0");
    Serial.print("\t");
    Serial.print(lux, 1);
    Serial.print("\t");
    Serial.print(duty);
    Serial.print("\t");
    Serial.println(evento);
  }
}

// ─────────────── Función de cálculo de duty ───────────────
int calcularDuty(float lux) {
  if (lux < LUX_MIN_REG) return 255;
  if (lux > LUX_MAX_REG) return 0;
  return map((int)lux, LUX_MIN_REG, LUX_MAX_REG, 255, 0);
}

// ─────────────── Conversión de estado a texto ───────────────
const char* nombreEstado(Estado e) {
  switch (e) {
    case IDLE:   return "IDLE  ";
    case ACTIVO: return "ACTIVO";
    case GRACIA: return "GRACIA";
    case LUZ_OK: return "LUZ_OK";
  }
  return "?????";
}
