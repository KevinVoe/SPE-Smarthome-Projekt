// =============================================================================
//  main.cpp  –  Verdrahtung & Ablauf der Haussteuerung
// -----------------------------------------------------------------------------
//  Ablauf je loop():  Eingaenge + Sensoren -> Kontext -> Regel-Schichten ->
//                     Interlocks -> anwenden() -> Aufzug -> Telemetrie.
//  Die Regel-Schichten stehen in Regelung; hier wird nur INITIALISIERT,
//  EINGELESEN und der Soll-Zustand auf die Module UEBERTRAGEN.
//  Der Aufzug ist ein EIGENES Zustands-Subsystem (laeuft NEBEN der Soll-Pipeline).
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "Regelung.h"
#include "Licht.h"
#include "DiscoLight.h"
#include "Servoaktor.h"
#include "DigitaleOutputs.h"
#include "DigitalInput.h"
#include "Sensorik.h"
#include "Aufzug.h"
#include "Kommunikation.h"

// ─── Module / globale Objekte ────────────────────────────────────────────────
const int lichtPins[LICHT_MAX_KANAELE] = {
    LICHT_PIN_K0, LICHT_PIN_K1, LICHT_PIN_K2,
    LICHT_PIN_K3, LICHT_PIN_K4, LICHT_PIN_K5
};
Licht          licht(lichtPins);
DiscoLight     disco(DISCO_STRIP_PIN, DISCO_ANZAHL_LEDS);
Sensorik       sensorik;                // DHT (Temp/Feuchte) + Solar
Aufzug         aufzug(AUFZUG_IN1, AUFZUG_IN2, AUFZUG_IN3, AUFZUG_IN4,
                      AUFZUG_STEP_INTERVALL_US, AUFZUG_TIMEOUT_MS);
HardwareSerial Link(2);                 // UART2 zum Pi
Kommunikation  kommunikation(Link);

// Persistenter Zustand ZWISCHEN den Loops (Soll wird jede Loop neu gebaut):
DashboardState gDash;         // Pi-Overrides (mit TTL)
TasterState    gTaster;       // Etagen-Klima-Modus per Taster (mit TTL)
Soll           gAutoSnapshot; // eingefrorener Automatik-Anteil (Tageszeit+Sensor)
uint32_t       gLoops = 0;    // Loop-Zaehler (Schleifenrate, s. debugAusgabe)

void anwenden(const Soll& s);
void telemetrie(const Kontext& k, const Soll& s);
void debugAusgabe(const Kontext& k, const Soll& s);
void eingaengeLesen(Kontext& k);
void sensorenLesen(Kontext& k);
void aufzugBedienen();

// =============================================================================
//  SETUP
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[main] Start.");

  // I2C-Bus EINMAL starten, DANN die Module, die darauf bauen.
  Wire.begin(PIN_SDA, PIN_SCL);
  digitaleOutputsBegin();   // besitzt + initialisiert den Ausgangs-MCP (alle aus)
  digitalInputBegin();      // besitzt + initialisiert den Eingangs-MCP (Pull-ups)
  servosBegin();            // PCA9685 (Servos auf Startlage)

  Serial.printf("[main] MCP-IN=%s  MCP-OUT=%s\n",
                digitalInputOk()   ? "OK" : "FEHLT",
                digitaleOutputsOk() ? "OK" : "FEHLT");

  // ESP-native Aktoren / Sensoren
  licht.begin();
  disco.begin();
  sensorik.begin();         // DHT + Solar
  aufzug.begin();           // Schrittmotor-Pins (IN1-4)

  // Link zum Raspberry Pi (UART2: RX=16, TX=17)
  Link.begin(115200, SERIAL_8N1, 16, 17);
  kommunikation.begin();
  kommunikation.onBefehl([](JsonDocument& d){ behandleBefehl(d, gDash); });

  Serial.println("[main] Setup fertig.");
}

