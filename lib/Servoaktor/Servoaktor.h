// =============================================================================
//  Servoaktor  –  servogetriebener Aktor am PCA9685
// -----------------------------------------------------------------------------
//  Generische Klasse fuer ALLE Servo-Aufgaben: Dachfenster, Jalousien,
//  Garagentor. Pro Objekt ein PCA9685-Kanal + die beiden Endlagen (Ticks).
//  Der PCA9685-Treiber wird von main bereitgestellt und per Referenz geteilt.
// =============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_PWMServoDriver.h>

class Servoaktor {
public:
  Servoaktor(Adafruit_PWMServoDriver& pwm, uint8_t kanal,
             uint16_t tickZu, uint16_t tickAuf);
  void begin();
  void auf();
  void zu();
  void setProzent(uint8_t prozent);    // 0 = zu, 100 = auf
  bool istAuf() const { return _prozent > 50; }

private:
  Adafruit_PWMServoDriver& _pwm;
  uint8_t  _kanal;
  uint16_t _tickZu, _tickAuf;
  uint8_t  _prozent = 0;
};
