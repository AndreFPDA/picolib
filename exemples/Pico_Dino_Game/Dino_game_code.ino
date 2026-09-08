#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// --- User config: change pins to match your wiring ---
#define TOUCH_SENSOR_PIN 5  // now used as the push button (jump) pin
#define BUZZER_PIN 12
// ----------------------------------------------------

#include "t-rex-duino.h"

void setup() {
  Serial.begin(9600);
  Serial.println("=== DINO GAME - BUTTON BUILD v3 - GPIO5 ===");
  Wire.begin(21, 22);
  pinMode(TOUCH_SENSOR_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
}

unsigned long _lastDebugPrint = 0;

void loop() {
  runDinoGame();
}