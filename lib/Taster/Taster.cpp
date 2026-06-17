#include "Taster.h"

Taster::Taster(IoPin pinEg, IoPin pinOg1, IoPin pinOg2,
               IoPin pinAufzugEg, IoPin pinAufzugOg1, IoPin pinAufzugOg2,
               IoPin pinEndschalterEg, IoPin pinEndschalterOg1, IoPin pinEndschalterOg2)
  : _pinEg(pinEg), _pinOg1(pinOg1), _pinOg2(pinOg2),
    _pinAufzugEg(pinAufzugEg), _pinAufzugOg1(pinAufzugOg1), _pinAufzugOg2(pinAufzugOg2),
    _pinEndschalterEg(pinEndschalterEg), _pinEndschalterOg1(pinEndschalterOg1),
    _pinEndschalterOg2(pinEndschalterOg2) {}

void Taster::begin() {
  io.pinMode(_pinEg,  INPUT_PULLUP);   // Taster gegen GND -> gedrueckt = LOW
  io.pinMode(_pinOg1, INPUT_PULLUP);
  io.pinMode(_pinOg2, INPUT_PULLUP);

  io.pinMode(_pinAufzugEg,  INPUT_PULLUP);
  io.pinMode(_pinAufzugOg1, INPUT_PULLUP);
  io.pinMode(_pinAufzugOg2, INPUT_PULLUP);

  io.pinMode(_pinEndschalterEg,  INPUT_PULLUP);
  io.pinMode(_pinEndschalterOg1, INPUT_PULLUP);
  io.pinMode(_pinEndschalterOg2, INPUT_PULLUP);
}

void Taster::update() {
  _status.eg  = _entprelltLesen(_pinEg,  _rohEg,  _tEg,  _bEg);
  _status.og1 = _entprelltLesen(_pinOg1, _rohOg1, _tOg1, _bOg1);
  _status.og2 = _entprelltLesen(_pinOg2, _rohOg2, _tOg2, _bOg2);

  _status.aufzugEg  = _entprelltLesen(_pinAufzugEg,  _rohAufzugEg,  _tAufzugEg,  _bAufzugEg);
  _status.aufzugOg1 = _entprelltLesen(_pinAufzugOg1, _rohAufzugOg1, _tAufzugOg1, _bAufzugOg1);
  _status.aufzugOg2 = _entprelltLesen(_pinAufzugOg2, _rohAufzugOg2, _tAufzugOg2, _bAufzugOg2);

  _status.endschalterEg  = _entprelltLesen(_pinEndschalterEg,  _rohEndEg,  _tEndEg,  _bEndEg);
  _status.endschalterOg1 = _entprelltLesen(_pinEndschalterOg1, _rohEndOg1, _tEndOg1, _bEndOg1);
  _status.endschalterOg2 = _entprelltLesen(_pinEndschalterOg2, _rohEndOg2, _tEndOg2, _bEndOg2);
}

// Klassisches Entprellverfahren: Aendert sich der ROHE Pegel, wird die Zeit
// gemerkt. Bleibt der Pegel danach mindestens _entprellMs lang stabil, wird
// er als "wahrer", bereinigter Zustand uebernommen. Das filtert Kontakt-
// wackeln in BEIDE Richtungen (Druecken/Ausloesen und Loslassen).
bool Taster::_entprelltLesen(IoPin pin, int& letzterRohwert, unsigned long& letzteAenderung,
                              bool& letzterBereinigterWert) {
  int           rohwert = io.digitalRead(pin);
  unsigned long jetzt    = millis();

  if (rohwert != letzterRohwert) {
    letzteAenderung = jetzt;
    letzterRohwert  = rohwert;
  }

  if ((jetzt - letzteAenderung) > _entprellMs) {
    // Pegel ist seit > _entprellMs stabil -> als bereinigten Wert uebernehmen.
    // INPUT_PULLUP: ausgeloest/gedrueckt = LOW -> bereinigter Wert = (LOW).
    letzterBereinigterWert = (rohwert == LOW);
  }

  return letzterBereinigterWert;
}