#pragma once
#include <Arduino.h>

class Aufzug {
public:
  enum class Etage   : uint8_t { EG = 0, OG1 = 1, OG2 = 2 };
  enum class Zustand : uint8_t {
    STEHT, FAEHRT_AUF, FAEHRT_AB, FEHLER,
    HOMING,      // Startup-Suchfahrt nach oben bis zum ersten Reed
    FREIFAHREN   // Not-Aus-Rueckfahrt nach unten (bis OG2 / Extra-Timeout)
  };

  Aufzug(int in1, int in2, int in3, int in4,
         uint32_t stepIntervalUs, uint32_t timeoutMs, uint32_t notausTimeoutMs);

  void begin();
  void fahreZu(Etage ziel);
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
  void _takte();                          // nicht-blockierender Einzelschritt
  void _aktualisierePosition(bool eg, bool og1, bool og2);
  void _starteFreifahren();
  bool _endschalterFuer(Etage e, bool eg, bool og1, bool og2) const;

  int      _in[4];
  uint32_t _stepIntervalUs, _timeoutMs, _notausTimeoutMs;

  Zustand       _zustand       = Zustand::STEHT;
  Etage         _aktuelleEtage = Etage::EG;
  Etage         _zielEtage     = Etage::EG;
  int8_t        _richtung      = +1;
  uint8_t       _seqIndex      = 0;
  unsigned long _letzterStepUs = 0;
  unsigned long _fahrtStartMs  = 0;
  unsigned long _notausStartMs = 0;
};
