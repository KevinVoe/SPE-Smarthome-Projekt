#include "Sensorik.h"
#include "Config.h"

Sensorik::Sensorik() : _dht(SENSORIK_DHT_PIN, SENSORIK_DHT_TYP) {}

void Sensorik::begin() {
  _dht.begin();
  pinMode(SENSORIK_WASSER_PIN, INPUT);
  // Solar-Pin ist ein ADC1-Eingang (input-only) -> kein pinMode noetig.
}

void Sensorik::update() {
  // DHT11/22 brauchen mindestens 2 Sekunden zwischen zwei Messungen.
  if (millis() - _letzt < 2000) return;
  _letzt = millis();

  // ── Solarpanel (immer, unabhaengig vom DHT) ────────────────────────────────
  // analogReadMilliVolts() nutzt die ESP32-ADC-Kalibrierung (genauer als analogRead).
  // 50/50-Spannungsteiler -> tatsaechliche Panelspannung = 2 x Pin-Spannung.
  float pinV = analogReadMilliVolts(SENSORIK_SOLAR_PIN) / 1000.0f;
  _pv = roundf(pinV * 2.0f * 10.0f) / 10.0f;   // auf 0.1 V runden (weniger Telemetrie-Jitter)

  // ── Water Sensor (Gewaechshaus): hoher Rohwert = trocken -> invertiert mappen ─
  int  roh = analogRead(SENSORIK_WASSER_PIN);
  long p   = map(roh, SENSORIK_WASSER_TROCKEN, SENSORIK_WASSER_NASS, 0, 100);
  _boden   = (float)constrain(p, 0, 100);

  // ── DHT (Temperatur + Feuchte) ─────────────────────────────────────────────
  float t = _dht.readTemperature();
  float f = _dht.readHumidity();
  if (isnan(t) || isnan(f)) {   // ungueltig (Sensor nicht angeschlossen/defekt)
    _ok = false;
    return;
  }
  _temp    = t;
  _feuchte = f;
  _ok      = true;
}

float Sensorik::temperatur() { return _temp; }
float Sensorik::feuchte()    { return _feuchte; }
float Sensorik::pvSpannung() { return _pv; }
float Sensorik::bodenfeuchte() { return _boden; }
