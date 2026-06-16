#include "Licht.h"

Licht::Licht(int stripPin, uint16_t anzahlLeds)
  : _strip(anzahlLeds, stripPin, NEO_GRB + NEO_KHZ800) {}

void Licht::begin() {
  _strip.begin();
  _strip.show();                       // alle LEDs aus
}

void Licht::an() {
  if (_an) return;                     // nur bei Aenderung neu schreiben
  for (uint16_t i = 0; i < _strip.numPixels(); i++)
    _strip.setPixelColor(i, _strip.Color(255, 255, 200));  // warmweiss
  _strip.show();
  _an = true;
}

void Licht::aus() {
  if (!_an) return;
  _strip.clear();
  _strip.show();
  _an = false;
}

void Licht::update() {
  // Platz fuer sanftes Dimmen / Lichteffekte (nicht-blockierend per millis()).
}
