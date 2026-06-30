// =============================================================================
//  DigitalInput  –  besitzt den EINGANGS-MCP23017 (Entprellung + Flanke)
// -----------------------------------------------------------------------------
//  Dieses Modul besitzt seinen Chip selbst (kein gemeinsames Io mehr) und setzt
//  alle 16 Pins auf INPUT_PULLUP. Es bietet:
//   - gedrueckt(e)        : entprellter Pegel (aktiv/geschlossen = true)
//   - geradeGedrueckt(e)  : Einmal-Impuls bei steigender Flanke
//
//  Es kennt KEINE Modi/TTL - das ist Regelungs-Logik (TasterState in Regelung,
//  gefuettert aus geradeGedrueckt()). Reeds fragt man per gedrueckt() ab.
//
//  WICHTIG: Wire muss laufen (Wire.begin(...) EINMAL in main/setup), bevor
//  digitalInputBegin() aufgerufen wird.
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"   // MCPIN_*, ADDR_MCP_IN

// Reihenfolge = interner Index. Neuer Eingang -> hier + in DigitalInput.cpp (pinVon).
enum class Eingang : uint8_t {
  TASTER_EG, TASTER_OG1, TASTER_OG2,                       // Klima-Modus-Taster je Etage
  AUFZUG_TASTER_EG, AUFZUG_TASTER_OG1, AUFZUG_TASTER_OG2,  // Aufzug-Ruftaster je Etage
  REED_TUER,                                               // Tuerkontakt
  REED_AUFZUG_EG, REED_AUFZUG_OG1, REED_AUFZUG_OG2,  // Aufzug-Etagen-Reeds (je Etage einer)
  REED_AUFZUG_OBEN,                                  // oberer Ueberfahr-Schalter (Sicherheit)
  ANZAHL
};

void digitalInputBegin();    // Eingangs-MCP init: alle Pins INPUT_PULLUP
bool digitalInputOk();       // true, wenn der Eingangs-MCP am Bus antwortet
void digitalInputUpdate();   // EINMAL pro loop(): entprellen + Flanken berechnen

bool gedrueckt(Eingang e);        // entprellter Pegel: aktiv/geschlossen = true
bool geradeGedrueckt(Eingang e);  // true GENAU im Update der steigenden Flanke
bool gedruecktRoh(Eingang e);     // UNentprellt: unmittelbarer Pegel (fuer schnelle Reeds/Endschalter)
