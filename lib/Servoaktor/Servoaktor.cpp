#include "Servoaktor.h"

Servoaktor::Servoaktor(Adafruit_PWMServoDriver& pwm, uint8_t kanal,
                       uint16_t tickZu, uint16_t tickAuf)
  : _pwm(pwm), _kanal(kanal), _tickZu(tickZu), _tickAuf(tickAuf) {}

void Servoaktor::begin() { zu(); }     // definierte Startlage: geschlossen

void Servoaktor::auf() { setProzent(100); }
void Servoaktor::zu()  { setProzent(0); }

void Servoaktor::setProzent(uint8_t prozent) {
  if (prozent > 100) prozent = 100;
  _prozent = prozent;
  // linear zwischen den Endlagen interpolieren (int-Mathematik, damit auch
  // tickAuf < tickZu erlaubt ist, falls der Servo "andersherum" eingebaut ist).
  int spanne = (int)_tickAuf - (int)_tickZu;
  uint16_t tick = (uint16_t)(_tickZu + spanne * (int)prozent / 100);
  _pwm.setPWM(_kanal, 0, tick);
}