// =============================================================================
//  LOOP
// =============================================================================
void loop() {
  gLoops++;                  // jede Iteration zaehlen (Schleifenrate, s. debugAusgabe)

  // 1) Eingaenge + Sensoren -> Kontext
  Kontext k;
  k.stunde      = berechneTageszeit(millis(), TAG_LAENGE_MS);
  k.phase       = phaseAus(k.stunde);
  k.sonnenSeite = sonnenSeiteAus(k.stunde);
  eingaengeLesen(k);         // Taster/Reeds entprellt + Etagen-Klima-Modus
  sensorenLesen(k);          // Temperatur, Feuchte, Solar-Spannung

  kommunikation.update_empfang();

  // 2) Soll bilden (Regel-Schichten).
  //    Automatik-Stopp vom Dashboard: Tageszeit + Sensor werden EINGEFROREN
  //    (letzter Stand wird gehalten); Taster + Dashboard bleiben aktiv.
  k.automatikAus = automatikEingefroren(gDash);

  Soll soll;
  if (!k.automatikAus) {
    tageszeitRegeln(k, soll);
    sensorRegeln(k, soll);
    gAutoSnapshot = soll;        // Automatik-Anteil merken (fuer ein evtl. Einfrieren)
  } else {
    soll = gAutoSnapshot;        // eingefrorenen Automatik-Anteil wiederverwenden
  }
  handRegeln(k, soll);           // Taster (Hand) - bleibt auch im Freeze aktiv
  dashboardRegeln(soll, gDash);  // Pi-Overrides - bleiben aktiv

  // 3) Konflikte prioritaetsbewusst aufloesen
  interlocks(soll);

  // 4) Anwenden  +  Aufzug (eigenes Subsystem)  +  Telemetrie
  anwenden(soll);
  aufzugBedienen();
  k.aufzugEtage = (uint8_t)aufzug.aktuelleEtage();   // fuer die Telemetrie
  telemetrie(k, soll);

  // Debug-Ausgabe (1x pro Sekunde)
  static uint32_t t = 0;
  if (millis() - t >= 1000) { t = millis(); debugAusgabe(k, soll); }
}

// =============================================================================
//  EINGAENGE  –  Taster/Reeds lesen, Etagen-Klima-Modus (mit TTL) fortschalten
// =============================================================================
void eingaengeLesen(Kontext& k) {
  digitalInputUpdate();
  if (geradeGedrueckt(Eingang::TASTER_EG))  tasterWeiterschalten(gTaster, 0);
  if (geradeGedrueckt(Eingang::TASTER_OG1)) tasterWeiterschalten(gTaster, 1);
  if (geradeGedrueckt(Eingang::TASTER_OG2)) tasterWeiterschalten(gTaster, 2);
  tasterTick(gTaster);   // abgelaufene Hand-Modi -> Automatik
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) k.klimaModus[e] = tasterModus(gTaster, e);
}

// =============================================================================
//  SENSOREN  –  Messwerte in den Kontext schreiben
// =============================================================================
void sensorenLesen(Kontext& k) {
  sensorik.update();                       // DHT (alle 2 s) + Solar + Bodenfeuchte
  k.temperatur   = sensorik.temperatur();  // EIN Sensor fuers ganze Haus
  k.feuchte      = sensorik.feuchte();
  k.pvSpannung   = sensorik.pvSpannung();
  k.bodenfeuchte = sensorik.bodenfeuchte();// Gewaechshaus
  k.helligkeit   = sensorik.pvSpannung();  // Solarpanel dient zugleich als Helligkeit (grob)
}

// =============================================================================
//  AUFZUG  –  Ruftaster -> Fahrt; Reeds + oberer Schalter -> update()
//  (eigenstaendiges Subsystem, nicht ueber den Soll-Zustand)
// =============================================================================
void aufzugBedienen() {
  // Ruftaster (steigende Flanke). Ein Druck waehrend der Fahrt wechselt das Ziel
  // (und ggf. die Richtung) - das macht die Aufzug-Klasse selbst.
  if (geradeGedrueckt(Eingang::AUFZUG_TASTER_EG))  aufzug.fahreZu(Aufzug::Etage::EG);
  if (geradeGedrueckt(Eingang::AUFZUG_TASTER_OG1)) aufzug.fahreZu(Aufzug::Etage::OG1);
  if (geradeGedrueckt(Eingang::AUFZUG_TASTER_OG2)) aufzug.fahreZu(Aufzug::Etage::OG2);

  // Etagen-Reeds + oberer Ueberfahr-Schalter -> Motor takten / stoppen (JEDE Loop!)
  aufzug.update(gedrueckt(Eingang::REED_AUFZUG_EG),
                gedrueckt(Eingang::REED_AUFZUG_OG1),
                gedrueckt(Eingang::REED_AUFZUG_OG2),
                gedrueckt(Eingang::REED_AUFZUG_OBEN));
  // TODO: FEHLER quittieren (z.B. per Dashboard-Befehl oder Reset-Taster):
  //       aufzug.fehlerQuittieren();  + danach Referenzfahrt nach EG.
}

