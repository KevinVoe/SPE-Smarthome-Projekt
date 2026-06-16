#include "Aufzug.h"

Aufzug::Aufzug(IoPin so, IoPin su, IoPin r, IoPin e,
               uint32_t z1, uint32_t z2, uint32_t to)
  : _swOben(so), _swUnten(su), _richtung(r), _enable(e),
    _zeitEgOg1(z1), _zeitOg1Og2(z2), _timeout(to) {}

void Aufzug::begin() {
  io.pinMode(_swOben,  INPUT_PULLUP);   // Endschalter gegen GND -> gedrueckt = LOW
  io.pinMode(_swUnten, INPUT_PULLUP);
  io.pinMode(_richtung, OUTPUT);
  io.pinMode(_enable,   OUTPUT);
  _motorStop();
}

bool Aufzug::_obenGedrueckt()  { return io.digitalRead(_swOben)  == LOW; }
bool Aufzug::_untenGedrueckt() { return io.digitalRead(_swUnten) == LOW; }

void Aufzug::_motorAuf()  { io.digitalWrite(_richtung, HIGH); io.digitalWrite(_enable, HIGH); }
void Aufzug::_motorAb()   { io.digitalWrite(_richtung, LOW);  io.digitalWrite(_enable, HIGH); }
void Aufzug::_motorStop() { io.digitalWrite(_enable, LOW); }

uint32_t Aufzug::_fahrzeit(Etage von, Etage nach) {
  // Summe der Teilstrecken zwischen den Etagen.
  int a = (von < nach) ? (int)von : (int)nach;
  int b = (von < nach) ? (int)nach : (int)von;
  uint32_t zeit = 0;
  for (int e = a; e < b; ++e) {
    zeit += (e == EG) ? _zeitEgOg1 : _zeitOg1Og2;
  }
  return zeit;
}

void Aufzug::referenzieren() {
  // Nach unten fahren bis zum unteren Endschalter = EG (Nullpunkt).
  _zustand    = Zustand::REFERENZFAHRT;
  _fahrtStart = millis();
  _motorAb();
}

void Aufzug::fahreZu(Etage ziel) {
  if (_zustand == Zustand::FEHLER)         return;            // erst quittieren
  if (_zustand == Zustand::UNREFERENZIERT) { referenzieren(); return; }
  if (_zustand != Zustand::BEREIT)         return;            // Fahrt laeuft noch
  if (ziel == _aktuelleEtage)              return;

  _zielEtage  = ziel;
  _fahrtDauer = _fahrzeit(_aktuelleEtage, ziel);
  _fahrtStart = millis();
  if (ziel > _aktuelleEtage) { _zustand = Zustand::FAHRT_AUF; _motorAuf(); }
  else                       { _zustand = Zustand::FAHRT_AB;  _motorAb();  }
}

void Aufzug::update() {
  uint32_t jetzt = millis();

  switch (_zustand) {
    case Zustand::REFERENZFAHRT:
      if (_untenGedrueckt()) {                       // Nullpunkt erreicht
        _motorStop(); _aktuelleEtage = EG; _zustand = Zustand::BEREIT;
      } else if (jetzt - _fahrtStart > _timeout) {   // Schalter nie erreicht
        _motorStop(); _zustand = Zustand::FEHLER;
      }
      break;

    case Zustand::FAHRT_AUF:
      if (_obenGedrueckt()) {                         // Endlage oben = OG2
        _motorStop(); _aktuelleEtage = OG2; _zustand = Zustand::BEREIT;
      } else if (jetzt - _fahrtStart >= _fahrtDauer) {// Fahrzeit erreicht
        _motorStop(); _aktuelleEtage = _zielEtage; _zustand = Zustand::BEREIT;
      } else if (jetzt - _fahrtStart > _timeout) {    // Sicherheit
        _motorStop(); _zustand = Zustand::FEHLER;
      }
      break;

    case Zustand::FAHRT_AB:
      if (_untenGedrueckt()) {                         // Endlage unten = EG
        _motorStop(); _aktuelleEtage = EG; _zustand = Zustand::BEREIT;
      } else if (jetzt - _fahrtStart >= _fahrtDauer) {
        _motorStop(); _aktuelleEtage = _zielEtage; _zustand = Zustand::BEREIT;
      } else if (jetzt - _fahrtStart > _timeout) {
        _motorStop(); _zustand = Zustand::FEHLER;
      }
      break;

    default:   // UNREFERENZIERT / BEREIT / FEHLER: nichts zu tun
      break;
  }
}
