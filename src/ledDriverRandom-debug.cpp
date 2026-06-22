// =============================================================================
//  ledDriver_ledc_test.cpp  –  PWM ueber explizite LEDC-API statt analogWrite
// -----------------------------------------------------------------------------
//  analogWrite() crasht auf diesem Setup (WDT-Reset). Daher nutzen wir die
//  LEDC-PWM-Peripherie direkt mit EXPLIZIT gesetzter Frequenz + Aufloesung.
//  Das ist der robuste, empfohlene Weg fuer PWM auf dem ESP32.
//
//  API-Stand: Arduino-ESP32 Core 2.0.x (ledcSetup / ledcAttachPin / ledcWrite).
// =============================================================================
#include <Arduino.h>
 
const int pins[6] = {2, 4, 5, 18, 19, 23};
 
constexpr uint32_t PWM_FREQ   = 5000;   // 5 kHz – flimmerfrei fuer LEDs
constexpr uint8_t  PWM_RES_BIT = 8;     // 8 Bit -> Duty 0..255 wie analogWrite
 
void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println();
    Serial.println(">>> LEDC-TEST laeuft <<<");
    Serial.flush();
 
    // Jeder Pin bekommt einen eigenen LEDC-Kanal (0..5).
    for (uint8_t i = 0; i < 6; i++) {
        Serial.print("Konfiguriere LEDC-Kanal "); Serial.print(i);
        Serial.print(" auf GPIO "); Serial.print(pins[i]); Serial.println("...");
        Serial.flush();
 
        ledcSetup(i, PWM_FREQ, PWM_RES_BIT);   // Kanal i: 5kHz, 8 Bit
        ledcAttachPin(pins[i], i);             // GPIO an Kanal i binden
        ledcWrite(i, 0);                       // aus
 
        Serial.println("  -> ok");
        Serial.flush();
        delay(200);
    }
    Serial.println("Alle 6 Kanaele konfiguriert, KEIN Crash.");
    Serial.flush();
}
 
void loop() {
    // Sanftes Durchfaden aller Kanaele als Sichttest
    static uint8_t duty = 0;
    static int8_t  dir  = 5;
 
    for (uint8_t i = 0; i < 6; i++) {
        ledcWrite(i, duty);
    }
    duty += dir;
    if (duty >= 250 || duty <= 5) dir = -dir;
 
    delay(30);
}