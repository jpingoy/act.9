#include <Arduino.h>

// Pin analogico donde esta conectado el punto medio del divisor con el LDR.
constexpr uint8_t LDR_PIN = 14;

// Pines de los LEDs, ordenados desde el primero hasta el quinto indicador.
constexpr uint8_t LED_PINS[] = {15, 2, 4, 5, 18};
constexpr uint8_t LED_COUNT = sizeof(LED_PINS) / sizeof(LED_PINS[0]);

// Duracion de cada medicion de calibracion y tiempo entre muestras del ADC.
constexpr uint16_t CALIBRATION_TIME_MS = 5000;
constexpr uint16_t SAMPLE_INTERVAL_MS = 25;

// Valores de referencia obtenidos durante la calibracion.
int brightReference = 0;
int ambientReference = 0;

// Obtiene el promedio de varias lecturas para reducir las fluctuaciones del LDR.
int readAverage(uint16_t durationMs) {
  uint32_t sum = 0;
  uint32_t samples = 0;
  uint32_t start = millis();

  while (millis() - start < durationMs) {
    sum += analogRead(LDR_PIN);
    ++samples;
    delay(SAMPLE_INTERVAL_MS);
  }

  return samples == 0 ? analogRead(LDR_PIN) : sum / samples;
}

void turnOffLeds() {
  // Apaga todos los LEDs antes de iniciar una nueva medicion o calibracion.
  for (uint8_t ledPin : LED_PINS) {
    digitalWrite(ledPin, LOW);
  }
}

void calibrateSensor() {
  // Primera referencia: fuente de luz colocada aproximadamente a 30 cm.
  Serial.println();
  Serial.println("Calibracion del LDR");
  Serial.println("Coloca la fuente de luz a unos 30 cm del LDR.");
  Serial.println("Iniciando en 3 segundos...");
  delay(3000);

  Serial.println("Midiendo referencia de luz maxima...");
  brightReference = readAverage(CALIBRATION_TIME_MS);

  // Segunda referencia: luz ambiental sin la fuente cerca del sensor.
  Serial.println("Retira la fuente de luz y deja solo la iluminacion ambiental.");
  Serial.println("Iniciando segunda medicion en 3 segundos...");
  delay(3000);

  Serial.println("Midiendo referencia ambiental...");
  ambientReference = readAverage(CALIBRATION_TIME_MS);

  Serial.print("Referencia a 30 cm: ");
  Serial.println(brightReference);
  Serial.print("Referencia ambiental: ");
  Serial.println(ambientReference);
  Serial.println("Calibracion terminada.");
}

void updateLeds() {
  // Convierte la lectura actual en un porcentaje entre las dos referencias.
  int reading = analogRead(LDR_PIN);
  int referenceRange = brightReference - ambientReference;

  // Evita una division invalida si ambas mediciones de calibracion son iguales.
  if (referenceRange == 0) {
    turnOffLeds();
    return;
  }

  int brightnessPercent = ((reading - ambientReference) * 100) / referenceRange;
  // Limita el resultado al rango valido aunque la lectura salga de las referencias.
  brightnessPercent = constrain(brightnessPercent, 0, 100);

  // Cada nivel representa un quinto de la intensidad calibrada.
  uint8_t ledsOn = (brightnessPercent * LED_COUNT + 99) / 100;

  for (uint8_t index = 0; index < LED_COUNT; ++index) {
    digitalWrite(LED_PINS[index], index < ledsOn ? HIGH : LOW);
  }
}

void setup() {
  // Abre el monitor serial para mostrar el proceso y los resultados de calibracion.
  Serial.begin(115200);

  // Usa el ADC de 12 bits (0 a 4095) y el rango de entrada mas amplio del ESP32.
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  // Configura los cinco pines como salidas digitales y comienza con los LEDs apagados.
  for (uint8_t ledPin : LED_PINS) {
    pinMode(ledPin, OUTPUT);
  }
  turnOffLeds();

  calibrateSensor();
}

void loop() {
  // Actualiza el indicador de intensidad aproximadamente diez veces por segundo.
  updateLeds();
  delay(100);
}

