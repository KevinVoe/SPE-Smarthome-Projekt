#include "Licht.h"

Licht::Licht() : _strip(LICHT_ANZAHL_LEDS, LICHT_STRIP_PIN, NEO_GRB + NEO_KHZ800) {
}

void Licht::begin() {
  pinMode(LICHT_PIR_PIN, INPUT);

  _strip.begin();
  _strip.show(); // alle LEDs erstmal aus
}

void Licht::_ledsAn() {
  for (int i = 0; i < LICHT_ANZAHL_LEDS; i++) {
    _strip.setPixelColor(i, _strip.Color(255, 255, 255)); // weißes Licht
  }
  _strip.show();
}

void Licht::_ledsAus() {
  for (int i = 0; i < LICHT_ANZAHL_LEDS; i++) {
    _strip.setPixelColor(i, 0);
  }
  _strip.show();
}

void Licht::update() {
  if (digitalRead(LICHT_PIR_PIN) == HIGH) {
    _ledsAn();
  } else {
    _ledsAus();
  }
}