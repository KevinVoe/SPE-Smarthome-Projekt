#include "Heizung.h"
#include config.h

Heizung::Heizung(int tasterPin, int ledPin) {
  _tasterPin = tasterPin;
  _ledPin = ledPin;
  _heizungAn = false;
  _letzterZustand = HIGH;  // HIGH = nicht gedrückt (wegen INPUT_PULLUP)
  _letzteAenderung = 0;
}

void Heizung::begin() {
  pinMode(_tasterPin, INPUT_PULLUP);
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_ledPin, LOW); // Heizung startet aus
}

void Heizung::update() {
  bool aktuellerZustand = digitalRead(_tasterPin);
  unsigned long jetzt = millis();

  // Reagiere nur auf den Moment des Drückens (fallende Flanke: HIGH -> LOW)
  // und nur, wenn seit der letzten Änderung genug Zeit vergangen ist (Entprellen)
  if (aktuellerZustand == LOW && _letzterZustand == HIGH) {
    if (jetzt - _letzteAenderung > _entprellzeit) {
      _heizungAn = !_heizungAn;              // Zustand umschalten
      digitalWrite(_ledPin, _heizungAn ? HIGH : LOW);
      _letzteAenderung = jetzt;
    }
  }

  _letzterZustand = aktuellerZustand;
}

bool Heizung::istAn() {
  return _heizungAn;
}