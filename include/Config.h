// =============================================================================
//  Config.h  –  ZENTRALE Konfiguration des Smarthomes
// -----------------------------------------------------------------------------
//  HIER (und nur hier) werden Pin-Belegungen, I2C-Adressen und globale
//  Parameter eingetragen. Wer Hardware umsteckt oder Werte tunt, aendert
//  ausschliesslich diese Datei - die Module bleiben unveraendert.
//
//  Pin-Schreibweise (siehe Io-Schicht):
//     espPin(13)  -> GPIO 13 direkt am ESP32
//     mcpPin(0)   -> Pin 0 (GPA0) am Port-Expander MCP23017
//  Ein Modul weiss NICHT, ob sein Pin am ESP32 oder am MCP haengt - das
//  entscheidet allein diese Datei. Umstecken = nur hier eine Zeile aendern.
// =============================================================================
#pragma once
#include "Io.h"   // liefert IoPin, espPin(), mcpPin()

// ─── I2C-Bus (gemeinsam: MCP23017, PCA9685, BH1750, SHT31) ───────────────────
constexpr int     PIN_SDA = 21;
constexpr int     PIN_SCL = 22;

constexpr uint8_t ADDR_MCP23017 = 0x20;   // Port-Expander (Taster/Schaltausgaenge)
constexpr uint8_t ADDR_PCA9685  = 0x40;   // PWM-Treiber (Servos)
constexpr uint8_t ADDR_BH1750   = 0x23;   // Helligkeitssensor
constexpr uint8_t ADDR_SHT31    = 0x44;   // Temperatur/Feuchte

// ─── Serielle Verbindung zum Raspberry Pi (Dashboard) ────────────────────────
//  Eigener UART (UART2), damit die USB-Konsole frei fuer Debug/Flash bleibt.
//  Alternative: ESP32 direkt per USB an den Pi -> in main.cpp Serial2 durch
//  Serial ersetzen.
constexpr int      PIN_PI_RX = 16;        // ESP32 RX2  <- Pi TX
constexpr int      PIN_PI_TX = 17;        // ESP32 TX2  -> Pi RX
constexpr uint32_t COMM_BAUD = 115200;

// ─── 1-Wire (DS18B20 Temperatursensoren) ─────────────────────────────────────
constexpr int     PIN_DS18B20 = 4;

// =============================================================================
//  HEIZUNG  (3 Etagen, je Taster + LED; LED = simulierte Heizung)
//  Taster -> Eingaenge am MCP   |   Heiz-LED -> Schaltausgaenge am MCP
// =============================================================================
constexpr IoPin HEIZUNG_EG_TASTER  = mcpPin(0);
constexpr IoPin HEIZUNG_OG1_TASTER = mcpPin(1);
constexpr IoPin HEIZUNG_OG2_TASTER = mcpPin(2);

constexpr IoPin HEIZUNG_EG_LED  = mcpPin(8);
constexpr IoPin HEIZUNG_OG1_LED = mcpPin(9);
constexpr IoPin HEIZUNG_OG2_LED = mcpPin(10);

constexpr float HEIZUNG_SOLL_DEFAULT = 21.0f;   // °C Solltemperatur (Automatik)
constexpr float HEIZUNG_HYSTERESE    = 0.5f;    // °C +/- um den Sollwert

// =============================================================================
//  AUFZUG  (Brushed-DC ueber Seilwinde, einfacher Treiber: Richtung + Enable)
// =============================================================================
constexpr IoPin AUFZUG_ENDSCHALTER_OBEN  = espPin(32);  // schliesst ganz oben (OG2)
constexpr IoPin AUFZUG_ENDSCHALTER_UNTEN = espPin(33);  // schliesst ganz unten (EG)
constexpr IoPin AUFZUG_MOTOR_RICHTUNG    = espPin(25);  // HIGH = auf, LOW = ab
constexpr IoPin AUFZUG_MOTOR_ENABLE      = espPin(26);  // HIGH = Motor laeuft

