// =============================================================================
//  DigitaleOutputs  –  besitzt den AUSGANGS-MCP23017 (Transistoren, active-high)
// -----------------------------------------------------------------------------
//  Dieses Modul besitzt seinen Chip selbst (kein gemeinsames Io mehr). Hinter
//  jedem Ausgang steht die REALE Funktion: die Regelung ruft heizen(Etage::OG1,
//  true) statt "Pin X auf HIGH" - lesbar und unabhaengig von der Pin-Belegung
//  (MCPOUT_* in Config.h).
//
//  WICHTIG: Wire muss laufen (Wire.begin(...) EINMAL in main/setup), bevor
//  digitaleOutputsBegin() aufgerufen wird.
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"   // Etage, MCPOUT_*, ADDR_MCP_OUT

void digitaleOutputsBegin();   // Ausgangs-MCP init: alle Pins OUTPUT, LOW (aus)
bool digitaleOutputsOk();      // true, wenn der Ausgangs-MCP am Bus antwortet

void heizen(Etage etage, bool an);    // rote LED je Etage
void kuehlen(Etage etage, bool an);   // blaue LED je Etage
