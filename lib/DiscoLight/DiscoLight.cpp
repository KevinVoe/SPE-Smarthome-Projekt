// =============================================================================
//  DiscoLight.cpp  –  Implementierung des Party-Stimmungslichts
// =============================================================================
//
//  ANIMATIONSKONZEPT
//  ─────────────────
//  Das Licht arbeitet komplett ohne delay(), alle Zeitangaben per millis().
//  Pro Frame (ca. 50/s) werden drei unabhaengige Layer berechnet:
//
//  [1] HELLIGKEIT (Beat-Pulsation)
//      Ein simulierter 4/4-Takt (ca. 120 BPM, leicht random) erzeugt
//      kurze Ausschlaege auf DISCO_HELL_MAX, dazwischen Abdimmen auf
//      DISCO_HELL_MIN. FastLED.setBrightness() skaliert den gesamten Strip.
//
//  [2] FARBEN (Modus-basiert, wechselt alle paar Sekunden)
//      Modus 0 – SYNCHRON:  alle LEDs dieselbe Farbe, langsamer Hue-Sweep
//      Modus 1 – SEGMENTE:  Strip in 2-4 Segmente aufgeteilt, je eigene Farbe
//      Modus 2 – CHAOTISCH: jede LED eigene Hue, gelegentlich Sprung auf neue
//
//  [3] STROBE / BLINK
//      Alle paar Sekunden blinken zufaellig ausgewaehlte LEDs kurzzeitig
//      schnell (3–8 Hz) fuer 0.5–2 s. Strobe ueberlagert Layer [2].
//
// =============================================================================
#include "DiscoLight.h"
#include <Arduino.h>
 
// ── Hilfsmakro: sicheres random im Bereich [lo, hi] ──────────────────────────
static inline uint32_t rnd(uint32_t lo, uint32_t hi) {
  return lo + (uint32_t)random(hi - lo + 1);
}
 
// =============================================================================
//  Konstruktor / begin
// =============================================================================
DiscoLight::DiscoLight(int stripPin, uint16_t anzahlLeds)
  : _pin(stripPin),
    _anzahl(anzahlLeds > DISCO_MAX_LEDS ? DISCO_MAX_LEDS : anzahlLeds)
{}
 
void DiscoLight::begin() {
  // FastLED verlangt zur Compile-Zeit den Pin als Template-Parameter.
  // Da dieser Wert aus Config.h kommt und wir eine dynamische Klasse wollen,
  // nutzen wir addLeds mit dem gespeicherten Pin via Compile-Zeit-Konstante.
  // Hinweis: Falls DISCO_STRIP_PIN != 18 ist, den Wert hier anpassen ODER
  //          DISCO_STRIP_PIN direkt als Template-Argument verwenden:
  //          FastLED.addLeds<WS2813, DISCO_STRIP_PIN, GRB>(_leds, _anzahl)
  FastLED.addLeds<WS2813, 0, GRB>(_leds, _anzahl);
  FastLED.setBrightness(DISCO_HELL_MAX);
  FastLED.clear(true);
 
  // Initialisierung der LED-Hue-Tabelle
  for (uint16_t i = 0; i < _anzahl; i++) {
    _ledHue[i] = (uint8_t)(i * 255 / _anzahl);
    _ledSat[i] = 240;
  }
}
 
// =============================================================================
//  an / aus
// =============================================================================
void DiscoLight::an() {
  _an          = true;
  _frame       = 0;
  _beatPhase   = 0;
  _beatLen     = rnd(35, 45);        // ~120 BPM bei 50 fps → ~25 Frames/Beat
  _helligkeit  = DISCO_HELL_MAX;
  _farbModus   = 0;
  _hue         = random(256);
  _hueZielwert = _hue;
  _naechsterModus  = rnd(200, 400);  // erster Moduswechsel nach 4–8 s
  _naechsterStrobe = rnd(100, 250);  // erster Strobe nach 2–5 s
  _strobeAktiv     = false;
  _letzt       = millis();
}
 
void DiscoLight::aus() {
  _an = false;
  FastLED.clear(true);
}
 
