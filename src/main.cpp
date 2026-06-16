// =============================================================================
//  main.cpp  –  Orchestrierung & uebergeordnete REGELUNGSLOGIK des Smarthomes
// -----------------------------------------------------------------------------
//  Aufbau:
//   * Oben werden alle Module/Objekte angelegt (Pins/Parameter aus Config.h).
//   * setup()  initialisiert IO-Basis (I2C/MCP, PCA9685) und dann die Module.
//   * loop()   ist kooperativ und NICHT-blockierend: jedes Modul hat update();
//              KEIN delay() verwenden!
//   * regelung()        enthaelt die ETAGEN-/MODUL-UEBERGREIFENDEN Automatik-Regeln.
//   * verarbeiteBefehl() setzt Befehle vom Raspberry Pi um.
//   * statusSenden()    meldet zyklisch Zustaende ans Dashboard.
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#include "Config.h"
#include "Io.h"
#include "Sensorik.h"
#include "Heizung.h"
#include "Aufzug.h"
#include "Licht.h"
#include "DiscoLight.h"
#include "Servoaktor.h"
#include "Alarm.h"
#include "Kommunikation.h"

// ─── Gemeinsame Treiber ──────────────────────────────────────────────────────
Adafruit_PWMServoDriver pwm(ADDR_PCA9685);     // Servo-PWM-Treiber (PCA9685)

// ─── Module / Objekte ────────────────────────────────────────────────────────
Sensorik sensorik;

Heizung heizungEG (HEIZUNG_EG_TASTER,  HEIZUNG_EG_LED,  HEIZUNG_SOLL_DEFAULT, HEIZUNG_HYSTERESE);
Heizung heizungOG1(HEIZUNG_OG1_TASTER, HEIZUNG_OG1_LED, HEIZUNG_SOLL_DEFAULT, HEIZUNG_HYSTERESE);
Heizung heizungOG2(HEIZUNG_OG2_TASTER, HEIZUNG_OG2_LED, HEIZUNG_SOLL_DEFAULT, HEIZUNG_HYSTERESE);

Aufzug aufzug(AUFZUG_ENDSCHALTER_OBEN, AUFZUG_ENDSCHALTER_UNTEN,
              AUFZUG_MOTOR_RICHTUNG,   AUFZUG_MOTOR_ENABLE,
              AUFZUG_ZEIT_EG_OG1_MS,   AUFZUG_ZEIT_OG1_OG2_MS, AUFZUG_TIMEOUT_MS);

Licht      licht(LICHT_STRIP_PIN, LICHT_ANZAHL_LEDS);
DiscoLight disco(DISCO_STRIP_PIN, DISCO_ANZAHL_LEDS);

Servoaktor dachfensterOG2(pwm, SERVO_DACHFENSTER_OG2, SERVO_TICK_ZU, SERVO_TICK_AUF);
Servoaktor dachfensterOG1(pwm, SERVO_DACHFENSTER_OG1, SERVO_TICK_ZU, SERVO_TICK_AUF);
Servoaktor jalousieEG    (pwm, SERVO_JALOUSIE_EG,     SERVO_TICK_ZU, SERVO_TICK_AUF);
Servoaktor jalousieOG1   (pwm, SERVO_JALOUSIE_OG1,    SERVO_TICK_ZU, SERVO_TICK_AUF);
Servoaktor garagentor    (pwm, SERVO_GARAGENTOR,      SERVO_TICK_ZU, SERVO_TICK_AUF);

Alarm alarmAnlage(ALARM_RELAIS, ALARM_BUZZER);   // 'alarm' ist eine POSIX-Funktion -> anderer Name

Kommunikation komm(Serial2);   // Link zum Raspberry Pi (UART2). USB-Variante: komm(Serial)

// ─── Prototypen ──────────────────────────────────────────────────────────────
void regelung();
void verarbeiteBefehl(JsonDocument& doc);
void statusSenden();

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);                              // Debug-Konsole (USB)
  Serial2.begin(COMM_BAUD, SERIAL_8N1, PIN_PI_RX, PIN_PI_TX);  // Link zum Pi

  // Erst die IO-/I2C-Basis, DANN die Module initialisieren.
  io.begin(PIN_SDA, PIN_SCL, ADDR_MCP23017);
  pwm.begin();
  pwm.setPWMFreq(50);                                // 50 Hz fuer Servos

  sensorik.begin();

  heizungEG.begin();
  heizungOG1.begin();
  heizungOG2.begin();

  aufzug.begin();

  licht.begin();
  disco.begin();

  dachfensterOG2.begin();
  dachfensterOG1.begin();
  jalousieEG.begin();
  jalousieOG1.begin();
  garagentor.begin();

  alarmAnlage.begin();

  komm.begin();
  komm.onBefehl(verarbeiteBefehl);                   // Befehle vom Pi -> Handler

  aufzug.referenzieren();                            // einmal Position suchen
}

