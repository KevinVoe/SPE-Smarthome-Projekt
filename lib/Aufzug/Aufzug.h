// =============================================================================
//  Aufzug  –  STEUERUNGSKLASSE fuer den Schrittmotor-Aufzug (STEP/DIR/ENABLE)
// -----------------------------------------------------------------------------
//  Steuert NUR den Motor (ESP32-GPIOs direkt - kein Io mehr). Endschalter liest
//  der Aufzug NICHT selbst: main reicht die bereinigten Pegel an update() weiter.
//
//      Aufzug aufzug(STEP, DIR, ENABLE, AUFZUG_STEP_INTERVALL_US, AUFZUG_TIMEOUT_MS);
//      aufzug.fahreZu(Aufzug::Etage::OG2);
//      aufzug.update(endEg, endOg1, endOg2);   // 1x pro loop()
//
//  ENDSCHALTER = einzige Wahrheit (keine Schrittzaehlung). Timeout schuetzt vor
//  Dauerlauf. Vollschritt: ein STEP-Puls = ein Schritt; ENABLE aktiv LOW.
// =============================================================================
#pragma once
#include <Arduino.h>

class Aufzug {
public:
  enum class Etage : uint8_t { EG = 0, OG1 = 1, OG2 = 2 };

  enum class Zustand : uint8_t {
    STEHT, FAEHRT_AUF, FAEHRT_AB, FEHLER
  };

  Aufzug(int pinStep, int pinDir, int pinEnable,
         uint32_t stepIntervalUs, uint32_t timeoutMs);

  void begin();
  void fahreZu(Etage ziel);
  void update(bool endschalterEg, bool endschalterOg1, bool endschalterOg2);

  Zustand zustand()       const { return _zustand; }
  Etage   aktuelleEtage() const { return _aktuelleEtage; }
  bool    hatFehler()     const { return _zustand == Zustand::FEHLER; }
  void    fehlerQuittieren();

private:
  void _motorEin(bool richtungAufwaerts);
  void _motorAus();
  bool _endschalterFuer(Etage e, bool eg, bool og1, bool og2) const;

  int _pinStep, _pinDir, _pinEnable;       // ESP32-GPIOs (native)
  uint32_t _stepIntervalUs, _timeoutMs;

  Zustand _zustand       = Zustand::STEHT;
  Etage   _aktuelleEtage = Etage::EG;
  Etage   _zielEtage     = Etage::EG;
  unsigned long _letzterStepUs = 0;
  unsigned long _fahrtStartMs  = 0;
};
