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
               uint32_t stepIntervalUs, uint32_t timeoutMs, uint32_t notausTimeoutMs)
  : _stepIntervalUs(stepIntervalUs), _timeoutMs(timeoutMs), _notausTimeoutMs(notausTimeoutMs) {
  _in[0] = in1; _in[1] = in2; _in[2] = in3; _in[3] = in4;
}

void Aufzug::begin() {
  for (uint8_t i = 0; i < 4; i++) pinMode(_in[i], OUTPUT);
  _motorAus();

  _motorEin(true);
  _fahrtStartMs = millis();
  _zustand      = Zustand::HOMING;
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
  for (uint8_t i = 0; i < 4; i++) digitalWrite(_in[i], LOW);
}

void Aufzug::fahreZu(Etage ziel) {
  // Waehrend Homing/Not-Aus-Rueckfahrt oder im FEHLER keine neuen Fahrten annehmen.
  if (_zustand == Zustand::FEHLER || _zustand == Zustand::HOMING ||
      _zustand == Zustand::FREIFAHREN) return;
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

void Aufzug::_takte() {
  unsigned long jetztUs = micros();
  if (jetztUs - _letzterStepUs >= _stepIntervalUs) {
    _letzterStepUs = jetztUs;
    _seqIndex = (uint8_t)((_seqIndex + _richtung + 4) % 4);
    _schreibeSequenz(_seqIndex);
  }
}

void Aufzug::update(bool endschalterEg, bool endschalterOg1, bool endschalterOg2,
                    bool ueberfahrOben) {
  switch (_zustand) {

    // ── Normale Fahrt zu einer Zieletage ────────────────────────────────────
    case Zustand::FAEHRT_AUF:
    case Zustand::FAEHRT_AB:
      if (ueberfahrOben) { _starteFreifahren(); return; }       // Not-Aus -> Selbst-Rueckfahrt
      if (millis() - _fahrtStartMs > _timeoutMs) {              // Sicherheit: Dauerlauf
        _motorAus(); _zustand = Zustand::FEHLER; return;
      }
      // Position nachfuehren (auch beim Passieren einer Zwischen-Etage).
      _aktualisierePosition(endschalterEg, endschalterOg1, endschalterOg2);
      if (_aktuelleEtage == _zielEtage) {                       // Ziel-Reed erreicht
        _motorAus(); _zustand = Zustand::STEHT; return;
      }
      _takte();   // nicht-blockierender Einzelschritt
      return;

    // ── Startup-Homing: nach oben bis zum ERSTEN beliebigen Reed ─────────────
    case Zustand::HOMING:
      if (ueberfahrOben) { _starteFreifahren(); return; }       // oben ohne Etage -> runter
      if (millis() - _fahrtStartMs > _timeoutMs) {
        _motorAus(); _zustand = Zustand::FEHLER; return;
      }
      if (endschalterEg || endschalterOg1 || endschalterOg2) {  // Position gefunden
        _aktualisierePosition(endschalterEg, endschalterOg1, endschalterOg2);
        _motorAus(); _zustand = Zustand::STEHT;
        return;
      }
      _takte();   // nicht-blockierender Einzelschritt
      return;

    // ── Not-Aus-Rueckfahrt: nach unten bis OG2-Reed, dann SELBST quittieren ──
    case Zustand::FREIFAHREN:
      // Der obere Schalter wird hier BEWUSST ignoriert (wir fahren ja von ihm weg).
      if (endschalterOg2) {                                     // OG2 erreicht (~5 cm unter oben)
        _aktuelleEtage = Etage::OG2; _motorAus(); _zustand = Zustand::STEHT; return;
      }
      if (millis() - _notausStartMs > _notausTimeoutMs) {       // Extra-Timeout: IN JEDEM FALL stoppen
        _motorAus(); _zustand = Zustand::STEHT; return;
      }
      _takte();   // nicht-blockierender Einzelschritt
      return;

    default:   // STEHT, FEHLER: nichts tun
      return;
  }
}

// Position aus dem GERADE aktiven Etagen-Reed nachfuehren.
void Aufzug::_aktualisierePosition(bool eg, bool og1, bool og2) {
  if      (eg)  _aktuelleEtage = Etage::EG;
  else if (og1) _aktuelleEtage = Etage::OG1;
  else if (og2) _aktuelleEtage = Etage::OG2;
}

// Not-Aus-Rueckfahrt starten: abwaerts mit eigenem Timeout.
void Aufzug::_starteFreifahren() {
  _motorEin(false);                 // abwaerts
  _notausStartMs = millis();
  _zustand       = Zustand::FREIFAHREN;
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
