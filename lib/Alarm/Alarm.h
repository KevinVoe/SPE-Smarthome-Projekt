// =============================================================================
//  Alarm  –  Alarmrelais + Buzzer
// -----------------------------------------------------------------------------
//  Scharf/Unscharf wird vom Dashboard gesetzt. Die Bedingung zum Ausloesen
//  (offener Tuer-/Fensterkontakt) prueft die Regelung in main.cpp und ruft
//  dann ausloesen() auf.
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Io.h"

class Alarm {
public:
  Alarm(IoPin relais, IoPin buzzer);
  void begin();
  void update();                       // Platz fuer Sirenen-Muster

  void scharfschalten() { _scharf = true; }
  void entschaerfen();
  void ausloesen();                    // wirkt nur, wenn scharf

  bool istScharf()     const { return _scharf; }
  bool istAusgeloest() const { return _ausgeloest; }

private:
  void _setze(bool an);
  IoPin _relais, _buzzer;
  bool  _scharf     = false;
  bool  _ausgeloest = false;
};
