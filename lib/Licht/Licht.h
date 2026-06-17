// =============================================================================
//  Licht  –  PWM-Ansteuerung von bis zu 6 unabhaengigen 12V-LED-Kanaelen
//            ueber IRLZ44N-MOSFETs (Gate direkt an ESP32-GPIO).
// -----------------------------------------------------------------------------
//  Jeder Kanal wird mit setKanal(kanal, stufe) gesetzt:
//    kanal  : 0 .. 5  (max. LICHT_MAX_KANAELE - 1)
//    stufe  : 0 = aus
//             1 = LOW_BRIGHTNESS_DUTY_CYCLE   (aus Config.h)
//             2 = MEDIUM_BRIGHTNESS_DUTY_CYCLE (aus Config.h)
//             3 = 255 (voll an)
//
//  WANN welcher Kanal geschaltet wird, entscheidet die Regelung in main.cpp.
//  Dieses Modul ist reines Ausgabe-Modul ohne eigene Regellogik.
//
//  Hinweis IRLZ44N: Logic-Level-MOSFET, Gate-Schwelle ~1-2V -> direkt
//  mit 3,3V ESP32-GPIO ansteuerbar. Kein Gate-Vorwiderstand noetig,
//  empfohlen werden aber 100 Ohm zum Daempfen von Schwingungen.
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"

class Licht {
public:
  // pins: Array mit genau LICHT_MAX_KANAELE GPIO-Nummern, z.B. {5,19,23,...}
  Licht(const int pins[LICHT_MAX_KANAELE]);

  void begin();                              // alle Pins als OUTPUT, alle aus

  // Hauptschnittstelle: Kanal 0-5 auf Stufe 0-3 setzen
  void setKanal(uint8_t kanal, uint8_t stufe);

  // Komfort-Helfer
  void kanalAus(uint8_t kanal)        { setKanal(kanal, 0); }
  void alleAus();

  uint8_t getStufe(uint8_t kanal) const;    // aktuell gesetzte Stufe abfragen

private:
  int     _pins[LICHT_MAX_KANAELE];
  uint8_t _stufen[LICHT_MAX_KANAELE];       // gespeicherte Stufen (0-3)

  // Stufe -> Duty-Cycle-Wert (0-255)
  static uint8_t _stufeToDuty(uint8_t stufe);
};