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
#include "esp_timer.h"   // Hardware-Timer fuer gleichmaessigen Schritttakt

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
  void _starteFreifahren();   // Not-Aus-Rueckfahrt nach unten starten
  bool _endschalterFuer(Etage e, bool eg, bool og1, bool og2) const;

  // Der Motor wird von einem ESP32-Hardware-Timer (esp_timer) getaktet, damit die
  // Schritte GLEICHMAESSIG kommen - unabhaengig davon, wie lange die Haupt-Loop
  // gerade braucht (LCD/DHT/Telemetrie). _onTimer() macht EINEN Schritt, solange
  // _faehrt gesetzt ist. Gemeinsame Felder sind per _mux geschuetzt.
  static void _timerThunk(void* arg);   // C-Callback -> ruft _onTimer()
  void        _onTimer();

  int      _in[4];                  // IN1..IN4 (ULN2003)
  uint32_t _stepIntervalUs, _timeoutMs, _notausTimeoutMs;
  esp_timer_handle_t _timer = nullptr;
  portMUX_TYPE       _mux   = portMUX_INITIALIZER_UNLOCKED;

  Zustand  _zustand       = Zustand::STEHT;
  Etage    _aktuelleEtage = Etage::EG;
  Etage    _zielEtage     = Etage::EG;
  volatile int8_t  _richtung = +1;     // +1 = aufwaerts, -1 = abwaerts (Timer liest)
  volatile uint8_t _seqIndex = 0;      // 0..3 (Vollschritt-Phase, Timer veraendert)
  volatile bool    _faehrt   = false;  // true = Timer soll takten
  unsigned long _fahrtStartMs  = 0;
  unsigned long _notausStartMs = 0;
};
