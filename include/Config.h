// =============================================================================
//  Config.h  –  ZENTRALE Konfiguration des Smarthomes
// -----------------------------------------------------------------------------
//  AKTUELLER STAND: Simulationsaufbau auf dem Breadboard.
//  Alle Pins liegen direkt am ESP32 (kein MCP23017 - kommt erst, wenn die
//  echte Verkabelung mit allen Modulen ansteht).
// =============================================================================
#pragma once
#include <Arduino.h>   // uint8_t / uint16_t / uint32_t

// ── Dashboard-Override mit Time-To-Live ─────────────────────────────────────
constexpr uint32_t DASHBOARD_TTL_MS = 3000;   // 3 s (Testwert): Override-Dauer; Pi muss Befehle wiederholen, sonst Automatik
constexpr uint32_t FREEZE_TTL_MS    = 300000; // Automatik-Stopp vom Dashboard: nach 5 min zurueck auf Automatik

// =============================================================================
//  I2C-BUS  (gemeinsam: 2x MCP23017 + PCA9685)
// -----------------------------------------------------------------------------
//  MCP-IN  = nur Eingaenge (alle Pins INPUT_PULLUP, von Io::begin gesetzt)
//  MCP-OUT = nur Ausgaenge (Transistoren, active-high: HIGH = leitet)
//  Adressen ueber A0/A1/A2 am MCP einstellen.
// =============================================================================
constexpr int     PIN_SDA      = 21;
constexpr int     PIN_SCL      = 22;
constexpr uint8_t ADDR_MCP_IN  = 0x20;   // MCP23017 #1 - Eingaenge
constexpr uint8_t ADDR_MCP_OUT = 0x24;   // MCP23017 #2 - Ausgaenge (A2=high; Test mit 1 MCP)
constexpr uint8_t ADDR_PCA9685 = 0x40;   // PWM-Treiber (Servos)

// =============================================================================
//  DOMAIN-TYPEN  (gemeinsame Aufzaehlungen fuer alle Module + Regelung)
// =============================================================================
enum class Etage    : uint8_t { EG = 0, OG1 = 1, OG2 = 2 };
enum class Seite    : uint8_t { LINKS = 0, RECHTS = 1 };
enum class Position : uint8_t { ZU = 0, AUF = 1 };

// =============================================================================
//  DIGITALE AUSGAENGE  (Transistoren am MCP-OUT, active-high)  Index = Etage
// =============================================================================
constexpr uint8_t MCPOUT_HEIZEN[3]  = { 2, 1, 0 };   // rote LED je Etage
constexpr uint8_t MCPOUT_KUEHLEN[3] = { 5, 4, 3 };   // blaue LED je Etage

// =============================================================================
//  DIGITALE EINGAENGE  (am MCP-IN, INPUT_PULLUP; aktiv/geschlossen = LOW)
// =============================================================================
constexpr uint8_t  MCPIN_TASTER[3]   = { 7, 6, 15 };  // Klima-Modus-Taster je Etage
constexpr uint8_t  MCPIN_AUFZUG_TASTER[3] = { 2, 1, 0 };  // Aufzug-Taster je Etage
constexpr uint8_t  MCPIN_REED_AUFZUG[3]   = { 5, 4, 3 }; // Etagen-Reeds EG/OG1/OG2
constexpr uint8_t  MCPIN_REED_AUFZUG_OBEN = 8;           // oberer Ueberfahr-Schalter (Sicherheit)
constexpr uint8_t  MCPIN_REED_TUER   = 10;            // Tuerkontakt
constexpr uint32_t TASTER_TTL_MS     = 10000;        // Hand-Modus -> danach Automatik


// =============================================================================
//  SERVOS am PCA9685  (Kanal + Endlagen-Ticks je Servo - pro Servo EINMESSEN!)
//  Jalousie: [Etage][Seite] | Dachfenster: [Seite] (beide im OG2) | Garage: 1
// =============================================================================
struct ServoEndlage { uint8_t kanal; uint16_t tickZu; uint16_t tickAuf; };

constexpr ServoEndlage SERVO_JALOUSIE[3][2] = {
  { {0, 150, 500}, {1, 150, 500} },   // EG  links / rechts
  { {2, 150, 500}, {3, 150, 500} },   // OG1 links / rechts
  { {4, 150, 500}, {5, 150, 500} },   // OG2 links / rechts
};
constexpr ServoEndlage SERVO_DACHFENSTER[2] = {
  {6, 150, 500},   // OG2 links
  {7, 150, 500},   // OG2 rechts
};
constexpr ServoEndlage SERVO_GARAGE = { 8, 150, 500 };


// =============================================================================
//  LICHT  (12V LED-Strips ueber IRLZ44N-MOSFETs, PWM vom ESP32)
// -----------------------------------------------------------------------------
//  Dimmstufen (global fuer alle Kanaele):
//    Stufe 0 = aus         -> Duty-Cycle 0
//    Stufe 1 = low         -> LOW_BRIGHTNESS_DUTY_CYCLE
//    Stufe 2 = medium      -> MEDIUM_BRIGHTNESS_DUTY_CYCLE
//    Stufe 3 = voll        -> 255
//  Werte 0-255 (analogWrite-Skala).
// =============================================================================
constexpr uint8_t LOW_BRIGHTNESS_DUTY_CYCLE    = 30;
constexpr uint8_t MEDIUM_BRIGHTNESS_DUTY_CYCLE = 120;

