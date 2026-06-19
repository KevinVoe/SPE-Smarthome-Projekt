// =============================================================================
//  ledDriverRandom_debug.cpp  –  Minimaltest zur Eingrenzung des Boot-Loops
// -----------------------------------------------------------------------------
//  STUFE 1: Nur Serial.begin() + println(). Wenn das schon resettet,
//  liegt es NICHT an Licht/analogWrite, sondern an etwas Grundlegenderem
//  (Stromversorgung/Power-On, Brownout-Detector, defektes Board, etc.)
//
//  Wenn STUFE 1 stabil laeuft: Schritt fuer Schritt die auskommentierten
//  Bloecke darunter einzeln aktivieren (entkommentieren), neu flashen,
//  testen. Sobald der Reset wieder auftritt, ist die zuletzt aktivierte
//  Zeile/der zuletzt aktivierte Block der Schuldige.
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
    delay(2000);                       // Zeit fuer Serial Monitor zum Verbinden
    Serial.println("STUFE 1: Nur Serial laeuft. Wenn das stabil ist -> weiter zu Stufe 2.");

    //── STUFE 2: licht.begin() einzeln testen ──────────────────────────────
    Serial.println("STUFE 2: rufe licht.begin() auf...");
    licht.begin();
    Serial.println("STUFE 2: licht.begin() abgeschlossen, kein Crash.");

    // ── STUFE 3: einen einzelnen Kanal setzen ──────────────────────────────
    // Serial.println("STUFE 3: setze Kanal 0 auf Stufe 1...");
    // licht.setKanal(0, 1);
    // Serial.println("STUFE 3: Kanal 0 gesetzt, kein Crash.");

    // ── STUFE 4: alle 6 Kanaele einzeln, mit Serial-Ausgabe pro Kanal ──────
    // for (uint8_t kanal = 0; kanal < LICHT_MAX_KANAELE; kanal++) {
    //     Serial.print("STUFE 4: setze Kanal "); Serial.println(kanal);
    //     licht.setKanal(kanal, 1);
    //     delay(200);
    // }
    // Serial.println("STUFE 4: alle Kanaele gesetzt, kein Crash.");
}

void loop() {
    Serial.println("loop laeuft...");
    delay(1000);
}
