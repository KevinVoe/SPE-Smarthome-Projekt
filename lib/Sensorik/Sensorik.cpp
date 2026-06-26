#include "Sensorik.h"
#include "Config.h"

Sensorik::Sensorik() : _dht(SENSORIK_DHT_PIN, SENSORIK_DHT_TYP) {}

void Sensorik::begin() {
  _dht.begin();
  pinMode(SENSORIK_WASSER_PIN, INPUT);
}

void Sensorik::update() {
  // ── DHT11/22: nur alle 2 Sekunden ─────────────────────────────────────────
  if (millis() - _letzt >= 2000) {
    _letzt = millis();

    float t = _dht.readTemperature();
    float f = _dht.readHumidity();

    if (isnan(t) || isnan(f)) {
      _ok = false;
    } else {
      _temp        = t;
      _luftFeuchte = f;
      _ok          = true;
    }
  }

  // ── Water Sensor: analog, jeden loop() lesbar ──────────────────────────────
  //  Hoher Rohwert = trocken, niedriger Rohwert = nass → daher invertiert mappen
  int rohwert = analogRead(SENSORIK_WASSER_PIN);

  float prozent = map(rohwert,
                      SENSORIK_WASSER_TROCKEN,
                      SENSORIK_WASSER_NASS,
                      0, 100);

  if (prozent < 0)   prozent = 0;
  if (prozent > 100) prozent = 100;

  _bodenFeuchte = prozent;
}

float Sensorik::temperatur()   { return _temp; }
float Sensorik::feuchte()      { return _luftFeuchte; }
float Sensorik::bodenfeuchte() { return _bodenFeuchte; }