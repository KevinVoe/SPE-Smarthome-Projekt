#include "DiscoLight.h"

DiscoLight::DiscoLight(int stripPin, uint16_t anzahlLeds)
  : _strip(anzahlLeds, stripPin, NEO_GRB + NEO_KHZ800) {}

void DiscoLight::begin() {
  _strip.begin();
  _strip.show();
}

void DiscoLight::aus() {
  _an = false;
  _strip.clear();
  _strip.show();
}

void DiscoLight::update() {
  if (!_an) return;
  if (millis() - _letzt < 30) return;        // ~33 Bilder/s, nicht-blockierend
  _letzt = millis();

  for (uint16_t i = 0; i < _strip.numPixels(); i++) {
    uint16_t hue = _phase + (uint32_t)i * 65536UL / _strip.numPixels();
    _strip.setPixelColor(i, _strip.gamma32(_strip.ColorHSV(hue)));
  }
  _strip.show();
  _phase += 1024;                            // Farbverlauf weiterdrehen
}
