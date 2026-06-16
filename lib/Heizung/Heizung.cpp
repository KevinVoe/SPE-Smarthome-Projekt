#include "Heizung.h"

Heizung::Heizung(IoPin tasterPin, IoPin ledPin, float sollTemp, float hysterese)
  : _tasterPin(tasterPin), _ledPin(ledPin), _soll(sollTemp), _hysterese(hysterese) {}

void Heizung::begin() {
  io.pinMode(_tasterPin, INPUT_PULLUP);   // Taster gegen GND -> gedrueckt = LOW
  io.pinMode(_ledPin, OUTPUT);
  _schalten(false);                       // Heizung startet aus
}

void Heizung::setIstTemperatur(float t) { _ist = t; }
void Heizung::setSoll(float t)          { _soll = t; }
void Heizung::setModus(HeizModus m)     { _modus = m; }
void Heizung::einschaltenManuell()      { _modus = HeizModus::MANUELL_AN; }

void Heizung::update() {
  _tasterAuswerten();
  _regeln();
}

void Heizung::_tasterAuswerten() {
  int           jetzt = io.digitalRead(_tasterPin);
  unsigned long t     = millis();

  // Fallende Flanke (HIGH->LOW) = Tastendruck, mit Entprellung.
  if (jetzt == LOW && _letzterTaster == HIGH && (t - _letzteAenderung) > _entprellMs) {
    // Taster schaltet zwischen Automatik und "manuell an" um.
    _modus = (_modus == HeizModus::AUTO) ? HeizModus::MANUELL_AN : HeizModus::AUTO;
    _letzteAenderung = t;
  }
  _letzterTaster = jetzt;
}

void Heizung::_regeln() {
  if (_modus == HeizModus::MANUELL_AN) {
    _schalten(true);
    return;
  }
  // AUTO: Zweipunkt-Regelung (Hysterese)
  if (isnan(_ist)) return;                          // ohne Messwert nichts tun
  if (_ist < _soll - _hysterese)      _schalten(true);
  else if (_ist > _soll + _hysterese) _schalten(false);
  // dazwischen: Zustand bewusst halten (das ist die Hysterese)
}

void Heizung::_schalten(bool an) {
  _heizungAn = an;
  io.digitalWrite(_ledPin, an ? HIGH : LOW);
}