// =============================================================================
//  LOOP  –  kooperativ, nie blockieren (kein delay()!)
// =============================================================================
void loop() {
  // 1) Sensoren aktualisieren
  sensorik.update();

  // 2) aktuelle Messwerte in die Regler reichen
  heizungEG.setIstTemperatur (sensorik.temperatur(0));
  heizungOG1.setIstTemperatur(sensorik.temperatur(1));
  heizungOG2.setIstTemperatur(sensorik.temperatur(2));

  // 3) Module abarbeiten (jedes hat sein eigenes Timing per millis())
  heizungEG.update();
  heizungOG1.update();
  heizungOG2.update();
  aufzug.update();
  licht.update();
  disco.update();
  alarmAnlage.update();

  // 4) eingehende Befehle vom Pi
  komm.update();

  // 5) uebergeordnete Regelungslogik / Automatik-Regeln
  regelung();

  // 6) Status zyklisch ans Dashboard senden
  statusSenden();
}

// =============================================================================
//  REGELUNGSLOGIK  –  ETAGEN-/MODUL-UEBERGREIFENDE Regeln.
//  (Einzelregelung wie die Heizungs-Hysterese steckt im jeweiligen Modul.)
//  Neue Automatik-Regeln einfach hier als weiteren Block ergaenzen.
// =============================================================================
void regelung() {
  // ── Regel 1: Dachfenster im obersten Geschoss schliessen, wenn dort geheizt wird
  if (heizungOG2.istAn()) {
    dachfensterOG2.zu();
  }

  // ── Regel 2: Licht nur bei Bewegung UND Dunkelheit
  bool bewegung = (io.digitalRead(LICHT_PIR) == HIGH);
  bool dunkel   = (sensorik.helligkeit() < LICHT_DUNKEL_LUX);
  if (bewegung && dunkel) licht.an();
  else if (!bewegung)     licht.aus();

  // ── Regel 3: Alarm bei Tuer-/Fensterkontakt, wenn scharf geschaltet
  bool tuerOffen    = (io.digitalRead(REED_TUER_EG)    == HIGH);
  bool fensterOffen = (io.digitalRead(REED_FENSTER_OG) == HIGH);
  if (tuerOffen || fensterOffen) alarmAnlage.ausloesen();

  // ── Regel 4: Beschattung nach Helligkeit (Beispiel: Jalousie EG)
  if (sensorik.helligkeit() > BESCHATTUNG_HELL_LUX) jalousieEG.zu();
  else                                              jalousieEG.auf();

  // TODO: weitere Regeln hier ergaenzen.
}

// =============================================================================
//  BEFEHLE vom Raspberry Pi  (JSON-Zeilen, siehe README -> "Protokoll")
//  Beispiel:  {"cmd":"heizung","raum":"og2","set":1}
// =============================================================================
void verarbeiteBefehl(JsonDocument& doc) {
  const char* cmd = doc["cmd"] | "";

  if (strcmp(cmd, "heizung") == 0) {
    const char* raum = doc["raum"] | "";
    bool an = doc["set"] | false;
    Heizung* h = nullptr;
    if      (strcmp(raum, "eg")  == 0) h = &heizungEG;
    else if (strcmp(raum, "og1") == 0) h = &heizungOG1;
    else if (strcmp(raum, "og2") == 0) h = &heizungOG2;
    if (h) { if (an) h->einschaltenManuell(); else h->setModus(HeizModus::AUTO); }
  }
  else if (strcmp(cmd, "aufzug") == 0) {
    int etage = doc["etage"] | 0;
    aufzug.fahreZu((Aufzug::Etage)etage);
  }
  else if (strcmp(cmd, "disco") == 0) {
    bool an = doc["set"] | false;
    if (an) disco.an(); else disco.aus();
  }
  else if (strcmp(cmd, "alarm") == 0) {
    bool scharf = doc["set"] | false;
    if (scharf) alarmAnlage.scharfschalten(); else alarmAnlage.entschaerfen();
  }
  // TODO: weitere Befehle ergaenzen (jalousie, dachfenster, garage, licht, ...).
}

// =============================================================================
//  STATUS an den Pi  (nur alle TAKT_STATUS_SENDEN_MS, nicht-blockierend)
// =============================================================================
void statusSenden() {
  static uint32_t letzt = 0;
  if (millis() - letzt < TAKT_STATUS_SENDEN_MS) return;
  letzt = millis();

  komm.sendeStatus("temp_eg",      sensorik.temperatur(0));
  komm.sendeStatus("heizung_og2",  heizungOG2.istAn());
  komm.sendeStatus("aufzug_etage", (float)aufzug.etage());
  // TODO: weitere Statuswerte ergaenzen.
}
