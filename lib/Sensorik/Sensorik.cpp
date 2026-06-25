#include "Sensorik.h"
#include "Config.h"

Sensorik::Sensorik() : _dht(SENSORIK_DHT_PIN, SENSORIK_DHT_TYP) {}

void Sensorik::begin() {
  _dht.begin();
}

void Sensorik::update() {
  // DHT11/22 brauchen mindestens 2 Sekunden zwischen zwei Messungen
  if (millis() - _letzt < 2000) return;
  _letzt = millis();

  float t = _dht.readTemperature();
  float f = _dht.readHumidity();

  // isnan() prueft ob der Sensor einen ungueltigen Wert geliefert hat
  // (passiert wenn der Sensor nicht angeschlossen ist)
  if (isnan(t) || isnan(f)) {
    _ok = false;
    return;
  }

  _temp    = t;
  _feuchte = f;
  _ok      = true;
}

float Sensorik::temperatur() { return _temp; }
float Sensorik::feuchte()    { return _feuchte; }