// =============================================================================
//  update()  –  Herzstuck, aufrufen in loop()
// =============================================================================
void DiscoLight::update() {
  if (!_an) return;
 
  uint32_t jetzt = millis();
  if (jetzt - _letzt < DISCO_UPDATE_INTERVALL_MS) return;
  _letzt = jetzt;
  _frame++;
 
  _berechneHelligkeit();
  _berechneFarben();
  _berechneStrobe();
  _schreibeStrip();
}
 
// =============================================================================
//  [1] HELLIGKEIT  –  Beat-Pulsation
// =============================================================================
void DiscoLight::_berechneHelligkeit() {
  _beatPhase++;
 
  if (_beatPhase >= _beatLen) {
    _beatPhase = 0;
    // Beat-Laenge leicht variieren → "lebendiger" Rhythmus
    _beatLen = rnd(22, 30);          // 22–30 Frames (~60-90 fps → 120 BPM +/-)
  }
 
  // Anstiegsphase: erste 20 % des Beats -> schnell auf Max
  // Abfallphase:   restliche 80 %       -> langsames Abdimmen auf Min
  float t = (float)_beatPhase / (float)_beatLen;
 
  if (t < 0.20f) {
    // Schneller Anstieg (cosinus-foermig)
    float u  = t / 0.20f;
    float cos_u = 0.5f * (1.0f - cosf(u * 3.14159f));
    _helligkeit  = (uint8_t)(DISCO_HELL_MIN + cos_u * (DISCO_HELL_MAX - DISCO_HELL_MIN));
  } else {
    // Langsames Abdimmen (exponentiell)
    float u = (t - 0.20f) / 0.80f;
    float abfall = 1.0f - (u * u);   // quadratisch
    _helligkeit  = (uint8_t)(DISCO_HELL_MIN + abfall * (DISCO_HELL_MAX - DISCO_HELL_MIN));
  }
 
  FastLED.setBrightness(_helligkeit);
}
 
// =============================================================================
//  [2] FARBEN  –  Modus-basiert
// =============================================================================
void DiscoLight::_berechneFarben() {
  // ── Moduswechsel pruefen ──────────────────────────────────────────────────
  if (_frame >= _naechsterModus) {
    _starteNeuenModus();
    _naechsterModus = _frame + rnd(150, 400);   // naechster Wechsel 3–8 s
  }
 
  // ── Langsamen Hue-Sweep berechnen ─────────────────────────────────────────
  //    In Synchron- und Segmentmodus dreht die Basisfarbe alle ~10 s einmal rum
  static uint32_t hueTick = 0;
  if (++hueTick % 3 == 0) {                     // alle 3 Frames (~60 ms)
    _hue++;                                     // 256 Schritte * 60 ms ≈ 15 s
  }
 
  // ── Farben je Modus setzen ────────────────────────────────────────────────
  switch (_farbModus) {
 
    // ── Modus 0: SYNCHRON ─────────────────────────────────────────────────
    case 0:
      for (uint16_t i = 0; i < _anzahl; i++) {
        _leds[i] = CHSV(_hue, _saturation, 255);
      }
      break;
 
    // ── Modus 1: SEGMENTE ─────────────────────────────────────────────────
    case 1: {
      uint8_t segAnzahl = (uint8_t)rnd(2, 4);
      // Segmentanzahl nur beim Moduswechsel zufaellig – hier statisch aus
      // _naechsterModus-Zeitpunkt modulo 3 ableiten (Compiler-Konstante nicht noetig)
      segAnzahl = 2 + (_frame % 3 == 0 ? 1 : 0);   // 2 oder 3 Segmente
      uint16_t segLen = _anzahl / segAnzahl;
      for (uint16_t i = 0; i < _anzahl; i++) {
        uint8_t seg = (segLen > 0) ? (uint8_t)(i / segLen) : 0;
        uint8_t segHue = _hue + seg * (256 / segAnzahl);
        _leds[i] = CHSV(segHue, _saturation, 255);
      }
      break;
    }
 
    // ── Modus 2: CHAOTISCH ────────────────────────────────────────────────
    case 2:
      // Jede LED wandert auf eigener Hue-Bahn, gelegentlich Sprung
      for (uint16_t i = 0; i < _anzahl; i++) {
        if (_frame % rnd(8, 20) == (i % 12)) {
          // gelegentlicher Farbsprung dieser LED
          _ledHue[i] = (uint8_t)random(256);
          _ledSat[i] = (uint8_t)rnd(180, 255);
        } else {
          _ledHue[i]++;                         // langsamer Drift
        }
        _leds[i] = CHSV(_ledHue[i], _ledSat[i], 255);
      }
      break;
 
    default:
      break;
  }
}
 
