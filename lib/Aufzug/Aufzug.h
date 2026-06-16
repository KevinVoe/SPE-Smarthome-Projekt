// =============================================================================
//  Aufzug  –  Brushed-DC ueber Seilwinde, Positionierung per Zeitsteuerung
// -----------------------------------------------------------------------------
//  Treiber: einfach (Richtung + Enable), volle Drehzahl.
//  Referenzierung: nach unten fahren bis zum unteren Endschalter (= EG).
//  Etagen werden ueber kalibrierte FAHRZEITEN angefahren; die beiden
//  Endschalter (oben/unten) korrigieren die Position an den Endlagen.
//  Sicherheit: loest innerhalb AUFZUG_TIMEOUT_MS kein Endschalter aus
//  bzw. ist die Fahrzeit ueberschritten, geht der Aufzug in FEHLER (Not-Stopp).
//
//  Umgesetzt als nicht-blockierender ZUSTANDSAUTOMAT - kein delay()!
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Io.h"

class Aufzug {
public:
  enum Etage : uint8_t { EG = 0, OG1 = 1, OG2 = 2 };

  enum class Zustand : uint8_t {
    UNREFERENZIERT,   // Position unbekannt (nach dem Einschalten)
    REFERENZFAHRT,    // faehrt nach unten bis zum Endschalter
    BEREIT,           // steht, Position bekannt
    FAHRT_AUF,
    FAHRT_AB,
    FEHLER            // Not-Stopp (Timeout / Endschalter nie erreicht)
  };

  Aufzug(IoPin endschalterOben, IoPin endschalterUnten,
         IoPin motorRichtung, IoPin motorEnable,
         uint32_t zeitEgOg1, uint32_t zeitOg1Og2, uint32_t timeout);

  void begin();
  void update();                       // zyklisch aus loop() aufrufen

  void referenzieren();                // einmal nach dem Start aufrufen
  void fahreZu(Etage ziel);            // Fahrtwunsch (wird gepuffert, bis BEREIT)

  Zustand zustand() const { return _zustand; }
  Etage   etage()   const { return _aktuelleEtage; }

private:
  void     _motorAuf();
  void     _motorAb();
  void     _motorStop();
  bool     _obenGedrueckt();
  bool     _untenGedrueckt();
  uint32_t _fahrzeit(Etage von, Etage nach);

  IoPin    _swOben, _swUnten, _richtung, _enable;
  uint32_t _zeitEgOg1, _zeitOg1Og2, _timeout;

  Zustand  _zustand       = Zustand::UNREFERENZIERT;
  Etage    _aktuelleEtage = EG;
  Etage    _zielEtage     = EG;
  uint32_t _fahrtStart    = 0;
  uint32_t _fahrtDauer    = 0;
};
