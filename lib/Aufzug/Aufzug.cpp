#include "Aufzug.h"

Aufzug::Aufzug(int pinStep, int pinDir, int pinEnable,
               uint32_t stepIntervalUs, uint32_t timeoutMs)
  : _pinStep(pinStep), _pinDir(pinDir), _pinEnable(pinEnable),
    _stepIntervalUs(stepIntervalUs), _timeoutMs(timeoutMs) {}

void Aufzug::begin() {
  pinMode(_pinStep,   OUTPUT);
  pinMode(_pinDir,    OUTPUT);
  pinMode(_pinEnable, OUTPUT);
  _motorAus();   // Treiber deaktiviert starten (kein Strom auf den Motor)
}

void Aufzug::fahreZu(Etage ziel) {
  if (_zustand == Zustand::FAEHRT_AUF || _zustand == Zustand::FAEHRT_AB) return;  // faehrt schon
  if (_zustand == Zustand::FEHLER) return;                                        // erst quittieren
  if (ziel == _aktuelleEtage) return;                                             // schon da

  _zielEtage    = ziel;
  _fahrtStartMs = millis();

  bool aufwaerts = (static_cast<uint8_t>(ziel) > static_cast<uint8_t>(_aktuelleEtage));
  _motorEin(aufwaerts);
  _zustand = aufwaerts ? Zustand::FAEHRT_AUF : Zustand::FAEHRT_AB;
}

void Aufzug::update(bool endschalterEg, bool endschalterOg1, bool endschalterOg2) {
  if (_zustand != Zustand::FAEHRT_AUF && _zustand != Zustand::FAEHRT_AB) return;

  // Sicherheit: Timeout (Endschalter defekt / Seil blockiert / Motor blockiert).
  if (millis() - _fahrtStartMs > _timeoutMs) {
    _motorAus();
    _zustand = Zustand::FEHLER;
    return;
  }

  // Zielerreichung: passender Endschalter = einzige Wahrheit (keine Schrittzaehlung).
  if (_endschalterFuer(_zielEtage, endschalterEg, endschalterOg1, endschalterOg2)) {
    _motorAus();
    _aktuelleEtage = _zielEtage;
    _zustand       = Zustand::STEHT;
    return;
  }

  // Motor weitertakten (nicht-blockierend, kein delay()).
  unsigned long jetztUs = micros();
  if (jetztUs - _letzterStepUs >= _stepIntervalUs) {
    _letzterStepUs = jetztUs;
    digitalWrite(_pinStep, HIGH);
    digitalWrite(_pinStep, LOW);   // ein STEP-Puls = ein Vollschritt
  }
}

void Aufzug::fehlerQuittieren() {
  if (_zustand == Zustand::FEHLER) {
    _motorAus();
    _zustand = Zustand::STEHT;
    // Hinweis: _aktuelleEtage bleibt auf dem letzten bekannten Stand. Eine
    // Referenzfahrt (z.B. Richtung EG bis Endschalter) kann hier spaeter rein.
  }
}

void Aufzug::_motorEin(bool aufwaerts) {
  digitalWrite(_pinDir, aufwaerts ? HIGH : LOW);
  digitalWrite(_pinEnable, LOW);   // A4988/DRV8825: ENABLE aktiv LOW
  _letzterStepUs = micros();
}

void Aufzug::_motorAus() {
  digitalWrite(_pinEnable, HIGH);  // Treiber deaktivieren, kein Motorstrom
}

bool Aufzug::_endschalterFuer(Etage e, bool eg, bool og1, bool og2) const {
  switch (e) {
    case Etage::EG:  return eg;
    case Etage::OG1: return og1;
    case Etage::OG2: return og2;
  }
  return false;
}
