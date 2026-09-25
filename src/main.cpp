#include <Arduino.h>

// Clase para gestionar el arreglo de LEDs como un indicador.
class LedBar {
  private:
    const uint8_t* pins;
    uint8_t count;

  public:
    LedBar(const uint8_t* pinArray, uint8_t pinCount) {
      pins = pinArray;
      count = pinCount;
    }

    void begin() {
      for (uint8_t i = 0; i < count; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
      }
    }

    void turnOffAll() {
      for (uint8_t i = 0; i < count; i++) {
        digitalWrite(pins[i], LOW);
      }
    }

    // Enciende los LEDs proporcionalmente según el nivel recibido (0 a 100%)
    void displayLevel(int percentage) {
      // Cálculo claro del número de LEDs que deben encenderse
      uint8_t ledsToTurnOn = 0;

      if (percentage >= 80) {
        ledsToTurnOn = 5;
      } else if (percentage >= 60) {
        ledsToTurnOn = 4;
      } else if (percentage >= 40) {
        ledsToTurnOn = 3;
      } else if (percentage >= 20) {
        ledsToTurnOn = 2;
      } else if (percentage > 0) {
        ledsToTurnOn = 1;
      } else {
        ledsToTurnOn = 0;
      }

      for (uint8_t i = 0; i < count; i++) {
        if (i < ledsToTurnOn) {
          digitalWrite(pins[i], HIGH);
        } else {
          digitalWrite(pins[i], LOW);
        }
      }
    }
};

// Clase para gestionar el sensor LDR y su calibración.
class LightSensor {
  private:
    uint8_t pin;
    uint16_t sampleIntervalMs;
    int brightReference;
    int ambientReference;

  public:
    LightSensor(uint8_t analogPin, uint16_t sampleInterval = 25) {
      pin = analogPin;
      sampleIntervalMs = sampleInterval;
      brightReference = 0;
      ambientReference = 0;
    }

    void begin() {
      analogReadResolution(12);
      analogSetPinAttenuation(pin, ADC_11db);
    }

    int readAverage(uint16_t durationMs) {
      uint32_t sum = 0;
      uint32_t samples = 0;
      uint32_t start = millis();

      while (millis() - start < durationMs) {
        sum = sum + analogRead(pin);
        samples = samples + 1;
        delay(sampleIntervalMs);
      }

      if (samples == 0) {
        return analogRead(pin);
      }

      return sum / samples;
    }

    void calibrate(uint16_t calibrationTimeMs) {
      Serial.println();
      Serial.println("--- Calibracion del LDR ---");
      Serial.println("Coloca la fuente de luz a unos 30 cm del LDR.");
      Serial.println("Iniciando en 3 segundos...");
      delay(3000);

      Serial.println("Midiendo referencia de luz maxima...");
      brightReference = readAverage(calibrationTimeMs);

      Serial.println("Retira la fuente de luz y deja solo la iluminacion ambiental.");
      Serial.println("Iniciando segunda medicion en 3 segundos...");
      delay(3000);

      Serial.println("Midiendo referencia ambiental...");
      ambientReference = readAverage(calibrationTimeMs);

      Serial.print("Referencia a 30 cm: ");
      Serial.println(brightReference);
      Serial.print("Referencia ambiental: ");
      Serial.println(ambientReference);
      Serial.println("Calibracion terminada.");
    }

    int getBrightnessPercentage() {
      int reading = analogRead(pin);
      int referenceRange = brightReference - ambientReference;

      Serial.print("Lectura actual: ");
      Serial.println(reading);
      Serial.print("Rango de referencia: ");
      Serial.println(referenceRange);

      if (referenceRange == 0) {
        return 0;
      }

      int percentage = ((reading - ambientReference) * 100) / referenceRange;

      // Asegura que el valor se mantenga dentro del rango 0 - 100%
      return constrain(percentage, 0, 100);
    }
};

// --- Configuración de Hardware ---
constexpr uint8_t LDR_PIN = 14;
constexpr uint8_t LED_PINS[] = {15, 2, 4, 5, 18};
constexpr uint8_t LED_COUNT = sizeof(LED_PINS) / sizeof(LED_PINS[0]);
constexpr uint16_t CALIBRATION_TIME_MS = 5000;

// Instancias globales de las clases
LedBar bar (LED_PINS, LED_COUNT);
LightSensor sensor(LDR_PIN);

void setup() {
  Serial.begin(115200);

  bar.begin();
  sensor.begin();

  sensor.calibrate(CALIBRATION_TIME_MS);
}

void loop() {
  int brightness = sensor.getBrightnessPercentage();
  bar.displayLevel(brightness);
  delay(100);
}