constexpr uint8_t LICHT_MAX_KANAELE = 6;

//  GPIO-Pins der 6 MOSFET-Gates (nur PWM-faehige Pins des ESP32 verwenden!)
//  Reihenfolge = Kanal-Index: Kanal 0 -> LICHT_PIN_K0, usw.
constexpr int LICHT_PIN_K0 = 2; // Außen_licht
constexpr int LICHT_PIN_K1 = 4; // Tür_licht
constexpr int LICHT_PIN_K2 = 5; // EG_Licht
constexpr int LICHT_PIN_K3 = 18; // OG1_Licht
constexpr int LICHT_PIN_K4 = 19; // OG2_Licht
constexpr int LICHT_PIN_K5 = 23; //Gewächshaus_Licht

//  PWM-Ticks (0..4095) fuer die Endlagen - pro Servo einmal einmessen!
constexpr uint16_t SERVO_TICK_ZU  = 150;   // ~ 0°
constexpr uint16_t SERVO_TICK_AUF = 500;   // ~ 90°

constexpr float   BESCHATTUNG_HELL_LUX = 800.0f;  // ab hier Jalousie schliessen

// =============================================================================
//  Sensorik (DHT11/22, Ultraschall, Solarpanel)
// =============================================================================
constexpr int SENSORIK_DHT_PIN   = 13;   // DHT11/22-Datenpin (GPIO13)
constexpr int SENSORIK_DHT_TYP   = 11;   // DHT-Typ (11 = DHT11, 22 = DHT22)
constexpr int SENSORIK_SOLAR_PIN = 36;   // Solarpanel ueber 50/50-Teiler an ADC1 (Sensor VP, input-only); dient zugleich als Helligkeit
//  Water Sensor (Gewaechshaus-Bodenfeuchte), analog an ADC1 (input-only).
//  Kalibrieren: trocken in Luft -> TROCKEN-Rohwert, in Wasser -> NASS-Rohwert.
constexpr int SENSORIK_WASSER_PIN     = 34;    // Signal (analog, ADC1_CH6)
constexpr int SENSORIK_WASSER_TROCKEN = 3000;  // ADC-Rohwert trocken (einmessen!)
constexpr int SENSORIK_WASSER_NASS    = 1000;  // ADC-Rohwert nass    (einmessen!)

// =============================================================================
//  DISCOLIGHT  (Stimmungslicht, eigener NeoPixel-Strang)
// =============================================================================
constexpr int      DISCO_STRIP_PIN   = 0;
constexpr uint16_t DISCO_ANZAHL_LEDS = 26;

// =============================================================================
//  AUFZUG  (Schrittmotor 28BYJ-48 ueber ULN2003: IN1-IN4 = Halbschritt-Sequenz)
//  4 benachbarte, freie Output-Pins auf der linken ESP32-Leiste.
// =============================================================================
constexpr int      AUFZUG_IN1 = 14;
constexpr int      AUFZUG_IN2 = 27;
constexpr int      AUFZUG_IN3 = 26;
constexpr int      AUFZUG_IN4 = 25;
constexpr uint32_t AUFZUG_STEP_INTERVALL_US = 800;   // Halbschritt-Takt (kleiner = schneller; <~800us verliert oft Schritte)
constexpr uint32_t AUFZUG_TIMEOUT_MS        = 600000;  // Not-Aus, falls Ziel-Endschalter nie kommt

// =============================================================================
//  WHIRLPOOL  (DC-Motor ueber MOSFET, PWM vom ESP32)
// -----------------------------------------------------------------------------
//  Die Regelung schaltet den Pool nur AN/AUS; im An-Zustand liegt ein FESTER
//  Duty-Cycle an (Drehzahl wird nicht geregelt / nicht vom Dashboard verstellt).
// =============================================================================
constexpr int     WHIRLPOOL_PIN  = 32;   // freier PWM-faehiger Pin
constexpr uint8_t WHIRLPOOL_DUTY = 200;  // fester Duty (0..255), wenn der Pool laeuft

// =============================================================================
//  KLIMAANLAGE / AC  (zentrale Anlage ueber MOSFET, PWM vom ESP32)
// -----------------------------------------------------------------------------
//  KEIN direkter Dashboard-Zugriff: interlocks() bestimmt den Duty anhand der
//  ANZAHL kuehlender Etagen (1/2/3). Werte 0..255 (analogWrite-Skala), einmessen.
// =============================================================================
constexpr int     KLIMA_AC_PIN    = 33;  // freier PWM-faehiger Pin
constexpr uint8_t AC_DUTY_1ETAGE  = 100; // genau 1 Etage kuehlt
constexpr uint8_t AC_DUTY_2ETAGEN = 180; // 2 Etagen kuehlen
constexpr uint8_t AC_DUTY_3ETAGEN = 255; // alle 3 Etagen kuehlen

// ─── FREIE PINS / RESERVE ────────────────────────────────────────────────────
//  Noch frei: GPIO 12 + 15 (Boot-Strapping, mit Vorsicht), 35/39 (nur Eingang).