#include "Aufzug.h"

// 4-Phasen-VOLLSCHRITT-Sequenz fuer 28BYJ-48 (Spaltenreihenfolge IN1,IN2,IN3,IN4).
// Immer 2 Spulen gleichzeitig aktiv -> mehr Drehmoment pro Schritt, und nur
// 2048 statt 4096 Schritte/Umdrehung -> doppelte Drehzahl bei gleichem Takt.
static const uint8_t VOLLSCHRITT[4][4] = {
  {1, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 1},
  {1, 0, 0, 1},
};

Aufzug::Aufzug(int in1, int in2, int in3, int in4,
               uint32_t stepIntervalUs, uint32_t timeoutMs)
  : _stepIntervalUs(stepIntervalUs), _timeoutMs(timeoutMs) {
  _in[0] = in1; _in[1] = in2; _in[2] = in3; _in[3] = in4;
}

void Aufzug::begin() {
  for (uint8_t i = 0; i < 4; i++) pinMode(_in[i], OUTPUT);
  _motorAus();   // Spulen stromlos starten
}

void Aufzug::_schreibeSequenz(uint8_t idx) {
  for (uint8_t i = 0; i < 4; i++)
    digitalWrite(_in[i], VOLLSCHRITT[idx][i] ? HIGH : LOW);   // HALBSCHRITT -> VOLLSCHRITT
}

void Aufzug::_motorEin(bool aufwaerts) {
  _richtung      = aufwaerts ? +1 : -1;
  _letzterStepUs = micros();
}

void Aufzug::_motorAus() {
  for (uint8_t i = 0; i < 4; i++) digitalWrite(_in[i], LOW);   // alle Spulen aus
}

void Aufzug::fahreZu(Etage ziel) {
  if (_zustand == Zustand::FEHLER) return;                            // erst quittieren
  if (_zustand == Zustand::STEHT && ziel == _aktuelleEtage) return;   // schon da

  _zielEtage    = ziel;
  _fahrtStartMs = millis();   // Timeout-Fenster neu aufziehen

  // Richtung aus der zuletzt bekannten Etage. Bei laufender Fahrt fuehrt update()
  // _aktuelleEtage auf JEDEN passierten Reed nach -> die Richtung stimmt auch fuer
  // einen Zielwechsel MITTEN in der Fahrt (z.B. unterwegs OG1->OG2, dann EG -> dreht um).
  bool aufwaerts;
  if      (static_cast<uint8_t>(ziel) > static_cast<uint8_t>(_aktuelleEtage)) aufwaerts = true;
  else if (static_cast<uint8_t>(ziel) < static_cast<uint8_t>(_aktuelleEtage)) aufwaerts = false;
  else  // Ziel == zuletzt passierte Etage, aber noch in Fahrt -> umkehren
    aufwaerts = (_zustand == Zustand::FAEHRT_AB);

  _motorEin(aufwaerts);
  _zustand = aufwaerts ? Zustand::FAEHRT_AUF : Zustand::FAEHRT_AB;
}

void Aufzug::update(bool endschalterEg, bool endschalterOg1, bool endschalterOg2,
                    bool ueberfahrOben) {
  if (_zustand != Zustand::FAEHRT_AUF && _zustand != Zustand::FAEHRT_AB) return;

  // Hoechste Prioritaet: oberer Ueberfahr-Schalter -> Kabine zu weit oben -> Not-Stopp.
  if (ueberfahrOben) {
    _motorAus();
    _zustand = Zustand::FEHLER;
    return;
  }

  // Sicherheit: Timeout (Endschalter defekt / Seil blockiert / Motor blockiert).
  if (millis() - _fahrtStartMs > _timeoutMs) {
    _motorAus();
    _zustand = Zustand::FEHLER;
    return;
  }

  // Position nachfuehren: welcher Etagen-Reed ist GERADE aktiv? Auch beim
  // Passieren einer Zwischen-Etage -> _aktuelleEtage bleibt aktuell (wichtig
  // fuer einen Richtungswechsel mitten in der Fahrt).
  if      (endschalterEg)  _aktuelleEtage = Etage::EG;
  else if (endschalterOg1) _aktuelleEtage = Etage::OG1;
  else if (endschalterOg2) _aktuelleEtage = Etage::OG2;

  // Ziel erreicht? (Reed der Zieletage = einzige Wahrheit, keine Schrittzaehlung)
  if (_aktuelleEtage == _zielEtage) {
    _motorAus();
    _zustand = Zustand::STEHT;
    return;
  }

  // Motor weitertakten (nicht-blockierend): naechste Halbschritt-Phase setzen.
  unsigned long jetztUs = micros();
  if (jetztUs - _letzterStepUs >= _stepIntervalUs) {
    _letzterStepUs = jetztUs;
    _seqIndex = (uint8_t)((_seqIndex + _richtung + 4) % 4);   // war: + 8) % 8
    _schreibeSequenz(_seqIndex);
  }
}

void Aufzug::fehlerQuittieren() {
  if (_zustand == Zustand::FEHLER) {
    _motorAus();
    _zustand = Zustand::STEHT;
    // Hinweis: _aktuelleEtage bleibt auf dem letzten bekannten Stand. Eine
    // Referenzfahrt (Richtung EG bis Endschalter) kann hier spaeter rein.
  }
}

bool Aufzug::_endschalterFuer(Etage e, bool eg, bool og1, bool og2) const {
  switch (e) {
    case Etage::EG:  return eg;
    case Etage::OG1: return og1;
    case Etage::OG2: return og2;
  }
  return false;
}
