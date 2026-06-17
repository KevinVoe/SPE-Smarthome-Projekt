// =============================================================================
//  Heizung  –  reine STEUERUNGSKLASSE fuer eine Etagenheizung (LED-simuliert)
// -----------------------------------------------------------------------------
//  WICHTIG: Dieses Modul wertet KEINEN Taster aus und kennt KEINE Modi!
//  Es bietet ausschliesslich Funktionen, um die Heizung gezielt ein- oder
//  auszuschalten. Welcher Taster wann welchen Modus ausloest (Heizen/Kuehlen/
//  Fenster auf/Fenster zu/Automatik), entscheidet AUSSCHLIESSLICH main.cpp -
//  der Taster gehoert keinem einzelnen Modul, sondern schaltet zwischen
//  mehreren Modulen um.
//
//  main.cpp ruft z.B. heizungEG.setState(true) auf, sobald der Taster-Modus
//  fuer die EG auf "Heizen" steht - und heizungEG.setState(false), sobald ein
//  anderer Modus aktiv wird.
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Io.h"

class Heizung {
public:
  Heizung(IoPin ledPin);

  void begin();

  void setState(bool an);     // EINZIGER Weg, die Heizung zu schalten
  void einschalten()  { setState(true); }
  void ausschalten()  { setState(false); }

  bool istAn() const { return _heizungAn; }

private:
  IoPin _ledPin;
  bool  _heizungAn = false;
};