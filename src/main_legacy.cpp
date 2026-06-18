// =============================================================================
//  main.cpp  –  Orchestrierung & uebergeordnete Logik des Smarthomes
// -----------------------------------------------------------------------------
//  AKTUELLER STAND: Simulationsaufbau auf dem Breadboard. Module: Heizung,
//  Aufzug. Beide sind reine STEUERUNGSKLASSEN - sie lesen NIE selbst einen
//  Taster/Endschalter aus. Das macht ausschliesslich das Modul "Taster"
//  (Entprellung), main.cpp wertet die bereinigten Pegel aus und ruft gezielt
//  die passenden Funktionen der Module auf.
//
//  AUFZUG: main.cpp erkennt die Flanke auf den 3 Ruftasten (vorher nicht
//  gedrueckt, jetzt gedrueckt) und ruft daraufhin aufzug.fahreZu(etage) auf.
//  Die 3 Endschalter werden bei JEDEM loop()-Durchlauf an aufzug.update()
//  weitergereicht - der Aufzug haelt selbst Buch, wann er anhalten muss.
// =============================================================================
#include <Arduino.h>

#include "Config.h"
#include "Io.h"
#include "Taster.h"
#include "Heizung.h"
#include "Aufzug.h"

// =============================================================================
//  MODI  –  Reihenfolge, durch die der Modus-Taster jeder Etage durchschaltet
// =============================================================================
enum class Modus : uint8_t {
  HEIZEN = 0,
  KUEHLEN,
  FENSTER_JALOUSIE_AUF,
  FENSTER_JALOUSIE_ZU,
  AUTOMATIK,
  ANZAHL
};

// ─── Module / Objekte ────────────────────────────────────────────────────────
Taster taster(MODUS_TASTER_EG, MODUS_TASTER_OG1, MODUS_TASTER_OG2,
              AUFZUG_TASTER_EG, AUFZUG_TASTER_OG1, AUFZUG_TASTER_OG2,
              AUFZUG_ENDSCHALTER_EG, AUFZUG_ENDSCHALTER_OG1, AUFZUG_ENDSCHALTER_OG2);

Heizung heizungEG (HEIZUNG_EG_LED);
Heizung heizungOG1(HEIZUNG_OG1_LED);
Heizung heizungOG2(HEIZUNG_OG2_LED);

Aufzug aufzug(AUFZUG_MOTOR_STEP, AUFZUG_MOTOR_DIR, AUFZUG_MOTOR_ENABLE,
              AUFZUG_STEP_INTERVALL_US, AUFZUG_TIMEOUT_MS);

// ─── aktueller Modus je Etage (Heizung/Kuehlen/...) ──────────────────────────
Modus modusEG  = Modus::HEIZEN;
Modus modusOG1 = Modus::HEIZEN;
Modus modusOG2 = Modus::HEIZEN;

// ─── Flankenerkennung: vorheriger bereinigter Pegel, um NEUE Tastendruecke zu
//     erkennen (vorher false, jetzt true). main.cpp macht das selbst -
//     Taster liefert nur den aktuellen Pegel.
// =============================================================================
bool vorherModusEg  = false;
bool vorherModusOg1 = false;
bool vorherModusOg2 = false;

bool vorherAufzugEg  = false;
bool vorherAufzugOg1 = false;
bool vorherAufzugOg2 = false;

// ─── Prototypen ──────────────────────────────────────────────────────────────
void modusVerlassen(Modus m, Heizung& heizung);
void modusBetreten  (Modus m, Heizung& heizung);
void modusWechseln  (Modus& aktuell, Heizung& heizung);

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);     // Debug-Konsole (USB)

  taster.begin();

  heizungEG.begin();
  heizungOG1.begin();
  heizungOG2.begin();

  aufzug.begin();
}

// =============================================================================
//  LOOP  –  kooperativ, nie blockieren (kein delay()!)
// =============================================================================
void loop() {
  taster.update();                              // 1x pro loop: alle Eingaenge entprellt einlesen
  const TasterStatus& s = taster.status();       // bereinigte Pegel, true = ausgeloest/gehalten

  // ── Heizung: Modus-Taster je Etage, Flankenerkennung in main.cpp ──────────
  if (s.eg  && !vorherModusEg)  modusWechseln(modusEG,  heizungEG);
  if (s.og1 && !vorherModusOg1) modusWechseln(modusOG1, heizungOG1);
  if (s.og2 && !vorherModusOg2) modusWechseln(modusOG2, heizungOG2);

  vorherModusEg  = s.eg;
  vorherModusOg1 = s.og1;
  vorherModusOg2 = s.og2;

  // ── Aufzug: Ruftasten je Etage, Flankenerkennung in main.cpp ──────────────
  if (s.aufzugEg  && !vorherAufzugEg)  aufzug.fahreZu(Aufzug::Etage::EG);
  if (s.aufzugOg1 && !vorherAufzugOg1) aufzug.fahreZu(Aufzug::Etage::OG1);
  if (s.aufzugOg2 && !vorherAufzugOg2) aufzug.fahreZu(Aufzug::Etage::OG2);

  vorherAufzugEg  = s.aufzugEg;
  vorherAufzugOg1 = s.aufzugOg1;
  vorherAufzugOg2 = s.aufzugOg2;

  // Endschalter werden bei JEDEM Durchlauf weitergereicht, nicht nur bei
  // einer Flanke - der Aufzug muss laufend pruefen, ob er anhalten muss.
  aufzug.update(s.endschalterEg, s.endschalterOg1, s.endschalterOg2);
}

// =============================================================================
//  MODUS-WECHSEL (Heizung)  –  exklusiv: alter Modus wird beendet, neuer
//  gestartet. Hier werden GEZIELT die Funktionen der einzelnen Module
//  aufgerufen.
// =============================================================================
void modusWechseln(Modus& aktuell, Heizung& heizung) {
  modusVerlassen(aktuell, heizung);

  uint8_t naechster = (static_cast<uint8_t>(aktuell) + 1) % static_cast<uint8_t>(Modus::ANZAHL);
  aktuell = static_cast<Modus>(naechster);

  modusBetreten(aktuell, heizung);
}

void modusBetreten(Modus m, Heizung& heizung) {
  switch (m) {
    case Modus::HEIZEN:
      heizung.setState(true);
      break;
    case Modus::KUEHLEN:
      // TODO: kuehlung.setState(true), sobald das Modul "Kuehlung" existiert
      break;
    case Modus::FENSTER_JALOUSIE_AUF:
      // TODO: fenster.setState(true); jalousie.setState(true);
      break;
    case Modus::FENSTER_JALOUSIE_ZU:
      // TODO: fenster.setState(false); jalousie.setState(false);
      break;
    case Modus::AUTOMATIK:
      // TODO: automatik-Logik aktivieren
      break;
    default:
      break;
  }
}

void modusVerlassen(Modus m, Heizung& heizung) {
  switch (m) {
    case Modus::HEIZEN:
      heizung.setState(false);
      break;
    case Modus::KUEHLEN:
      // TODO: kuehlung.setState(false);
      break;
    case Modus::FENSTER_JALOUSIE_AUF:
    case Modus::FENSTER_JALOUSIE_ZU:
      // TODO: ggf. nichts zu tun
      break;
    case Modus::AUTOMATIK:
      // TODO: Automatik-Logik deaktivieren
      break;
    default:
      break;
  }
}