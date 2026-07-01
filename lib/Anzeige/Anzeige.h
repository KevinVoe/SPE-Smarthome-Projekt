// =============================================================================
//  Anzeige  –  20x4-LCD-Statusdisplay (HD44780 ueber PCF8574-I2C-Backpack)
// -----------------------------------------------------------------------------
//  Zeigt eine Statusseite: Uhrzeit, Lufttemperatur, Luftfeuchte, Solarspannung
//  und Aufzugsposition. Aktualisiert sich nur alle LCD_UPDATE_INTERVALL_MS
//  (Config.h), damit der I2C-Bus nicht unnoetig belastet wird.
//
//  Aufruf aus main.cpp:
//    anzeige.begin();          // einmal in setup() - NACH Wire.begin(...)
//    anzeige.update(k);        // in jeder loop() - intern gedrosselt
//
//  Adresse/Groesse/Intervall stehen in Config.h (ADDR_LCD, LCD_SPALTEN,
//  LCD_ZEILEN, LCD_UPDATE_INTERVALL_MS).
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"
#include "Regelung.h"   // Kontext (Uhrzeit, Temp, Feuchte, PV, Aufzug-Etage)

class Anzeige {
public:
  void begin();                     // LCD initialisieren (Wire muss bereits laufen)
  void update(const Kontext& k);    // Statusseite zeichnen (gedrosselt)
  bool ok() const { return _ok; }   // LCD beim Start am I2C-Bus erreichbar?

private:
  void _zeichneStatus(const Kontext& k);

  bool     _ok    = false;
  uint32_t _letzt = 0;              // Zeitpunkt der letzten Aktualisierung
};
