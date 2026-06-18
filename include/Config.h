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
//  Werden zentral vom Modul "Taster" eingelesen (Entprellung). Welcher Modus
//  daraus wird (Heizen/Kuehlen/Fenster.../Automatik), entscheidet main.cpp.
// =============================================================================
constexpr IoPin MODUS_TASTER_EG  = espPin(4);
constexpr IoPin MODUS_TASTER_OG1 = espPin(16);
constexpr IoPin MODUS_TASTER_OG2 = espPin(17);

// =============================================================================
//  HEIZUNG  (3 Etagen, je eine LED; LED = simulierte Heizung)
//  Reine Steuerung ueber Heizung::setState() - kein eigener Taster im Modul.
// =============================================================================
constexpr IoPin HEIZUNG_EG_LED  = espPin(2);
constexpr IoPin HEIZUNG_OG1_LED = espPin(15);
constexpr IoPin HEIZUNG_OG2_LED = espPin(18);

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
constexpr IoPin AUFZUG_TASTER_EG  = espPin(19);
constexpr IoPin AUFZUG_TASTER_OG1 = espPin(21);
constexpr IoPin AUFZUG_TASTER_OG2 = espPin(22);

//  AUFZUG  –  Ruftasten je Etage + Endschalter je Etage + Motortreiber
//  (A4988/DRV8825-Familie: STEP/DIR/ENABLE, Vollschritt, kein Microstepping)
constexpr IoPin AUFZUG_ENDSCHALTER_EG  = espPin(23);
constexpr IoPin AUFZUG_ENDSCHALTER_OG1 = espPin(25);
constexpr IoPin AUFZUG_ENDSCHALTER_OG2 = espPin(26);

constexpr IoPin AUFZUG_MOTOR_STEP   = espPin(27);
constexpr IoPin AUFZUG_MOTOR_DIR    = espPin(32);
constexpr IoPin AUFZUG_MOTOR_ENABLE = espPin(33);

constexpr uint32_t AUFZUG_STEP_INTERVALL_US = 800;    // Zeit zwischen 2 STEP-Pulsen (Tempo)
constexpr uint32_t AUFZUG_TIMEOUT_MS        = 8000;   // Sicherheits-Abschaltung

// ─── FREIE PINS / RESERVE ────────────────────────────────────────────────────
//  Fuer weitere Module (Kuehlung, Fenster, Jalousie) hier ergaenzen, z.B.:
//      constexpr IoPin KUEHLUNG_EG_LED = espPin(13);