// =============================================================================
//  ANWENDEN  –  Soll-Zustand auf die Module uebertragen (EINZIGE Hardware-Stelle)
// =============================================================================
void anwenden(const Soll& s) {
  // ── Licht (6 PWM-Kanaele; nur bei Aenderung schreiben) ─────────────────────
  static uint8_t letzteLichtStufe[LICHT_MAX_KANAELE] = {};
  for (uint8_t kanal = 0; kanal < LICHT_MAX_KANAELE; kanal++) {
    uint8_t neueStufe = (uint8_t)s.licht[kanal].wert;
    if (neueStufe != letzteLichtStufe[kanal]) {
      licht.setKanal(kanal, neueStufe);
      letzteLichtStufe[kanal] = neueStufe;
    }
  }

  // ── Heizung / Kuehlung (MCP-OUT; nur bei Aenderung -> wenig I2C-Verkehr) ────
  static bool letztHeiz[ANZ_ETAGEN]  = {};
  static bool letztKuehl[ANZ_ETAGEN] = {};
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    bool h  = s.heizung[e].wert  != 0;
    bool kk = s.kuehlLed[e].wert != 0;
    if (h  != letztHeiz[e])  { heizen((Etage)e,  h);  letztHeiz[e]  = h;  }
    if (kk != letztKuehl[e]) { kuehlen((Etage)e, kk); letztKuehl[e] = kk; }
  }

  // ── Servos: Jalousien + Dachfenster (Ziel setzen; Fahrt macht servosUpdate) ─
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++)
    for (uint8_t seite = 0; seite < ANZ_SEITEN; seite++)
      fahreJalousie(s.jalousie[e][seite].wert >= 50 ? Position::ZU : Position::AUF,
                    (Etage)e, (Seite)seite);
  Position dachPos = s.dachfensterOG2.wert >= 50 ? Position::AUF : Position::ZU;
  fahreDachfenster(dachPos, Seite::LINKS);
  fahreDachfenster(dachPos, Seite::RECHTS);
  // Hinweis: s.klimaanlage hat (noch) keinen physischen Ausgang - nur Logik/Telemetrie.

  // ── Disco (NeoPixel) ───────────────────────────────────────────────────────
  static uint8_t letzterDisco = 0;
  uint8_t discoNeu = (uint8_t)s.disco.wert;
  if (discoNeu == 1 && letzterDisco == 0)      disco.an();
  else if (discoNeu == 0 && letzterDisco == 1) disco.aus();
  letzterDisco = discoNeu;
  disco.update();

  // ── Servos einen Schritt weiterbewegen (sanft, nicht-blockierend) ──────────
  servosUpdate();
}

// =============================================================================
//  TELEMETRIE an den Pi  (max. 1x / 100 ms; sendet intern nur bei Aenderung)
// =============================================================================
#define TELEMETRIE_USB_SPIEGEL 1   // auf 0 setzen, um die USB-Ausgabe abzuschalten

void telemetrie(const Kontext& k, const Soll& s) {
  static uint32_t letzteMs = 0;
  if (millis() - letzteMs < 100) return;   // Drossel: hoechstens alle 100 ms
  letzteMs = millis();

  bool gesendet = kommunikation.update(s, k);

#if TELEMETRIE_USB_SPIEGEL
  if (gesendet) {
    Serial.print("[TX->Pi] ");
    Serial.println(kommunikation.letzteTelemetrie());
  }
#else
  (void)gesendet;
#endif
}

// Kompakte Statuszeile zum Mitlesen.
void debugAusgabe(const Kontext& k, const Soll& s) {
  Serial.printf("t=%5.2fh  Heiz[%d%d%d] Kuehl[%d%d%d] "
                "AC=%d DachOG2=%d Disco=%d  Licht[%d%d%d|%d%d%d]\n",
    k.stunde,
    s.heizung[0].wert,  s.heizung[1].wert,  s.heizung[2].wert,
    s.kuehlLed[0].wert, s.kuehlLed[1].wert, s.kuehlLed[2].wert,
    s.klimaanlage.wert, s.dachfensterOG2.wert > 0, s.disco.wert,
    s.licht[0].wert, s.licht[1].wert, s.licht[2].wert,
    s.licht[3].wert, s.licht[4].wert, s.licht[5].wert);

  static uint32_t letzteLoops = 0;
  uint32_t proSekunde = gLoops - letzteLoops;
  letzteLoops = gLoops;
  Serial.printf("          T=%.1fC rH=%.0f%% PV=%.1fV | Aufzug z=%d etage=%d | Loops %u (~%u/s)\n",
                k.temperatur, k.feuchte, k.pvSpannung,
                (int)aufzug.zustand(), (int)aufzug.aktuelleEtage(),
                (unsigned)gLoops, (unsigned)proSekunde);
}
