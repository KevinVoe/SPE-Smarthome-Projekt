#include "Heizung.h"

Heizung::Heizung(IoPin ledPin)
  : _ledPin(ledPin) {}

void Heizung::begin() {
  io.pinMode(_ledPin, OUTPUT);
  setState(false);            // Heizung startet aus
}

void Heizung::setState(bool an) {
  _heizungAn = an;
  io.digitalWrite(_ledPin, an ? HIGH : LOW);
}