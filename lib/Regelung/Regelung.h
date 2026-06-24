// =============================================================================
//  Regelung  –  das "Gehirn" des Smarthomes (Soll-Zustand + Schichten)
// -----------------------------------------------------------------------------
//  KONZEPT (Details: ARCHITECTURE.md):
//   * Jede Loop wird ein FRISCHER Soll-Zustand gebildet.
//   * Regeln schreiben mit setze(feld, wert, PRIO) hinein - die HOECHSTE Prio
//     pro Feld gewinnt. Die Aufrufreihenfolge ist daher egal, nur die Prio zaehlt.
//   * Danach loest interlocks() Konflikte prioritaetsbewusst auf.
//   * main.cpp baut den Kontext, ruft die Schichten + interlocks SICHTBAR auf
//     und uebertraegt den Soll mit anwenden() auf die (dummen) Module.
//
//  Dieses Modul ist BEWUSST hardware-frei (kein Io, keine Aktoren) - dadurch
//  ist die Logik leicht testbar und unabhaengig von der Verkabelung.
// =============================================================================
#pragma once
#include <Arduino.h>

// ─── Prioritaets-Baender (frei justierbar) ───────────────────────────────────
constexpr uint8_t PRIO_ZEIT_BASIS  = 20;   // Grundverhalten nach Tageszeit
constexpr uint8_t PRIO_SENSOR      = 40;   // Sensor-Regeln (Hysterese, dunkel...)
constexpr uint8_t PRIO_ZEIT_SPERRE = 60;   // Tageszeit-Sperren (schlagen Sensor)
constexpr uint8_t PRIO_DASHBOARD   = 80;   // Dashboard-Befehle (z.B. Pi)
constexpr uint8_t PRIO_HAND        = 100;  // Handeingriff (Etagen-Taster)
// (Reserviert: PRIO_DASHBOARD spaeter zwischen 90 und 110)

// ─── Regelungs-Parameter ─────────────────────────────────────────────────────
//  TODO: bei Bedarf nach include/Config.h verschieben (zentrale Tuning-Stelle).
constexpr uint32_t TAG_LAENGE_MS   = 60000;   // 1 simulierter Tag = 10 min real
constexpr float    SOLL_TEMP_TAG   = 21.0f;    // °C Sollwert tagsueber
constexpr float    SOLL_TEMP_NACHT = 18.0f;    // °C Sollwert nachts
constexpr float    HYSTERESE       = 0.5f;     // °C +/- um den Sollwert
constexpr float    DUNKEL_LUX      = 50.0f;    // darunter gilt es als "dunkel"
constexpr float    SONNE_HELL_LUX  = 800.0f;   // ab hier beschatten

// ─── Etagen / Indizes ────────────────────────────────────────────────────────
constexpr uint8_t ANZ_ETAGEN = 3;   // 0=EG, 1=OG1, 2=OG2
constexpr uint8_t ANZ_SEITEN = 2;   // 0=links, 1=rechts
constexpr uint8_t ANZ_LICHT  = 6;   // K0=EG,K1=OG1,K2=OG2,K3=Terrasse,K4=Eingang,K5=frei
constexpr uint8_t OG2        = 2;   // Index der 2. Etage (Disco/Dachfenster)

// ─── Datentypen ──────────────────────────────────────────────────────────────
enum class Phase      : uint8_t { NACHT, MORGEN, TAG, ABEND };
enum class KlimaModus : uint8_t { AUTOMATIK, HEIZEN, KUEHLEN };

// Ein Soll-Feld: Wert + die Prioritaet, mit der er gesetzt wurde.
struct Feld {
  int16_t wert = 0;   // generisch: 0/1, Prozent (Jalousie/Klappe) oder Stufe (Licht)
  uint8_t prio = 0;   // 0 = noch von niemandem gesetzt
};

// Der gewuenschte Gesamtzustand des Hauses (wird jede Loop neu gebaut).
struct Soll {
  Feld heizung[ANZ_ETAGEN];               // 0/1  rote LED
  Feld kuehlLed[ANZ_ETAGEN];              // 0/1  blaue LED
  Feld klappe[ANZ_ETAGEN];                // 0=zu,100=auf  (Luftklappe je Etage)
  Feld klimaanlage;                       // 0/1  zentrale Anlage (Dach)
  Feld dachfensterOG2;                    // 0=zu,100=auf
  Feld jalousie[ANZ_ETAGEN][ANZ_SEITEN];  // 0=offen,100=ganz beschattet
  Feld licht[ANZ_LICHT];                  // Stufe 0..3
  Feld disco;                             // 0/1 (nur OG2)
};

// Eingangsgroessen fuer die Regeln (baut main jede Loop aus Tastern/Sensoren).
struct Kontext {
  float      stunde      = 0.0f;          // simulierte Uhrzeit 0..24
  Phase      phase       = Phase::TAG;
  float      temperatur[ANZ_ETAGEN] = { 21.0f, 21.0f, 21.0f };
  float      helligkeit  = 100.0f;        // Lux
  bool       bewegung    = false;         // PIR (vorerst vereinfacht global)
  int8_t     sonnenSeite = -1;            // -1 keine, 0 links, 1 rechts
  KlimaModus klimaModus[ANZ_ETAGEN] = { KlimaModus::AUTOMATIK, KlimaModus::AUTOMATIK, KlimaModus::AUTOMATIK };
  bool       discoWunsch = false;
};

// ─── Dashboard-Override (Befehle vom Raspberry Pi) ───────────────────────────
// DASHBOARD_TTL_MS liegt in Config.h (zentrale Tuning-Stelle).
struct DashBefehl {
  int8_t   wert     = -1;   // -1 = inaktiv (kein Override)
  uint32_t deadline = 0;    // millis()-Zeitpunkt, ab dem der Befehl verfaellt
};

struct DashboardState {
  DashBefehl blind[ANZ_ETAGEN][ANZ_SEITEN];
  DashBefehl heat[ANZ_ETAGEN];
  DashBefehl light[ANZ_ETAGEN];
  DashBefehl mode[ANZ_ETAGEN];
  DashBefehl party, ac, skylight2;
  DashBefehl tv, skylight1, garage, front_door, elevator;  // (noch) ohne Soll-Feld
};

// ─── Kern-Helfer ─────────────────────────────────────────────────────────────
// Schreibt nur, wenn prio >= bisher gesetzter Prio (hoechste Prio gewinnt).
inline void setze(Feld& f, int16_t wert, uint8_t prio) {
  if (prio >= f.prio) { f.wert = wert; f.prio = prio; }
}
void dashSetze(DashBefehl& b, int wert);              // Wert setzen + TTL aufziehen

float  berechneTageszeit(uint32_t jetztMs, uint32_t tagLaengeMs);   // -> 0..24 h
Phase  phaseAus(float stunde);
int8_t sonnenSeiteAus(float stunde);    // -1/0/1
float  sollTemperatur(Phase p);

// ─── Schichten (in main SICHTBAR nacheinander aufrufen) ──────────────────────
void tageszeitRegeln(const Kontext& k, Soll& s);   // Basis (10) + Sperren (60)
void dashboardRegeln(Soll& s, DashboardState& dash);  // Pi-Befehle, dash wird mutiert (TTL)
void sensorRegeln  (const Kontext& k, Soll& s);    // 50
void handRegeln    (const Kontext& k, Soll& s);    // 100
void interlocks    (Soll& s);                      // Konflikte prioritaetsbewusst
