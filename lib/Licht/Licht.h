// =============================================================================
//  Licht  –  Raumlicht ueber einen NeoPixel-Streifen (reines Ausgabe-Modul)
// -----------------------------------------------------------------------------
//  WANN an/aus geschaltet wird, entscheidet die Regelung in main.cpp
//  (Automatik-Regel: Bewegung + Dunkelheit). Dieses Modul kann nur an()/aus().
// =============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class Licht {
public:
  Licht(int stripPin, uint16_t anzahlLeds);
  void begin();
  void update();                       // (aktuell nichts; fuer Effekte frei)
  void an();
  void aus();
  bool istAn() const { return _an; }

private:
  Adafruit_NeoPixel _strip;
  bool _an = false;
};
