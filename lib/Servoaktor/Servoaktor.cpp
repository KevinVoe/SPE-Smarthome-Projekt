#include "Servoaktor.h"
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

namespace {
  Adafruit_PWMServoDriver pwm(ADDR_PCA9685);   // eigener PCA9685-Treiber
  bool gServoOk = false;                        // PCA9685 am Bus erreichbar?

  // Interne Servo-Indizes:
  //   0..5 = Jalousie (etage*2 + seite)
  //   6..7 = Dachfenster (seite)
  //   8    = Garage
  constexpr uint8_t ANZ_SERVOS = 9;
  constexpr uint16_t SCHRITT   = 4;            // Ticks pro Update -> Tempo (kleiner = langsamer)

  uint8_t  gKanal[ANZ_SERVOS];
  uint16_t gIst[ANZ_SERVOS];
  uint16_t gZiel[ANZ_SERVOS];

  void setZiel(uint8_t idx, const ServoEndlage& s, Position p) {
    gKanal[idx] = s.kanal;
    gZiel[idx]  = (p == Position::AUF) ? s.tickAuf : s.tickZu;
  }
}

void servosBegin() {
  pwm.begin();
  Wire.beginTransmission(ADDR_PCA9685);          // Praesenz-Check (nur Diagnose)
  gServoOk = (Wire.endTransmission() == 0);
  pwm.setPWMFreq(50);   // 50 Hz fuer Servos

  // Kanaele + Startlage (ZU) aus Config uebernehmen.
  for (uint8_t e = 0; e < 3; e++)
    for (uint8_t s = 0; s < 2; s++) {
      uint8_t idx = e * 2 + s;
      gKanal[idx] = SERVO_JALOUSIE[e][s].kanal;
      gIst[idx] = gZiel[idx] = SERVO_JALOUSIE[e][s].tickZu;
    }
  for (uint8_t s = 0; s < 2; s++) {
    gKanal[6 + s] = SERVO_DACHFENSTER[s].kanal;
    gIst[6 + s] = gZiel[6 + s] = SERVO_DACHFENSTER[s].tickZu;
  }
  gKanal[8] = SERVO_GARAGE.kanal;
  gIst[8] = gZiel[8] = SERVO_GARAGE.tickZu;

  for (uint8_t i = 0; i < ANZ_SERVOS; i++) pwm.setPWM(gKanal[i], 0, gIst[i]);
}

void servosUpdate() {
  static uint32_t letzt = 0;
  if (millis() - letzt < 20) return;          // ~50 Schritte/s
  letzt = millis();

  for (uint8_t i = 0; i < ANZ_SERVOS; i++) {
    if (gIst[i] == gZiel[i]) continue;        // angekommen -> nichts senden
    int diff    = (int)gZiel[i] - (int)gIst[i];
    int schritt = (abs(diff) < (int)SCHRITT) ? diff : (diff > 0 ? (int)SCHRITT : -(int)SCHRITT);
    gIst[i]     = (uint16_t)((int)gIst[i] + schritt);
    pwm.setPWM(gKanal[i], 0, gIst[i]);
  }
}

void fahreJalousie(Position p, Etage etage, Seite seite) {
  uint8_t e = (uint8_t)etage, s = (uint8_t)seite;
  setZiel(e * 2 + s, SERVO_JALOUSIE[e][s], p);
}

void fahreDachfenster(Position p, Seite seite) {
  uint8_t s = (uint8_t)seite;
  setZiel(6 + s, SERVO_DACHFENSTER[s], p);
}

void fahreGarage(Position p) {
  setZiel(8, SERVO_GARAGE, p);
}

bool servosOk() { return gServoOk; }
