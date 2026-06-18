// =============================================================================
//  discoTest.cpp  –  Schnelltest fuer das DiscoLight-Modul
// -----------------------------------------------------------------------------
//  Startet sofort den Disco-Modus und haelt ihn am Laufen.
//  Optional: Einen Taster an MODUS_TASTER_EG druecken, um das Licht
//  an- und auszuschalten (Flankenerkennung, kein delay()).
//
//  WICHTIG:  In platformio.ini die build_src_filter-Zeile anpassen:
//    build_src_filter = +<discoTest.cpp>
//
//  Pinbelegung und LED-Anzahl kommen aus Config.h:
//    DISCO_STRIP_PIN   = 18
//    DISCO_ANZAHL_LEDS = 8
// =============================================================================
#include <Arduino.h>
#include "Config.h"
#include "DiscoLight.h"
 
// ── Objekt ────────────────────────────────────────────────────────────────────
DiscoLight disco(DISCO_STRIP_PIN, DISCO_ANZAHL_LEDS);
 
// ── Taster-Status (optional – Taster an MODUS_TASTER_EG = GPIO 4) ────────────
static bool vorherTaster = false;
 
// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
 
  disco.begin();
  disco.an();           // Disco sofort starten
 
  Serial.println("=== DiscoLight Test ===");
  Serial.println("Taster an GPIO 4 druecken -> Disco an/aus umschalten");
}
 
// =============================================================================
//  LOOP  –  nie blockieren!
// =============================================================================
void loop() {
 
  // ── DiscoLight ticken lassen – das ist der einzige Pflichtaufruf ──────────
  disco.update();
  delay(8);
}