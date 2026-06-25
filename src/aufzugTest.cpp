// =============================================================================
//  aufzugTest.cpp  –  Test der Aufzugssteuerung (28BYJ-48 / ULN2003, IN1-4)
// -----------------------------------------------------------------------------
//  Aktivieren in platformio.ini:  build_src_filter = +<aufzugTest.cpp>
//  Verwendet die ECHTE Aufzug-Klasse (Motor laeuft real ueber IN1-4 = 25/26/27/14).
//  Simuliert werden NUR (mangels Hardware):
//    * Ruftaster      -> serielle Konsole: '0'=EG, '1'=OG1, '2'=OG2, 'q'=quittieren
//    * Etagen-Reeds   -> ZEITBASIERT: nach FAHRZEIT_PRO_ETAGE_MS je Etagen-Abstand
//                        "schliesst" der Ziel-Reed; der Aufzug stoppt dann.
//    * oberer Ueberfahr-Schalter -> 'u' = simuliert "Kabine zu weit oben" -> FEHLER
//  Spaeter werden diese Simulationsbloecke durch DigitalInput ersetzt.
// =============================================================================
#include <Arduino.h>
#include "Config.h"
#include "Aufzug.h"

Aufzug aufzug(AUFZUG_IN1, AUFZUG_IN2, AUFZUG_IN3, AUFZUG_IN4,
              AUFZUG_STEP_INTERVALL_US, AUFZUG_TIMEOUT_MS);

// ── Reed-Simulation (zeitbasiert) ───────────────────────────────────────────
constexpr uint32_t FAHRZEIT_PRO_ETAGE_MS = 4000;  // simulierte Fahrzeit je Etagen-Abstand
bool          gFahrtAktiv = false;
Aufzug::Etage gZiel       = Aufzug::Etage::EG;
uint32_t      gAnkunftMs  = 0;
bool          gUeberfahr  = false;   // einmaliger Impuls fuer den oberen Schalter

static const char* etageName(Aufzug::Etage e) {
  return e == Aufzug::Etage::EG ? "EG" : e == Aufzug::Etage::OG1 ? "OG1" : "OG2";
}
static const char* zustandName(Aufzug::Zustand z) {
  switch (z) {
    case Aufzug::Zustand::STEHT:      return "STEHT";
    case Aufzug::Zustand::FAEHRT_AUF: return "FAEHRT_AUF";
    case Aufzug::Zustand::FAEHRT_AB:  return "FAEHRT_AB";
    default:                          return "FEHLER";
  }
}

// Ruftaste simulieren: Fahrt anfordern und (falls sie startet) den Reed-Timer scharf machen.
static void ruf(Aufzug::Etage ziel) {
  Aufzug::Etage vorher = aufzug.aktuelleEtage();
  aufzug.fahreZu(ziel);
  Aufzug::Zustand z = aufzug.zustand();
  if (z == Aufzug::Zustand::FAEHRT_AUF || z == Aufzug::Zustand::FAEHRT_AB) {
    int dist    = abs((int)ziel - (int)vorher);
    gZiel       = ziel;
    gAnkunftMs  = millis() + FAHRZEIT_PRO_ETAGE_MS * (uint32_t)dist;
    gFahrtAktiv = true;
    Serial.printf("[Ruf] %s -> %s  (sim. Ankunft in %lu ms)\n",
                  etageName(vorher), etageName(ziel),
                  (unsigned long)(FAHRZEIT_PRO_ETAGE_MS * dist));
  } else {
    Serial.printf("[Ruf] ignoriert (Zustand=%s, Etage=%s)\n",
                  zustandName(z), etageName(aufzug.aktuelleEtage()));
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);
  aufzug.begin();
  Serial.println("\n[aufzugTest] 28BYJ-48/ULN2003 an IN1-4 = GPIO 25/26/27/14.");
  Serial.println("Befehle: 0=EG  1=OG1  2=OG2  u=oberer Schalter (Ueberfahren)  q=Fehler quittieren");
}

void loop() {
  // 1) Ruftaster + oberer Schalter simulieren (serielle Konsole)
  if (Serial.available()) {
    char c = (char)Serial.read();
    if      (c == '0') ruf(Aufzug::Etage::EG);
    else if (c == '1') ruf(Aufzug::Etage::OG1);
    else if (c == '2') ruf(Aufzug::Etage::OG2);
    else if (c == 'u') { gUeberfahr = true; Serial.println("[Sim] oberer Ueberfahr-Schalter ausgeloest"); }
    else if (c == 'q') { aufzug.fehlerQuittieren(); Serial.println("[Quittiert]"); }
  }

  // 2) Etagen-Reeds simulieren: Ziel-Reed schliesst nach Fahrzeit
  bool reed[3] = { false, false, false };
  if (gFahrtAktiv && (int32_t)(millis() - gAnkunftMs) >= 0)
    reed[(int)gZiel] = true;

  // 3) Aufzug-Steuerung: taktet den realen Motor, prueft Reeds + Ueberfahr + Timeout
  aufzug.update(reed[0], reed[1], reed[2], gUeberfahr);
  gUeberfahr = false;   // Einmal-Impuls

  // Fahrt-Flag zuruecksetzen, sobald nicht mehr gefahren wird (STEHT oder FEHLER)
  if (aufzug.zustand() != Aufzug::Zustand::FAEHRT_AUF &&
      aufzug.zustand() != Aufzug::Zustand::FAEHRT_AB)
    gFahrtAktiv = false;

  // 4) Zustandswechsel ausgeben
  static Aufzug::Zustand letzterZ = Aufzug::Zustand::STEHT;
  static Aufzug::Etage   letzteE  = Aufzug::Etage::EG;
  if (aufzug.zustand() != letzterZ || aufzug.aktuelleEtage() != letzteE) {
    letzterZ = aufzug.zustand();
    letzteE  = aufzug.aktuelleEtage();
    Serial.printf("[Status] %s, Etage=%s\n", zustandName(letzterZ), etageName(letzteE));
  }
}
