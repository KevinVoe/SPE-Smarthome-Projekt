#include "Licht.h"

Licht::Licht(const int pins[LICHT_MAX_KANAELE]) {
  for (uint8_t i = 0; i < LICHT_MAX_KANAELE; i++) {
    _pins[i]   = pins[i];
    _stufen[i] = 0;
  }
}

void Licht::begin() {
  for (uint8_t i = 0; i < LICHT_MAX_KANAELE; i++) {
    pinMode(_pins[i], OUTPUT);
    analogWrite(_pins[i], 0);              // sicher aus beim Start
  }
}

void Licht::setKanal(uint8_t kanal, uint8_t stufe) {
  if (kanal >= LICHT_MAX_KANAELE) return;  // ungueltigen Kanal ignorieren
  if (stufe > 3)           stufe = 3;      // auf Maximum begrenzen

  _stufen[kanal] = stufe;
  analogWrite(_pins[kanal], _stufeToDuty(stufe));
}

void Licht::alleAus() {
  for (uint8_t i = 0; i < LICHT_MAX_KANAELE; i++)
    setKanal(i, 0);
}

uint8_t Licht::getStufe(uint8_t kanal) const {
  if (kanal >= LICHT_MAX_KANAELE) return 0;
  return _stufen[kanal];
}

// -----------------------------------------------------------------------------
//  Stufe -> Duty-Cycle-Wert
//  0 = aus  |  1 = low  |  2 = medium  |  3 = full
//  Werte kommen aus Config.h – dort zentralisiert aendern.
// -----------------------------------------------------------------------------
uint8_t Licht::_stufeToDuty(uint8_t stufe) {
  switch (stufe) {
    case 1:  return LOW_BRIGHTNESS_DUTY_CYCLE;
    case 2:  return MEDIUM_BRIGHTNESS_DUTY_CYCLE;
    case 3:  return 255;
    default: return 0;
  }
}
