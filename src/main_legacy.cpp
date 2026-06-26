// =============================================================================
//  main.cpp  –  Orchestrierung & uebergeordnete Logik des Smarthomes
// =============================================================================
#include <Arduino.h>
#include "Config.h"
#include "Io.h"
#include "Taster.h"
#include "Heizung.h"
#include "Aufzug.h"
#include "Servoaktor.h"
#include "Jalousie.h"
#include "Sensorik.h"
#include <Adafruit_PWMServoDriver.h>

// =============================================================================
//  MODI
// =============================================================================
enum class Modus : uint8_t {
  HEIZEN = 0,
  KUEHLEN,
  FENSTER_JALOUSIE_AUF,
  FENSTER_JALOUSIE_ZU,
  AUTOMATIK,
  ANZAHL
};

// ─── PCA9685 ─────────────────────────────────────────────────────────────────
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);

// ─── Servoaktoren (je 2 pro Etage, links + rechts) ───────────────────────────
Servoaktor servoJalousieEgLinks  (pca, JALOUSIE_EG_LINKS_KANAL,   JALOUSIE_TICK_ZU, JALOUSIE_TICK_AUF);
Servoaktor servoJalousieEgRechts (pca, JALOUSIE_EG_RECHTS_KANAL,  JALOUSIE_TICK_ZU, JALOUSIE_TICK_AUF);
Servoaktor servoJalousieOg1Links (pca, JALOUSIE_OG1_LINKS_KANAL,  JALOUSIE_TICK_ZU, JALOUSIE_TICK_AUF);
Servoaktor servoJalousieOg1Rechts(pca, JALOUSIE_OG1_RECHTS_KANAL, JALOUSIE_TICK_ZU, JALOUSIE_TICK_AUF);
Servoaktor servoJalousieDgLinks  (pca, JALOUSIE_DG_LINKS_KANAL,   JALOUSIE_TICK_ZU, JALOUSIE_TICK_AUF);
Servoaktor servoJalousieDgRechts (pca, JALOUSIE_DG_RECHTS_KANAL,  JALOUSIE_TICK_ZU, JALOUSIE_TICK_AUF);

// ─── Jalousie-Module ─────────────────────────────────────────────────────────
Jalousie jalousieEG (servoJalousieEgLinks,  servoJalousieEgRechts);
Jalousie jalousieOG1(servoJalousieOg1Links, servoJalousieOg1Rechts);
Jalousie jalousieDG (servoJalousieDgLinks,  servoJalousieDgRechts);

// ─── Taster ──────────────────────────────────────────────────────────────────
Taster taster(MODUS_TASTER_EG,  MODUS_TASTER_OG1,  MODUS_TASTER_OG2,
              AUFZUG_TASTER_EG, AUFZUG_TASTER_OG1, AUFZUG_TASTER_OG2,
              AUFZUG_ENDSCHALTER_EG, AUFZUG_ENDSCHALTER_OG1, AUFZUG_ENDSCHALTER_OG2,
              JALOUSIE_TASTER_EG, JALOUSIE_TASTER_OG1, JALOUSIE_TASTER_DG);

// ─── Heizung ─────────────────────────────────────────────────────────────────
Heizung heizungEG (HEIZUNG_EG_LED);
Heizung heizungOG1(HEIZUNG_OG1_LED);
Heizung heizungOG2(HEIZUNG_OG2_LED);

// ─── Aufzug ──────────────────────────────────────────────────────────────────
Aufzug aufzug(AUFZUG_MOTOR_STEP, AUFZUG_MOTOR_DIR, AUFZUG_MOTOR_ENABLE,
              AUFZUG_STEP_INTERVALL_US, AUFZUG_TIMEOUT_MS);

// ─── Sensorik ────────────────────────────────────────────────────────────────
Sensorik sensorik;

// ─── Modus je Etage ──────────────────────────────────────────────────────────
Modus modusEG  = Modus::HEIZEN;
Modus modusOG1 = Modus::HEIZEN;
Modus modusOG2 = Modus::HEIZEN;

// ─── Flankenerkennung ────────────────────────────────────────────────────────
bool vorherModusEg  = false;
bool vorherModusOg1 = false;
bool vorherModusOg2 = false;

bool vorherAufzugEg  = false;
bool vorherAufzugOg1 = false;
bool vorherAufzugOg2 = false;