//  Fahrzeiten zwischen BENACHBARTEN Etagen (durch Ausprobieren kalibrieren!)
constexpr uint32_t AUFZUG_ZEIT_EG_OG1_MS  = 2500;
constexpr uint32_t AUFZUG_ZEIT_OG1_OG2_MS = 2500;
//  Sicherheit: spaetestens nach dieser Zeit MUSS ein Endschalter ausloesen,
//  sonst Not-Stopp (Seil gerissen / blockiert / Motor dreht durch).
constexpr uint32_t AUFZUG_TIMEOUT_MS      = 6000;

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

//  Anzahl Kanaele (Hardware-Maximum: 6 – softwareseitig fest verdrahtet)
constexpr uint8_t LICHT_MAX_KANAELE = 6;

//  GPIO-Pins der 6 MOSFET-Gates (nur PWM-faehige Pins des ESP32 verwenden!)
//  Reihenfolge = Kanal-Index: Kanal 0 -> LICHT_PIN_K0, usw.
constexpr int LICHT_PIN_K0 = 18;
constexpr int LICHT_PIN_K1 = 19;
constexpr int LICHT_PIN_K2 = 21;
constexpr int LICHT_PIN_K3 = 22;
constexpr int LICHT_PIN_K4 = 23;
constexpr int LICHT_PIN_K5 = 5;

// =============================================================================
//  BESCHATTUNG / DACHFENSTER  (Servos am PCA9685, Kanaele 0..7, 50 Hz)
// =============================================================================
constexpr uint8_t SERVO_DACHFENSTER_OG2 = 0;   // relevant fuer Automatik-Regel 1
constexpr uint8_t SERVO_DACHFENSTER_OG1 = 1;
constexpr uint8_t SERVO_JALOUSIE_EG     = 2;
constexpr uint8_t SERVO_JALOUSIE_OG1    = 3;
constexpr uint8_t SERVO_GARAGENTOR      = 4;

//  PWM-Ticks (0..4095) fuer die Endlagen - pro Servo einmal einmessen!
constexpr uint16_t SERVO_TICK_ZU  = 150;   // ~ 0°
constexpr uint16_t SERVO_TICK_AUF = 500;   // ~ 90°

constexpr float   BESCHATTUNG_HELL_LUX = 800.0f;  // ab hier Jalousie schliessen

// =============================================================================
//  SICHERHEIT  (Reed-Kontakte, PIR, Alarmrelais, Buzzer)
// =============================================================================
constexpr IoPin REED_TUER_EG     = mcpPin(5);
constexpr IoPin REED_FENSTER_OG  = mcpPin(6);
constexpr IoPin PIR_SCHLAFZIMMER = mcpPin(4);
constexpr IoPin ALARM_RELAIS     = mcpPin(11);
constexpr IoPin ALARM_BUZZER     = mcpPin(12);

// =============================================================================
//  DISCOLIGHT  (Stimmungslicht, eigener NeoPixel-Strang)
// =============================================================================
constexpr int      DISCO_STRIP_PIN   = 18;
constexpr uint16_t DISCO_ANZAHL_LEDS = 8;

// =============================================================================
//  ZEITTAKTE / ABTASTRATEN  (alles nicht-blockierend, millis-basiert)
// =============================================================================
constexpr uint32_t TAKT_TASTER_ENTPRELL_MS = 50;    // Tasterentprellung
constexpr uint32_t TAKT_SENSOR_MS          = 1000;  // Sensoren einlesen
constexpr uint32_t TAKT_STATUS_SENDEN_MS   = 1000;  // Status an den Pi senden

// ─── FREIE PINS / RESERVE ────────────────────────────────────────────────────
//  MCP frei : 7, 13, 14, 15
//  Bei neuer Hardware hier eine Zeile ergaenzen, z.B.:
//      constexpr IoPin MEIN_NEUER_TASTER = mcpPin(7);
