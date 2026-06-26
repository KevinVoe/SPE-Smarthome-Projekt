// =============================================================================
//  Config.h  –  ZENTRALE Konfiguration des Smarthomes
// =============================================================================
#pragma once
#include "Io.h"

// =============================================================================
//  MODUS-TASTER
// =============================================================================
constexpr IoPin MODUS_TASTER_EG  = espPin(4);
constexpr IoPin MODUS_TASTER_OG1 = espPin(16);
constexpr IoPin MODUS_TASTER_OG2 = espPin(17);

// =============================================================================
//  HEIZUNG
// =============================================================================
constexpr IoPin HEIZUNG_EG_LED  = espPin(2);
constexpr IoPin HEIZUNG_OG1_LED = espPin(15);
constexpr IoPin HEIZUNG_OG2_LED = espPin(18);

// =============================================================================
//  LICHT  (PWM ueber MOSFET)
// =============================================================================
constexpr uint8_t LOW_BRIGHTNESS_DUTY_CYCLE    = 30;
constexpr uint8_t MEDIUM_BRIGHTNESS_DUTY_CYCLE = 120;
constexpr uint8_t LICHT_MAX_KANAELE            = 6;

constexpr int LICHT_PIN_K0 = 18;
constexpr int LICHT_PIN_K1 = 19;
constexpr int LICHT_PIN_K2 = 21;
constexpr int LICHT_PIN_K3 = 22;
constexpr int LICHT_PIN_K4 = 23;
constexpr int LICHT_PIN_K5 = 5;

// =============================================================================
//  JALOUSIEN  (je 2 Servos pro Etage am PCA9685)
// =============================================================================
constexpr uint8_t JALOUSIE_EG_LINKS_KANAL   = 0;
constexpr uint8_t JALOUSIE_EG_RECHTS_KANAL  = 1;
constexpr uint8_t JALOUSIE_OG1_LINKS_KANAL  = 2;
constexpr uint8_t JALOUSIE_OG1_RECHTS_KANAL = 3;
constexpr uint8_t JALOUSIE_DG_LINKS_KANAL   = 4;
constexpr uint8_t JALOUSIE_DG_RECHTS_KANAL  = 5;

constexpr uint16_t JALOUSIE_TICK_ZU  = 102;
constexpr uint16_t JALOUSIE_TICK_AUF = 512;

constexpr IoPin JALOUSIE_TASTER_EG  = mcpPin(13);
constexpr IoPin JALOUSIE_TASTER_OG1 = mcpPin(14);
constexpr IoPin JALOUSIE_TASTER_DG  = mcpPin(15);

// =============================================================================
//  DACHFENSTER / GARAGENTOR
// =============================================================================
constexpr uint8_t SERVO_DACHFENSTER_OG2 = 6;
constexpr uint8_t SERVO_DACHFENSTER_OG1 = 7;
constexpr uint8_t SERVO_GARAGENTOR      = 8;

constexpr uint16_t SERVO_TICK_ZU  = 150;
constexpr uint16_t SERVO_TICK_AUF = 500;

// =============================================================================
//  SENSORIK  –  DHT11 (Luft) + Water Sensor (Boden)
// -----------------------------------------------------------------------------
//  DHT11: GPIO 25, Signal / VCC 3,3V / GND
//         Pinbelegung Modul: links = Signal, mitte = VCC, rechts = GND
//
//  Water Sensor: S → GPIO 34, + → GPIO 26, - → GND
//         GPIO 26 schaltet VCC nur kurz beim Messen (Korrosionsschutz!)
//
//  Kalibrierung Water Sensor:
//         Sensor trocken in Luft → Serial Monitor → Wert bei TROCKEN eintragen
//         Sensor in Wasser       → Serial Monitor → Wert bei NASS eintragen
// =============================================================================
constexpr int     SENSORIK_DHT_PIN = 25;
constexpr uint8_t SENSORIK_DHT_TYP = DHT11;   // DHT11 oder DHT22

constexpr int SENSORIK_WASSER_PIN     = 34;   // Signal (analog)
constexpr int SENSORIK_WASSER_PWR     = 26;   // VCC (wird geschaltet)
constexpr int SENSORIK_WASSER_TROCKEN = 3000; // einmessen und anpassen!
constexpr int SENSORIK_WASSER_NASS    = 1000; // einmessen und anpassen!

// =============================================================================
//  SICHERHEIT
// =============================================================================
constexpr IoPin REED_TUER_EG     = mcpPin(5);
constexpr IoPin REED_FENSTER_OG  = mcpPin(6);
constexpr IoPin PIR_SCHLAFZIMMER = mcpPin(4);
constexpr IoPin ALARM_RELAIS     = mcpPin(11);
constexpr IoPin ALARM_BUZZER     = mcpPin(12);

// =============================================================================
//  DISCOLIGHT
// =============================================================================
constexpr int      DISCO_STRIP_PIN   = 18;
constexpr uint16_t DISCO_ANZAHL_LEDS = 8;

// =============================================================================
//  ZEITTAKTE
// =============================================================================
constexpr uint32_t TAKT_TASTER_ENTPRELL_MS = 50;
constexpr uint32_t TAKT_SENSOR_MS          = 1000;
constexpr uint32_t TAKT_STATUS_SENDEN_MS   = 1000;

// =============================================================================
//  AUFZUG
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

constexpr uint32_t AUFZUG_STEP_INTERVALL_US = 800;
constexpr uint32_t AUFZUG_TIMEOUT_MS        = 8000;