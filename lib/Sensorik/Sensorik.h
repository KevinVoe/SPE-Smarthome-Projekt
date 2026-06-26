// =============================================================================
//  Sensorik  –  Klimasensorik + Bodenfeuchte
// -----------------------------------------------------------------------------
//  Zwei Sensoren in einem Modul:
//
//  1. DHT11 / DHT22  (GPIO 25, digitales Protokoll, 1 Datenkabel)
//     → Liefert Lufttemperatur und Luftfeuchte
//     → Wird alle 2 Sekunden abgefragt
//
//  2. Water Sensor  (GPIO 34, analoges Signal)
//     → Misst Bodenfeuchte: trocken = hoher ADC-Wert, nass = niedriger Wert
//     → Wird auf 0..100% umgerechnet (0 = trocken, 100 = nass)
//     → SENSORIK_WASSER_TROCKEN / _NASS in Config.h einmessen!
// =============================================================================
#pragma once
#include <Arduino.h>
#include <DHT.h>

class Sensorik {
public:
  Sensorik();
  void begin();
  void update();

  float temperatur();     // Lufttemperatur in Grad C
  float feuchte();        // Luftfeuchte in %
  float bodenfeuchte();   // Bodenfeuchte in % (0=trocken, 100=nass)

  bool istOk() const { return _ok; }

private:
  DHT      _dht;
  float    _temp         = 0.0f;
  float    _luftFeuchte  = 0.0f;
  float    _bodenFeuchte = 0.0f;
  bool     _ok           = false;
  uint32_t _letzt        = 0;
};