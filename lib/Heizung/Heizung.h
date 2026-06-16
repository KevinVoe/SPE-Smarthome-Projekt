// =============================================================================
//  Heizung  –  eine Etagenheizung (LED-simuliert)
// -----------------------------------------------------------------------------
//  Zwei Betriebsarten:
//   * AUTO        : Zweipunkt-Regelung (Hysterese) um die Solltemperatur
//   * MANUELL_AN  : per Taster fest eingeschaltet (Regelung wird ignoriert)
//  Der Taster schaltet zwischen beiden Betriebsarten um.
//
//  Dieses Modul dient zugleich als VORLAGE fuer weitere Module:
//  Konstruktor nimmt Pins/Parameter entgegen, begin()/update() werden aus
//  main aufgerufen, Hardwarezugriff laeuft ueber die Io-Schicht.
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Io.h"

enum class HeizModus : uint8_t {
  AUTO,        // Zweipunkt-Regelung ueber Soll +/- Hysterese
  MANUELL_AN   // fest eingeschaltet
};

class Heizung {
public:
  Heizung(IoPin tasterPin, IoPin ledPin, float sollTemp, float hysterese);

  void begin();
  void update();                       // zyklisch aus loop() aufrufen

  void setIstTemperatur(float t);      // aktuelle Raumtemperatur reinreichen
  void setSoll(float t);
  void setModus(HeizModus m);
  void einschaltenManuell();           // z.B. vom Dashboard

  bool      istAn()  const { return _heizungAn; }
  float     soll()   const { return _soll; }
  HeizModus modus()  const { return _modus; }

private:
  void _tasterAuswerten();
  void _regeln();
  void _schalten(bool an);

  IoPin _tasterPin;
  IoPin _ledPin;
  float _soll;
  float _hysterese;
  float _ist = NAN;                    // noch kein Messwert vorhanden
  HeizModus _modus = HeizModus::AUTO;
  bool  _heizungAn = false;

  // Tasterentprellung
  int           _letzterTaster   = HIGH;
  unsigned long _letzteAenderung = 0;
  static constexpr unsigned long _entprellMs = 50;
};
