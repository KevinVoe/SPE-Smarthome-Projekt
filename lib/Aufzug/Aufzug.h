// =============================================================================
//  Aufzug  –  STEUERUNGSKLASSE fuer den Schrittmotor-Aufzug
//             (28BYJ-48 ueber ULN2003: IN1-IN4, Halbschritt-Sequenz)
// -----------------------------------------------------------------------------
//  Steuert NUR den Motor (ESP32-GPIOs direkt). Endschalter liest der Aufzug
//  NICHT selbst: der Aufrufer reicht die bereinigten Pegel an update() weiter.
//
//      Aufzug aufzug(IN1, IN2, IN3, IN4, AUFZUG_STEP_INTERVALL_US, AUFZUG_TIMEOUT_MS);
//      aufzug.fahreZu(Aufzug::Etage::OG2);
//      aufzug.update(endEg, endOg1, endOg2);   // 1x pro loop()
//
//  Antrieb: unipolarer Schrittmotor, 8-Phasen-HALBSCHRITT. "auf"/"ab" = Sequenz
//  vorwaerts/rueckwaerts; "Motor aus" = alle 4 Spulen-Pins LOW (stromlos).
//  ENDSCHALTER = einzige Wahrheit (keine Schrittzaehlung). Timeout schuetzt vor
//  Dauerlauf, falls ein Endschalter nicht ausloest.
// =============================================================================
#pragma once
#include <Arduino.h>

class Aufzug {
public:
  enum class Etage : uint8_t { EG = 0, OG1 = 1, OG2 = 2 };

  enum class Zustand : uint8_t {
    STEHT, FAEHRT_AUF, FAEHRT_AB, FEHLER,
    HOMING,      // Startup-Suchfahrt nach oben bis zum ersten Reed
    FREIFAHREN   // Not-Aus-Rueckfahrt nach unten (bis OG2 / Extra-Timeout)
  };

  Aufzug(int in1, int in2, int in3, int in4,
         uint32_t stepIntervalUs, uint32_t timeoutMs, uint32_t notausTimeoutMs);

  void begin();
  void fahreZu(Etage ziel);
  // endschalterEg/Og1/Og2 = Etagen-Reeds; ueberfahrOben = oberer Sicherheits-
  // schalter (Kabine zu weit oben) -> sofort Not-Stopp + FEHLER.
  void update(bool endschalterEg, bool endschalterOg1, bool endschalterOg2,
              bool ueberfahrOben);

  Zustand zustand()       const { return _zustand; }
  Etage   aktuelleEtage() const { return _aktuelleEtage; }
  Etage   zielEtage()     const { return _zielEtage; }
  bool    hatFehler()     const { return _zustand == Zustand::FEHLER; }
  void    fehlerQuittieren();

private:
  void _motorEin(bool richtungAufwaerts);
  void _motorAus();
  void _schreibeSequenz(uint8_t idx);
  void _aktualisierePosition(bool eg, bool og1, bool og2);
  void _takte();              // einen Schritt in _richtung (nicht-blockierend)
  void _starteFreifahren();   // Not-Aus-Rueckfahrt nach unten starten
  bool _endschalterFuer(Etage e, bool eg, bool og1, bool og2) const;

  int      _in[4];                  // IN1..IN4 (ULN2003)
  uint32_t _stepIntervalUs, _timeoutMs, _notausTimeoutMs;

  Zustand  _zustand       = Zustand::STEHT;
  Etage    _aktuelleEtage = Etage::EG;
  Etage    _zielEtage     = Etage::EG;
  int8_t   _richtung      = +1;     // +1 = aufwaerts, -1 = abwaerts (Sequenzrichtung)
  uint8_t  _seqIndex      = 0;      // 0..7 (Halbschritt-Phase)
  unsigned long _letzterStepUs = 0;
  unsigned long _fahrtStartMs  = 0;
  unsigned long _notausStartMs = 0;
};
