#include "Alarm.h"

Alarm::Alarm(IoPin relais, IoPin buzzer) : _relais(relais), _buzzer(buzzer) {}

void Alarm::begin() {
  io.pinMode(_relais, OUTPUT);
  io.pinMode(_buzzer, OUTPUT);
  _setze(false);
}

void Alarm::entschaerfen() {
  _scharf = false;
  _ausgeloest = false;
  _setze(false);
}

void Alarm::ausloesen() {
  if (_scharf) { _ausgeloest = true; _setze(true); }
}

void Alarm::update() {
  // TODO (optional): Buzzer im Takt blinken lassen statt Dauerton.
}

void Alarm::_setze(bool an) {
  io.digitalWrite(_relais, an ? HIGH : LOW);
  io.digitalWrite(_buzzer, an ? HIGH : LOW);
}
