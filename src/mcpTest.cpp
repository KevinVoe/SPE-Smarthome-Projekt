// =============================================================================
//  mcpTest.cpp  –  ROH-Bring-up: 2x MCP23017 + PCA9685 am selben I2C-Bus
// -----------------------------------------------------------------------------
//  Aktivieren in platformio.ini:   build_src_filter = +<mcpTest.cpp>
//  BEWUSST eigenstaendig: besitzt seine eigenen MCP-/PCA-Objekte (unabhaengig
//  von DigitalInput/DigitaleOutputs), um die Hardware roh zu pruefen:
//   - I2C-Scan (welche Adressen antworten?)
//   - alle 16 Eingaenge als Bitleiste
//   - Lauflicht ueber die 16 Ausgaenge
//   - 4 Servos langsam, gegenlaeufig
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_PWMServoDriver.h>
#include "Config.h"

Adafruit_MCP23X17       mcpIn;
Adafruit_MCP23X17       mcpOut;
Adafruit_PWMServoDriver pwm(ADDR_PCA9685);
bool inOk = false, outOk = false;

void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(PIN_SDA, PIN_SCL);
  inOk  = mcpIn.begin_I2C(ADDR_MCP_IN,  &Wire);
  outOk = mcpOut.begin_I2C(ADDR_MCP_OUT, &Wire);
  if (inOk)  for (uint8_t p = 0; p < 16; p++) mcpIn.pinMode(p, INPUT_PULLUP);
  if (outOk) for (uint8_t p = 0; p < 16; p++) { mcpOut.pinMode(p, OUTPUT); mcpOut.digitalWrite(p, LOW); }

  Serial.printf("\n[mcpTest] MCP-IN(0x%02X)=%s  MCP-OUT(0x%02X)=%s\n",
                ADDR_MCP_IN, inOk ? "OK" : "FEHLT", ADDR_MCP_OUT, outOk ? "OK" : "FEHLT");

  Serial.println("[I2C] Scan:");
  uint8_t n = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) { Serial.printf("  0x%02X\n", a); n++; }
  }
  Serial.printf("[I2C] %u Geraet(e). Erwartet: IN 0x%02X, OUT 0x%02X, PCA 0x%02X\n",
                n, ADDR_MCP_IN, ADDR_MCP_OUT, ADDR_PCA9685);

  pwm.begin();
  pwm.setPWMFreq(50);
  Serial.println("[mcpTest] bereit.");
}

void loop() {
  uint32_t jetzt = millis();

  // Eingaenge alle 500 ms (aktiv/gedrueckt = LOW -> '1')
  static uint32_t tIn = 0;
  if (jetzt - tIn >= 500) {
    tIn = jetzt;
    Serial.print("[IN ] ");
    for (uint8_t p = 0; p < 16; p++) {
      Serial.print(inOk && mcpIn.digitalRead(p) == LOW ? '1' : '0');
      if (p == 7) Serial.print(' ');
    }
    Serial.println();
  }

  // Lauflicht ueber die 16 Ausgaenge alle 200 ms
  static uint32_t tOut = 0;
  static uint8_t  aktiv = 0;
  if (jetzt - tOut >= 200) {
    tOut = jetzt;
    if (outOk) {
      mcpOut.digitalWrite(aktiv, LOW);
      aktiv = (aktiv + 1) % 16;
      mcpOut.digitalWrite(aktiv, HIGH);
    }
  }

  // 4 Servos langsam und gegenlaeufig
  static uint32_t tServo = 0;
  static uint16_t pos[4] = { SERVO_TICK_ZU, SERVO_TICK_AUF, SERVO_TICK_ZU, SERVO_TICK_AUF };
  static int8_t   dir[4] = { +1, -1, +1, -1 };
  constexpr uint16_t SCHRITT = 4;
  if (jetzt - tServo >= 20) {
    tServo = jetzt;
    for (uint8_t s = 0; s < 4; s++) {
      int next = (int)pos[s] + dir[s] * SCHRITT;
      if (next >= SERVO_TICK_AUF) { next = SERVO_TICK_AUF; dir[s] = -1; }
      if (next <= SERVO_TICK_ZU)  { next = SERVO_TICK_ZU;  dir[s] = +1; }
      pos[s] = (uint16_t)next;
      pwm.setPWM(s, 0, pos[s]);
    }
  }
}
