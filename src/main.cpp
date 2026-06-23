// =============================================================================
//  main.cpp  –  Grundgeruest der dreischichtigen Regelung
// -----------------------------------------------------------------------------
//  Zeigt den KOMPLETTEN Ablauf: Kontext bauen -> Schichten -> interlocks ->
//  anwenden. Eingaenge (Taster/Sensoren) und Ausgaenge (Module) sind noch
//  SIMULIERT bzw. als TODO markiert - das Verdrahten der dummen Module ist der
//  naechste Schritt. So laeuft die Logik schon jetzt auf dem nackten ESP32 und
//  gibt ihr Ergebnis per Serial aus (zum Mitlesen ohne Hardware).
// =============================================================================
#include <Arduino.h>
#include "Regelung.h"
#include "Config.h"
#include "Licht.h"
#include "DiscoLight.h"
#include "Kommunikation.h"

// Persistenter Zustand ZWISCHEN den Loops (der Soll wird jede Loop neu gebaut).
// Die Modus-Taster schalten diesen Wert spaeter per Flankenerkennung weiter.
const int lichtPins[LICHT_MAX_KANAELE] = {
    LICHT_PIN_K0, LICHT_PIN_K1, LICHT_PIN_K2,
    LICHT_PIN_K3, LICHT_PIN_K4, LICHT_PIN_K5
};
Licht licht(lichtPins);

DiscoLight disco(DISCO_STRIP_PIN, DISCO_ANZAHL_LEDS);

HardwareSerial Link(2);              // UART2 zum Pi
Kommunikation  kommunikation(Link);


KlimaModus klimaModus[ANZ_ETAGEN] = {
  KlimaModus::AUTOMATIK, KlimaModus::AUTOMATIK, KlimaModus::AUTOMATIK
};

void anwenden(const Kontext& k, const Soll& s);
void debugAusgabe(const Kontext& k, const Soll& s);

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[Regelung] Grundgeruest gestartet.");
  // TODO: io.begin(...), pwm.begin(), und alle Modul-begin() aufrufen,
  //       sobald die Module hier verdrahtet werden.
  licht.begin();
  disco.begin();
  Link.begin(115200, SERIAL_8N1, 16, 17);   // RX=16, TX=17
  kommunikation.begin();
  delay(300);
  Serial.println("[main.cpp] Setup fertig.");
}

void loop() {
  // ── 1) Kontext bauen ───────────────────────────────────────────────────────
  Kontext k;
  k.stunde      = berechneTageszeit(millis(), TAG_LAENGE_MS);
  k.phase       = phaseAus(k.stunde);
  k.sonnenSeite = sonnenSeiteAus(k.stunde);
  // TODO: echte Eingaenge einlesen:
  //   taster.update();  sensorik.update();
  //   for (e) k.temperatur[e] = sensorik.temperatur(e);
  //   k.helligkeit = sensorik.helligkeit();  k.bewegung = ...;
  //   Flankenerkennung der Klima-Taster -> klimaModus[e] weiterschalten
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) k.klimaModus[e] = klimaModus[e];

  // ── 2) Soll bilden: Schichten SICHTBAR nacheinander (Prio entscheidet) ─────
  Soll soll;                       // frisch -> alle Felder Default (0)
  tageszeitRegeln(k, soll);
  //sensorRegeln(k, soll);
  //handRegeln(k, soll);

  // ── 3) Konflikte aufloesen ─────────────────────────────────────────────────
  //interlocks(soll);

  // ── 4) Anwenden ────────────────────────────────────────────────────────────
  anwenden(k, soll);

  // ── Debug-Ausgabe (1x pro Sekunde) ─────────────────────────────────────────
  static uint32_t t = 0;
  if (millis() - t >= 1000) { t = millis(); debugAusgabe(k, soll); }
}

// -----------------------------------------------------------------------------
//  Soll-Zustand auf die (dummen) Module uebertragen - EINZIGE Hardware-Stelle.
//  TODO: echte Modul-Objekte ansteuern, z.B.:
//    heizungEG.setState(s.heizung[0].wert);
//    licht.setKanal(kanal, s.licht[kanal].wert);
//    jalousieLinksEG.setProzent(100 - s.jalousie[0][0].wert);  // Servo: 100=auf
//    dachfensterOG2Servo.setProzent(s.dachfensterOG2.wert);
//    if (s.disco.wert) disco.an(); else disco.aus();
//    io.digitalWrite(KLIMAANLAGE_PIN, s.klimaanlage.wert);
// -----------------------------------------------------------------------------
void anwenden(const Kontext& k, const Soll& s) {

  static uint8_t letzteLichtStufe[LICHT_MAX_KANAELE] = {};
  for (uint8_t e = 0; e < LICHT_MAX_KANAELE; e++) {
    uint8_t neueStufe = (uint8_t)s.licht[e].wert;
    if (neueStufe != letzteLichtStufe[e]) {
      licht.setKanal(e, neueStufe);
      letzteLichtStufe[e] = neueStufe;
    }
  }

  static uint8_t letzterDiscoStatus = 0;
  uint8_t neuerDiscoStatus = (uint8_t)s.disco.wert;
  if (neuerDiscoStatus == 1 && letzterDiscoStatus == 0) {
    disco.an();
  } else if (neuerDiscoStatus == 0 && letzterDiscoStatus == 1) {
    disco.aus();
  }
  letzterDiscoStatus = neuerDiscoStatus;
  disco.update();

  kommunikation.update(s, k);
}

// Kompakte Statuszeile zum Mitlesen, solange noch keine Hardware dranhaengt.
void debugAusgabe(const Kontext& k, const Soll& s) {
  Serial.printf("t=%5.2fh  Heiz[%d%d%d] Kuehl[%d%d%d] Klappe[%d%d%d] "
                "AC=%d DachOG2=%d Disco=%d  Licht[%d%d%d|%d%d%d]\n",
    k.stunde,
    s.heizung[0].wert,  s.heizung[1].wert,  s.heizung[2].wert,
    s.kuehlLed[0].wert, s.kuehlLed[1].wert, s.kuehlLed[2].wert,
    s.klappe[0].wert > 0, s.klappe[1].wert > 0, s.klappe[2].wert > 0,
    s.klimaanlage.wert, s.dachfensterOG2.wert > 0, s.disco.wert,
    s.licht[0].wert, s.licht[1].wert, s.licht[2].wert,
    s.licht[3].wert, s.licht[4].wert, s.licht[5].wert);
}
