#include "DigitalInput.h"
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

namespace {
  Adafruit_MCP23X17 mcpIn;      // dieses Modul besitzt den Eingangs-Chip
  bool gOk = false;

  constexpr uint8_t  N           = (uint8_t)Eingang::ANZAHL;
  constexpr uint32_t ENTPRELL_MS = 50;

  // Eingang -> MCP-IN-Pin (aus Config). Neuer Eingang -> hier ergaenzen.
  uint8_t pinVon(uint8_t i) {
    switch ((Eingang)i) {
      case Eingang::TASTER_EG:   return MCPIN_TASTER[0];
      case Eingang::TASTER_OG1:  return MCPIN_TASTER[1];
      case Eingang::TASTER_OG2:  return MCPIN_TASTER[2];
      case Eingang::AUFZUG_TASTER_EG:  return MCPIN_AUFZUG_TASTER[0];
      case Eingang::AUFZUG_TASTER_OG1: return MCPIN_AUFZUG_TASTER[1];
      case Eingang::AUFZUG_TASTER_OG2: return MCPIN_AUFZUG_TASTER[2];
      case Eingang::REED_TUER:        return MCPIN_REED_TUER;
      case Eingang::REED_AUFZUG_EG:   return MCPIN_REED_AUFZUG[0];
      case Eingang::REED_AUFZUG_OG1:  return MCPIN_REED_AUFZUG[1];
      case Eingang::REED_AUFZUG_OG2:  return MCPIN_REED_AUFZUG[2];
      case Eingang::REED_AUFZUG_OBEN: return MCPIN_REED_AUFZUG_OBEN;
      default:                        return 0;
    }
  }

  int      gRoh[N];        // zuletzt gelesener Rohpegel
  uint32_t gWechsel[N];    // Zeitpunkt der letzten Rohpegel-Aenderung
  bool     gStabil[N];     // entprellter Zustand (true = aktiv = LOW)
  bool     gFlanke[N];     // true im Update, in dem gStabil false->true ging
}

void digitalInputBegin() {
  gOk = mcpIn.begin_I2C(ADDR_MCP_IN, &Wire);
  if (gOk)
    for (uint8_t p = 0; p < 16; p++) mcpIn.pinMode(p, INPUT_PULLUP);

  for (uint8_t i = 0; i < N; i++) {
    gRoh[i] = HIGH; gWechsel[i] = 0; gStabil[i] = false; gFlanke[i] = false;
  }
}

bool digitalInputOk() { return gOk; }

void digitalInputUpdate() {
  uint32_t jetzt = millis();
  for (uint8_t i = 0; i < N; i++) {
    int roh = gOk ? mcpIn.digitalRead(pinVon(i)) : HIGH;
    if (roh != gRoh[i]) { gRoh[i] = roh; gWechsel[i] = jetzt; }

    bool vorher = gStabil[i];
    bool neu    = vorher;
    if (jetzt - gWechsel[i] > ENTPRELL_MS) neu = (roh == LOW);   // aktiv = LOW (Pullup)

    gFlanke[i] = (neu && !vorher);   // steigende Flanke: gerade aktiv geworden
    gStabil[i] = neu;
  }
}

bool gedrueckt(Eingang e)       { return gStabil[(uint8_t)e]; }
bool geradeGedrueckt(Eingang e) { return gFlanke[(uint8_t)e]; }

// Roh (UNentprellt): unmittelbarer Pegel des letzten Reads. Fuer schnelle
// Endschalter/Reeds, deren Impuls bei fahrender Kabine kuerzer als ENTPRELL_MS
// sein kann (sonst wuerde der Reed nie "stabil" und der Aufzug faehrt vorbei).
bool gedruecktRoh(Eingang e)    { return gOk && (gRoh[(uint8_t)e] == LOW); }
