// =============================================================================
//  DiscoLight  –  Party-Stimmungslicht  (WS2813, FastLED, nicht-blockierend)
// -----------------------------------------------------------------------------
//  Pinbelegung und LED-Anzahl kommen aus Config.h:
//    DISCO_STRIP_PIN    : Datenpin des WS2813-Strangs
//    DISCO_ANZAHL_LEDS  : Anzahl LEDs im Strang
//
//  Aufruf im Sketch:
//    DiscoLight disco(DISCO_STRIP_PIN, DISCO_ANZAHL_LEDS);
//    disco.begin();          // einmalig in setup()
//    disco.an();             // Party starten
//    disco.aus();            // Party stoppen  (Strip wird dunkel)
//    disco.update();         // MUSS in jedem loop()-Durchlauf aufgerufen werden!
//
//  Das Licht arbeitet komplett nicht-blockierend (millis-basiert).
// =============================================================================
#pragma once
#include <Arduino.h>
#include <FastLED.h>
 
// ── interne Konstanten ────────────────────────────────────────────────────────
//  Timing-Intervall fuer update() in Millisekunden (~50 fps)
static constexpr uint32_t DISCO_UPDATE_INTERVALL_MS = 20;
 
//  Helligkeits-Grenzen (0-255): Grunddimmen bis Beat-Ausschlag
static constexpr uint8_t  DISCO_HELL_MIN  = 64;   // ~25 %
static constexpr uint8_t  DISCO_HELL_MAX  = 255;  // 100 %
 
//  Anzahl der gleichzeitig verwalteten CRGB-Puffer-Slots
//  (wird intern als Template-Parameter benoetigt → muss Compile-Zeit-Konstante sein)
//  Entspricht DISCO_ANZAHL_LEDS aus Config.h; falls abweichend, hier anpassen.
//  Wert wird als MAX deklariert; tatsaechliche Anzahl kommt vom Konstruktor.
static constexpr uint16_t DISCO_MAX_LEDS = 64;
 
// =============================================================================
class DiscoLight {
public:
  // ---------------------------------------------------------------------------
  //  Konstruktor
  //  stripPin    : GPIO-Pin (z. B. DISCO_STRIP_PIN aus Config.h)
  //  anzahlLeds  : Anzahl LEDs  (z. B. DISCO_ANZAHL_LEDS aus Config.h)
  // ---------------------------------------------------------------------------
  DiscoLight(int stripPin, uint16_t anzahlLeds);
 
  // ---------------------------------------------------------------------------
  //  begin()  – einmalig in setup() aufrufen
  // ---------------------------------------------------------------------------
  void begin();
 
  // ---------------------------------------------------------------------------
  //  an()   – Disco-Modus starten
  //  aus()  – Disco-Modus stoppen, Strip dunkel schalten
  // ---------------------------------------------------------------------------
  void an();
  void aus();
 
  // ---------------------------------------------------------------------------
  //  update()  – MUSS in jedem loop()-Durchlauf aufgerufen werden!
  //  Liefert nur dann neue Frames, wenn DISCO_UPDATE_INTERVALL_MS vergangen sind.
  // ---------------------------------------------------------------------------
  void update();
 
  bool istAn() const { return _an; }
 
private:
  // ── Strip-Daten ────────────────────────────────────────────────────────────
  CRGB     _leds[DISCO_MAX_LEDS];
  int      _pin;
  uint16_t _anzahl;
 
  // ── Zustand ────────────────────────────────────────────────────────────────
  bool     _an      = false;
  uint32_t _letzt   = 0;    // Zeitstempel letztes Frame
  uint32_t _frame   = 0;    // Frame-Zaehler (fuer alle Zeitberechnungen)
 
  // ── Helligkeits-Pulsation ──────────────────────────────────────────────────
  uint8_t  _helligkeit = DISCO_HELL_MAX;
 
  //  "Beat"-Zustand: simulierter 4/4-Takt (ca. 120 BPM bei 50 fps)
  uint32_t _beatPhase  = 0;   // Phasenzaehler innerhalb eines Beats
  uint32_t _beatLen    = 0;   // Frames pro Beat (wird random variiert)
 
  // ── Farb-Modus ─────────────────────────────────────────────────────────────
  //  0 = synchron (alle gleich)   1 = Segmente   2 = chaotisch
  uint8_t  _farbModus      = 0;
  uint32_t _naechsterModus = 0;   // Frame-Zeitpunkt des naechsten Moduswechsels
 
  //  globale Hue fuer synchronen Modus / Basis fuer Segmentmodus
  uint8_t  _hue          = 0;
  uint8_t  _hueZielwert  = 0;
  uint8_t  _saturation   = 240;
 
  //  individuelle Hues fuer chaotischen Modus
  uint8_t  _ledHue[DISCO_MAX_LEDS];
  uint8_t  _ledSat[DISCO_MAX_LEDS];
 
  // ── Strobe / Blink-Effekt ──────────────────────────────────────────────────
  bool     _strobeAktiv    = false;
  uint32_t _strobeStart    = 0;   // Frame-Start
  uint32_t _strobeDauer    = 0;   // Frames Gesamtdauer
  uint32_t _strobeInterval = 0;   // Frames pro Blink-Halbperiode
  uint16_t _strobeMaske    = 0;   // Bitmaske: welche LEDs blinken (max 16)
  uint32_t _naechsterStrobe = 0;  // Frame-Zeitpunkt des naechsten Strobes
 
  // ── private Hilfsmethoden ──────────────────────────────────────────────────
  void   _berechneHelligkeit();
  void   _berechneFarben();
  void   _berechneStrobe();
  void   _schreibeStrip();
 
  void   _starteNeuenModus();
  void   _starteNeuenStrobe();
 
  uint8_t _interpoliereHue(uint8_t von, uint8_t nach, uint8_t t);
};