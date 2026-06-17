// =============================================================================
//  Config.h  –  ZENTRALE Konfiguration des Smarthomes
// -----------------------------------------------------------------------------
//  AKTUELLER STAND: Simulationsaufbau auf dem Breadboard.
//  Alle Pins liegen direkt am ESP32 (kein MCP23017 - kommt erst, wenn die
//  echte Verkabelung mit allen Modulen ansteht).
// =============================================================================
#pragma once
#include "Io.h"   // liefert IoPin, espPin(), mcpPin()

// =============================================================================
//  MODUS-TASTER  (je EINER pro Etage, schaltet zwischen 5 Modi durch)
//  Werden zentral vom Modul "Taster" eingelesen (Entprellung). Welcher Modus
//  daraus wird (Heizen/Kuehlen/Fenster.../Automatik), entscheidet main.cpp.
// =============================================================================
constexpr IoPin MODUS_TASTER_EG  = espPin(4);
constexpr IoPin MODUS_TASTER_OG1 = espPin(16);
constexpr IoPin MODUS_TASTER_OG2 = espPin(17);

// =============================================================================
//  HEIZUNG  (3 Etagen, je eine LED; LED = simulierte Heizung)
//  Reine Steuerung ueber Heizung::setState() - kein eigener Taster im Modul.
// =============================================================================
constexpr IoPin HEIZUNG_EG_LED  = espPin(2);
constexpr IoPin HEIZUNG_OG1_LED = espPin(15);
constexpr IoPin HEIZUNG_OG2_LED = espPin(18);

// =============================================================================
//  AUFZUG  –  Ruftasten je Etage + Endschalter je Etage + Motortreiber
//  (A4988/DRV8825-Familie: STEP/DIR/ENABLE, Vollschritt, kein Microstepping)
// =============================================================================
constexpr IoPin AUFZUG_TASTER_EG  = espPin(19);
constexpr IoPin AUFZUG_TASTER_OG1 = espPin(21);
constexpr IoPin AUFZUG_TASTER_OG2 = espPin(22);

constexpr IoPin AUFZUG_ENDSCHALTER_EG  = espPin(23);
constexpr IoPin AUFZUG_ENDSCHALTER_OG1 = espPin(25);
constexpr IoPin AUFZUG_ENDSCHALTER_OG2 = espPin(26);

constexpr IoPin AUFZUG_MOTOR_STEP   = espPin(27);
constexpr IoPin AUFZUG_MOTOR_DIR    = espPin(32);
constexpr IoPin AUFZUG_MOTOR_ENABLE = espPin(33);

constexpr uint32_t AUFZUG_STEP_INTERVALL_US = 800;    // Zeit zwischen 2 STEP-Pulsen (Tempo)
constexpr uint32_t AUFZUG_TIMEOUT_MS        = 8000;   // Sicherheits-Abschaltung

// ─── FREIE PINS / RESERVE ────────────────────────────────────────────────────
//  Fuer weitere Module (Kuehlung, Fenster, Jalousie) hier ergaenzen, z.B.:
//      constexpr IoPin KUEHLUNG_EG_LED = espPin(13);