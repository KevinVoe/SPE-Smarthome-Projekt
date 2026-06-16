// config.h
#pragma once

// ─── Direkte ESP32 GPIOs ───────────────────────────────
#define PIN_DS18B20     4   // 1-Wire Temperatursensoren
#define PIN_I2S_BCLK    26  // Audio
#define PIN_I2S_LRC     25
#define PIN_I2S_DOUT    27
#define LED_PIN 13

// ─── I2C Bus ───────────────────────────────────────────
#define PIN_SDA         21
#define PIN_SCL         22

// ─── I2C Adressen ──────────────────────────────────────
#define ADDR_MCP23017   0x20
#define ADDR_PCA9685    0x40
#define ADDR_BH1750     0x23
#define ADDR_SHT31      0x44

// ─── MCP23017 Pin-Belegung ─────────────────────────────
// Eingänge
#define MCP_REED_EG     0   // Türkontakt Erdgeschoss
#define MCP_REED_OG     1   // Türkontakt Obergeschoss
#define MCP_PIR_WOHNZIMMER  2
#define MCP_PIR_SCHLAFZIMMER 3

// Ausgänge
#define MCP_ALARM_RELAIS 8
#define MCP_BUZZER       9

// ─── PCA9685 Kanal-Belegung ────────────────────────────
// Servos (Kanäle 0–7, 50Hz)
#define PWM_SERVO_JALOUSIE_EG   0
#define PWM_SERVO_JALOUSIE_OG   1
#define PWM_SERVO_TUER_GARAGE   2

// LEDs (Kanäle 8–15, 50Hz)
#define PWM_LED_WOHNZIMMER  8
#define PWM_LED_SCHLAFZIMMER 9
#define PWM_LED_KUECHE      10
#define PWM_LED_FLUR        11