bool vorherJalousieEg  = false;
bool vorherJalousieOg1 = false;
bool vorherJalousieDg  = false;

// ─── Prototypen ──────────────────────────────────────────────────────────────
void modusVerlassen(Modus m, Heizung& heizung);
void modusBetreten (Modus m, Heizung& heizung);
void modusWechseln (Modus& aktuell, Heizung& heizung);

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);

  pca.begin();
  pca.setPWMFreq(50);

  taster.begin();

  heizungEG.begin();
  heizungOG1.begin();
  heizungOG2.begin();

  aufzug.begin();

  jalousieEG.begin();
  jalousieOG1.begin();
  jalousieDG.begin();

  sensorik.begin();

  Serial.println("Smarthome gestartet.");
}

// =============================================================================
//  LOOP
// =============================================================================
void loop() {
  taster.update();
  const TasterStatus& s = taster.status();

  // ── Heizung ───────────────────────────────────────────────────────────────
  if (s.eg  && !vorherModusEg)  modusWechseln(modusEG,  heizungEG);
  if (s.og1 && !vorherModusOg1) modusWechseln(modusOG1, heizungOG1);
  if (s.og2 && !vorherModusOg2) modusWechseln(modusOG2, heizungOG2);

  vorherModusEg  = s.eg;
  vorherModusOg1 = s.og1;
  vorherModusOg2 = s.og2;

  // ── Aufzug ────────────────────────────────────────────────────────────────
  if (s.aufzugEg  && !vorherAufzugEg)  aufzug.fahreZu(Aufzug::Etage::EG);
  if (s.aufzugOg1 && !vorherAufzugOg1) aufzug.fahreZu(Aufzug::Etage::OG1);
  if (s.aufzugOg2 && !vorherAufzugOg2) aufzug.fahreZu(Aufzug::Etage::OG2);

  vorherAufzugEg  = s.aufzugEg;
  vorherAufzugOg1 = s.aufzugOg1;
  vorherAufzugOg2 = s.aufzugOg2;

  aufzug.update(s.endschalterEg, s.endschalterOg1, s.endschalterOg2);

  // ── Jalousien ─────────────────────────────────────────────────────────────
  if (s.jalousieEg  && !vorherJalousieEg)  jalousieEG.umschalten();
  if (s.jalousieOg1 && !vorherJalousieOg1) jalousieOG1.umschalten();
  if (s.jalousieDg  && !vorherJalousieDg)  jalousieDG.umschalten();

  vorherJalousieEg  = s.jalousieEg;
  vorherJalousieOg1 = s.jalousieOg1;
  vorherJalousieDg  = s.jalousieDg;

  // ── Sensorik ──────────────────────────────────────────────────────────────
  sensorik.update();

  if (sensorik.istOk()) {
    Serial.print("Temp: ");
    Serial.print(sensorik.temperatur());
    Serial.print(" C  |  Feuchte: ");
    Serial.print(sensorik.feuchte());
    Serial.println(" %");
  }
}

// =============================================================================
//  MODUS-WECHSEL
// =============================================================================
void modusWechseln(Modus& aktuell, Heizung& heizung) {
  modusVerlassen(aktuell, heizung);
  uint8_t n = (static_cast<uint8_t>(aktuell) + 1) % static_cast<uint8_t>(Modus::ANZAHL);
  aktuell = static_cast<Modus>(n);
  modusBetreten(aktuell, heizung);
}

void modusBetreten(Modus m, Heizung& heizung) {
  switch (m) {
    case Modus::HEIZEN:               heizung.setState(true);  break;
    case Modus::KUEHLEN:              /* TODO */               break;
    case Modus::FENSTER_JALOUSIE_AUF: /* TODO */               break;
    case Modus::FENSTER_JALOUSIE_ZU:  /* TODO */               break;
    case Modus::AUTOMATIK:            /* TODO */               break;
    default: break;
  }
}

void modusVerlassen(Modus m, Heizung& heizung) {
  switch (m) {
    case Modus::HEIZEN:               heizung.setState(false); break;
    case Modus::KUEHLEN:              /* TODO */               break;
    case Modus::FENSTER_JALOUSIE_AUF: /* TODO */               break;
    case Modus::FENSTER_JALOUSIE_ZU:  /* TODO */               break;
    case Modus::AUTOMATIK:            /* TODO */               break;
    default: break;
  }
}