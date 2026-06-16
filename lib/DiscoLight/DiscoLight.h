// =============================================================================
//  DiscoLight  –  Stimmungslicht (NeoPixel-Regenbogen, nicht-blockierend)
// -----------------------------------------------------------------------------
//  Beispiel fuer eine zeitgesteuerte Animation OHNE delay(): update() macht
//  nur dann einen Schritt, wenn genug Zeit vergangen ist.
// =============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class DiscoLight {
public:
  DiscoLight(int stripPin, uint16_t anzahlLeds);
  void begin();
  void update();                       // animiert, solange an
  void an()  { _an = true; }
  void aus();
  bool istAn() const { return _an; }

private:
  Adafruit_NeoPixel _strip;
  bool     _an    = false;
  uint16_t _phase = 0;
  uint32_t _letzt = 0;
};
