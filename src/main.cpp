#include <Arduino.h>

constexpr uint8_t LDR_PIN = 14;
constexpr uint8_t LED_PINS[] = {15, 2, 4, 5, 18};
constexpr uint8_t LED_COUNT = sizeof(LED_PINS) / sizeof(LED_PINS[0]);
constexpr uint16_t CALIBRATION_TIME_MS = 5000;
constexpr uint16_t SAMPLE_INTERVAL_MS = 25;

int brightReference = 0;
int ambientReference = 0;

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
  for (uint8_t ledPin : LED_PINS) {
    digitalWrite(ledPin, LOW);
  }
}

void calibrateSensor() {
  Serial.println();
  Serial.println("Calibracion del LDR");
  Serial.println("Coloca la fuente de luz a unos 30 cm del LDR.");
  Serial.println("Iniciando en 3 segundos...");
  delay(3000);

  Serial.println("Midiendo referencia de luz maxima...");
  brightReference = readAverage(CALIBRATION_TIME_MS);

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
  int reading = analogRead(LDR_PIN);
  int referenceRange = brightReference - ambientReference;

  if (referenceRange == 0) {
    turnOffLeds();
    return;
  }

  int brightnessPercent = ((reading - ambientReference) * 100) / referenceRange;
  brightnessPercent = constrain(brightnessPercent, 0, 100);
  uint8_t ledsOn = (brightnessPercent * LED_COUNT + 99) / 100;

  for (uint8_t index = 0; index < LED_COUNT; ++index) {
    digitalWrite(LED_PINS[index], index < ledsOn ? HIGH : LOW);
  }
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(LDR_PIN, ADC_11db);

  for (uint8_t ledPin : LED_PINS) {
    pinMode(ledPin, OUTPUT);
  }
  turnOffLeds();

  calibrateSensor();
}

void loop() {
  updateLeds();
  delay(100);
}