// =============================================================================
//  [3] STROBE / BLINK
// =============================================================================
void DiscoLight::_berechneStrobe() {
  // ── Neuen Strobe starten, wenn Zeit ───────────────────────────────────────
  if (!_strobeAktiv && _frame >= _naechsterStrobe) {
    _starteNeuenStrobe();
  }
 
  if (!_strobeAktiv) return;
 
  // ── Strobe-Dauer pruefen ──────────────────────────────────────────────────
  uint32_t strobeFrame = _frame - _strobeStart;
  if (strobeFrame >= _strobeDauer) {
    _strobeAktiv      = false;
    _naechsterStrobe  = _frame + rnd(100, 300); // Pause 2–6 s
    return;
  }
 
  // ── Blinken: Halbperiode on/off ───────────────────────────────────────────
  bool strobeAn = ((strobeFrame / _strobeInterval) % 2 == 0);
 
  if (!strobeAn) {
    // Strobe-LEDs kurz dunkel schalten (nur die markierten)
    for (uint16_t i = 0; i < _anzahl && i < 16; i++) {
      if (_strobeMaske & (1 << i)) {
        _leds[i] = CRGB::Black;
      }
    }
  }
  // Im "An"-Zustand bleiben die Farben aus _berechneFarben() erhalten
}
 
// =============================================================================
//  _schreibeStrip  –  Daten ausgeben
// =============================================================================
void DiscoLight::_schreibeStrip() {
  FastLED.show();
}
 
// =============================================================================
//  Hilfsmethoden
// =============================================================================
 
void DiscoLight::_starteNeuenModus() {
  // Wechsel nie in denselben Modus (ausser Zufall trifft ihn)
  uint8_t neuerModus;
  do {
    neuerModus = (uint8_t)random(3);          // 0, 1 oder 2
  } while (neuerModus == _farbModus && random(4) != 0);  // 25% Wiederholung erlaubt
 
  _farbModus = neuerModus;
 
  // Bei Moduswechsel neue Saturation wuerfeln (manchmal weisser, manchmal satter)
  _saturation = (uint8_t)rnd(180, 255);
 
  // Im chaotischen Modus alle LED-Hues neu initialisieren
  if (_farbModus == 2) {
    for (uint16_t i = 0; i < _anzahl; i++) {
      _ledHue[i] = (uint8_t)random(256);
      _ledSat[i] = (uint8_t)rnd(180, 255);
    }
  }
}
 
void DiscoLight::_starteNeuenStrobe() {
  _strobeAktiv    = true;
  _strobeStart    = _frame;
  _strobeDauer    = rnd(25, 100);              // 0.5–2 s bei 50 fps
  _strobeInterval = rnd(3, 8);                // Blink-Halbperiode 3–8 Frames (6–16 Hz)
 
  // Zufaellige Auswahl von 1–4 LEDs fuer den Strobe (Bitmaske)
  _strobeMaske = 0;
  uint8_t strobeAnzahl = (uint8_t)rnd(1, min((uint16_t)4, _anzahl));
  for (uint8_t j = 0; j < strobeAnzahl; j++) {
    uint8_t led = (uint8_t)random(_anzahl < 16 ? _anzahl : 16);
    _strobeMaske |= (1 << led);
  }
}
 
uint8_t DiscoLight::_interpoliereHue(uint8_t von, uint8_t nach, uint8_t t) {
  // Kreisinterpolation auf dem Farbkreis (0–255)
  int16_t delta = (int16_t)nach - (int16_t)von;
  if (delta > 128)  delta -= 256;
  if (delta < -128) delta += 256;
  return (uint8_t)(von + (int16_t)((int32_t)delta * t / 255));
}
