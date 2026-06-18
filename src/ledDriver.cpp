// =============================================================================
//  ledDriver.cpp  –  Testprogramm fuer die PWM-MOSFET-Ansteuerung
// -----------------------------------------------------------------------------
//  Rotiert alle 6 Kanaele nacheinander durch alle 4 Dimmstufen (0-3).
//  Gibt den aktuellen Zustand per Serial aus.
//  Nutzt die Licht-Klasse – kein direktes analogWrite mehr.
// =============================================================================
#include <Arduino.h>
#include "Config.h"
#include "Licht.h"

const int lichtPins[LICHT_MAX_KANAELE] = {
    LICHT_PIN_K0, LICHT_PIN_K1, LICHT_PIN_K2,
    LICHT_PIN_K3, LICHT_PIN_K4, LICHT_PIN_K5
};
Licht licht(lichtPins);

void setup() {
    Serial.begin(115200);
    licht.begin();
    Serial.println("LED-Treiber gestartet. Teste alle 6 Kanaele...");
}

void loop() {
    for (uint8_t kanal = 0; kanal < LICHT_MAX_KANAELE; kanal++) {
        for (uint8_t stufe = 0; stufe <= 3; stufe++) {
            licht.alleAus();
            licht.setKanal(kanal, stufe);

            Serial.print("Kanal ");
            Serial.print(kanal);
            Serial.print("  Stufe ");
            Serial.print(stufe);
            Serial.print("  (");
            switch (stufe) {
                case 0: Serial.print("aus");    break;
                case 1: Serial.print("low");    break;
                case 2: Serial.print("medium"); break;
                case 3: Serial.print("voll");   break;
            }
            Serial.println(")");

            delay(500);
        }
    }
}