// =============================================================================
//  Config.h  –  ZENTRALE Konfiguration des Smarthomes
// -----------------------------------------------------------------------------
//  AKTUELLER STAND: Simulationsaufbau auf dem Breadboard.
//  Alle Pins liegen direkt am ESP32 (kein MCP23017 - kommt erst, wenn die
//  echte Verkabelung mit allen Modulen ansteht).
// =============================================================================
#pragma once
#include "Io.h"   // liefert IoPin, espPin(), mcpPin()

// =============================================================================
//  MODUS-TASTER  (je EINER pro Etage, schaltet zwischen 5 Modi durch)
// =============================================================================
constexpr IoPin MODUS_TASTER_EG  = espPin(4);
constexpr IoPin MODUS_TASTER_OG1 = espPin(16);
constexpr IoPin MODUS_TASTER_OG2 = espPin(17);

// =============================================================================
//  HEIZUNG  (3 Etagen, je eine LED; LED = simulierte Heizung)
// =============================================================================
constexpr IoPin HEIZUNG_EG_LED  = espPin(2);
constexpr IoPin HEIZUNG_OG1_LED = espPin(15);
constexpr IoPin HEIZUNG_OG2_LED = espPin(18);

// =============================================================================
//  LICHT  (12V LED-Strips ueber IRLZ44N-MOSFETs, PWM vom ESP32)
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
//  JALOUSIEN  (je 2 Servos pro Etage am PCA9685, 50 Hz)
// -----------------------------------------------------------------------------
//  Kanalzuweisung PCA9685 (0..15):
//    EG  : Kanal 0 (links) + Kanal 1 (rechts)
//    OG1 : Kanal 2 (links) + Kanal 3 (rechts)
//    DG  : Kanal 4 (links) + Kanal 5 (rechts)
//
//  PWM-Ticks EINMAL PRO SERVO einmessen!
//  Startwerte SG90: ZU = 102 (0 Grad), AUF = 512 (180 Grad)
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
//  DACHFENSTER / GARAGENTOR  (weitere Servos am PCA9685)
// =============================================================================
constexpr uint8_t SERVO_DACHFENSTER_OG2 = 6;
constexpr uint8_t SERVO_DACHFENSTER_OG1 = 7;
constexpr uint8_t SERVO_GARAGENTOR      = 8;

constexpr uint16_t SERVO_TICK_ZU  = 150;
constexpr uint16_t SERVO_TICK_AUF = 500;

// =============================================================================
//  SENSORIK  –  DHT11 Temperatur + Feuchte
// -----------------------------------------------------------------------------
//  Signal-Pin: GPIO 25 (getestet, stabil an 3,3V)
//  Pinbelegung Modul: Linker Pin = Signal, Mitte = VCC, Rechts = GND
//  DHT11:  Genauigkeit +-2 Grad C / +-5% Feuchte
//  DHT22:  Genauigkeit +-0.5 Grad C / +-2% Feuchte (genauer, selber Code)
// =============================================================================
constexpr int     SENSORIK_DHT_PIN = 25;
constexpr uint8_t SENSORIK_DHT_TYP = DHT11;   // DHT11 oder DHT22

// =============================================================================
//  SICHERHEIT  (Reed-Kontakte, PIR, Alarmrelais, Buzzer)
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