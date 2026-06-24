// =============================================================================
//  mcpTest.cpp  –  Bring-up/Test: 2x MCP23017 + PCA9685 am selben I2C-Bus
// -----------------------------------------------------------------------------
//  Aktivieren in platformio.ini:   build_src_filter = +<mcpTest.cpp>
//  Dann:  pio run -t upload  &&  pio device monitor
//
//  Der Test:
//   - initialisiert beide MCP (ueber io.begin) + den PCA9685
//   - meldet, ob beide MCP am Bus antworten
//   - liest alle 16 Eingaenge (mit Pull-up) und gibt sie als Bitleiste aus
//   - laesst ein "Lauflicht" ueber die 16 Ausgaenge laufen (Transistoren)
//   - faehrt einen Servo an PCA9685-Kanal 0 langsam hin und her
//  Alles nicht-blockierend (millis), kein delay() in loop().
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "Io.h"
#include "Config.h"

Adafruit_PWMServoDriver pwm(ADDR_PCA9685);   // PCA9685 am selben Bus

void setup() {
  Serial.begin(115200);
  delay(300);

  // WICHTIG: io.begin() startet den I2C-Bus (Wire) -> VOR pwm.begin() aufrufen.
  io.begin(PIN_SDA, PIN_SCL, ADDR_MCP_IN, ADDR_MCP_OUT);
  Serial.printf("\n[mcpTest] MCP-IN(0x%02X)=%s   MCP-OUT(0x%02X)=%s\n",
                ADDR_MCP_IN,  io.mcpInOk()  ? "OK" : "FEHLT",
                ADDR_MCP_OUT, io.mcpOutOk() ? "OK" : "FEHLT");
  if (!io.mcpInOk() || !io.mcpOutOk())
    Serial.println("[mcpTest] WARNUNG: nicht beide MCP erreichbar - Adressen/Verkabelung pruefen!");

  // ── I2C-Bus scannen: zeigt, welche Adressen WIRKLICH antworten ──────────────
  //    (Wire wurde bereits von io.begin() gestartet.)
  Serial.println("[I2C] Scan laeuft...");
  uint8_t gefunden = 0;
  for (uint8_t a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.printf("[I2C]   gefunden: 0x%02X\n", a);
      gefunden++;
    }
  }
  Serial.printf("[I2C] %u Geraet(e) am Bus. Erwartet: MCP-IN 0x%02X, MCP-OUT 0x%02X, PCA 0x%02X\n",
                gefunden, ADDR_MCP_IN, ADDR_MCP_OUT, ADDR_PCA9685);

  pwm.begin();
  pwm.setPWMFreq(50);   // 50 Hz fuer Servos
  Serial.println("[mcpTest] PCA9685 bereit. Test laeuft.");
}

void loop() {
  uint32_t jetzt = millis();

  // ── Eingaenge alle 500 ms lesen (gedrueckt/aktiv = LOW -> als '1' anzeigen) ──
  static uint32_t tIn = 0;
  if (jetzt - tIn >= 500) {
    tIn = jetzt;
    Serial.print("[IN ] ");
    for (uint8_t p = 0; p < 16; p++) {
      Serial.print(io.digitalRead(mcpInPin(p)) == LOW ? '1' : '0');
      if (p == 7) Serial.print(' ');     // Trenner zwischen Port A und B
    }
    Serial.println();
  }

  // ── Lauflicht ueber die 16 Ausgaenge alle 200 ms ───────────────────────────
  static uint32_t tOut  = 0;
  static uint8_t  aktiv = 0;
  if (jetzt - tOut >= 200) {
    tOut = jetzt;
    io.digitalWrite(mcpOutPin(aktiv), LOW);     // bisherigen Ausgang aus
    aktiv = (aktiv + 1) % 16;
    io.digitalWrite(mcpOutPin(aktiv), HIGH);    // naechsten an (active-high)
  }

  // ── 4 Servos (PCA9685-Kanal 0..3) langsam UND in unterschiedliche Richtungen ─
  //    "Langsam" = viele kleine Schritte (alle 20 ms statt einem Sprung).
  //    "Unterschiedliche Richtungen" = benachbarte Servos starten gespiegelt und
  //    laufen gegenlaeufig; am Endanschlag dreht jeder Servo einzeln um.
  static uint32_t tServo = 0;
  static uint16_t servoPos[4] = { SERVO_TICK_ZU, SERVO_TICK_AUF, SERVO_TICK_ZU, SERVO_TICK_AUF };
  static int8_t   servoDir[4] = { +1, -1, +1, -1 };
  constexpr uint16_t SERVO_SCHRITT = 4;   // Ticks pro Schritt -> kleiner = langsamer
  if (jetzt - tServo >= 20) {
    tServo = jetzt;
    for (uint8_t s = 0; s < 4; s++) {
      int next = (int)servoPos[s] + servoDir[s] * SERVO_SCHRITT;
      if (next >= SERVO_TICK_AUF) { next = SERVO_TICK_AUF; servoDir[s] = -1; }
      if (next <= SERVO_TICK_ZU)  { next = SERVO_TICK_ZU;  servoDir[s] = +1; }
      servoPos[s] = (uint16_t)next;
      pwm.setPWM(s, 0, servoPos[s]);
    }
  }
}
