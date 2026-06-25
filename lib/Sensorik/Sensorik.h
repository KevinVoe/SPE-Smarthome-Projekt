// =============================================================================
//  Sensorik  –  Temperatur und Feuchte ueber DHT-Sensor (DHT11 oder DHT22)
// -----------------------------------------------------------------------------
//  Ein einziger DHT-Sensor liefert BEIDE Werte (Temperatur + Feuchte) ueber
//  ein einzelnes Datenkabel an GPIO 13.
//
//  Wie es funktioniert:
//    - Der ESP32 schickt einen kurzen Startimpuls auf das Datenkabel
//    - Der DHT antwortet mit einem 40-Bit-Datenpaket (Temp + Feuchte + Pruefsumme)
//    - Das Modul liest dieses Paket aus und speichert die Werte intern
//    - update() macht das einmal pro 2 Sekunden (DHT braucht diese Pause)
//
//  Aufruf aus main.cpp:
//    sensorik.begin();              // einmal in setup()
//    sensorik.update();             // einmal pro loop()
//    float t = sensorik.temperatur();
//    float f = sensorik.feuchte();
// =============================================================================
#pragma once
#include <Arduino.h>
#include <DHT.h>

class Sensorik {
public:
  Sensorik();             // initialisiert DHT mit Pin + Typ aus Config.h
  void begin();
  void update();          // zyklisch aufrufen (intern: nur alle 2s neu einlesen)

  float temperatur();     // Grad Celsius, z.B. 22.5
  float feuchte();        // Relative Feuchte in %, z.B. 48.0

  // Letzter Lesezyklus erfolgreich? (false = Sensor nicht angeschlossen/defekt)
  bool istOk() const { return _ok; }

private:
  DHT      _dht;                  // DHT-Treiber-Objekt
  float    _temp    = 0.0f;
  float    _feuchte = 0.0f;
  bool     _ok      = false;
  uint32_t _letzt   = 0;
};