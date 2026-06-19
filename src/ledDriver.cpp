// =============================================================================
//  ledDriverRandom.cpp  –  Testprogramm: alle 6 Kanaele gleichzeitig,
//                           aber unabhaengig voneinander zufaellig ansteuern.
// -----------------------------------------------------------------------------
//  Jeder Kanal hat seinen eigenen Zufalls-Zeitpunkt fuer den naechsten
//  Stufenwechsel (0-3) und seine eigene zufaellige Wartezeit bis dahin.
//  Dadurch laufen alle 6 Kanaele komplett asynchron zueinander, statt
//  wie im Original nacheinander durchzurotieren.
//
//  Nutzt die Licht-Klasse – kein direktes analogWrite, kein alleAus()
//  (das wuerde alle anderen Kanaele gleichzeitig mit ausschalten!).
// =============================================================================
#include <Arduino.h>
#include "Config.h"
#include "Licht.h"
 
const int lichtPins[LICHT_MAX_KANAELE] = {
    LICHT_PIN_K0, LICHT_PIN_K1, LICHT_PIN_K2,
    LICHT_PIN_K3, LICHT_PIN_K4, LICHT_PIN_K5
};
Licht licht(lichtPins);
 
// ── Pro Kanal: Zeitpunkt des naechsten Wechsels + aktuelle Stufe ────────────
uint32_t naechsterWechsel[LICHT_MAX_KANAELE];
uint8_t  aktuelleStufe[LICHT_MAX_KANAELE];
 
// ── Mindest-/Maximaldauer, die eine Stufe gehalten wird (ms) ────────────────
constexpr uint32_t HALTEZEIT_MIN_MS = 300;
constexpr uint32_t HALTEZEIT_MAX_MS = 2000;
 
void setup() {
    Serial.begin(115200);
    licht.begin();
    Serial.println("LED-Treiber gestartet. Alle 6 Kanaele unabhaengig zufaellig...");
 
    uint32_t jetzt = millis();
    for (uint8_t kanal = 0; kanal < LICHT_MAX_KANAELE; kanal++) {
        // Stufen zufaellig initialisieren, Wechselzeitpunkte versetzt streuen,
        // damit nicht alle Kanaele zufaellig im selben Frame neu wuerfeln.
        aktuelleStufe[kanal]   = (uint8_t)random(0, 4);
        naechsterWechsel[kanal] = jetzt + random(HALTEZEIT_MIN_MS, HALTEZEIT_MAX_MS + 1);
        licht.setKanal(kanal, aktuelleStufe[kanal]);
        delay(10);   // kurze Pause: LEDC-Kanal-Initialisierung pro Pin entzerren
    }
}
 
void loop() {
    uint32_t jetzt = millis();
 
    for (uint8_t kanal = 0; kanal < LICHT_MAX_KANAELE; kanal++) {
        if (jetzt < naechsterWechsel[kanal]) continue;   // dieser Kanal ist noch nicht faellig
 
        uint8_t neueStufe = (uint8_t)random(0, 4);
        aktuelleStufe[kanal] = neueStufe;
        licht.setKanal(kanal, neueStufe);
 
        // naechsten Wechsel fuer GENAU DIESEN Kanal neu wuerfeln
        naechsterWechsel[kanal] = jetzt + random(HALTEZEIT_MIN_MS, HALTEZEIT_MAX_MS + 1);
 
        Serial.print("Kanal ");
        Serial.print(kanal);
        Serial.print(" -> Stufe ");
        Serial.println(neueStufe);
    }
}