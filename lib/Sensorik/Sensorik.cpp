#include "Sensorik.h"

// -----------------------------------------------------------------------------
//  ECHTE SENSOREN ANBINDEN (Schritt fuer Schritt):
//   1) In platformio.ini die passende lib_deps-Zeile einkommentieren.
//   2) Hier den Header einbinden und ein Objekt anlegen, z.B.:
//        #include <BH1750.h>
//        static BH1750 bh1750;
//   3) In begin()  ->  bh1750.begin();
//      In update() ->  _lux = bh1750.readLightLevel();
//   Sensoren laut Config:
//     - DS18B20 (Temperatur)  : OneWire + DallasTemperature, Pin PIN_DS18B20
//     - BH1750  (Helligkeit)  : I2C-Adresse ADDR_BH1750
//     - SHT31   (Temp/Feuchte): I2C-Adresse ADDR_SHT31
// -----------------------------------------------------------------------------

void Sensorik::begin() {
  // TODO: echte sensor.begin()-Aufrufe hier einfuegen.
}

void Sensorik::update() {
  if (millis() - _letzt < 1000) return;   // einmal pro Sekunde reicht
  _letzt = millis();
  // TODO: echte Messwerte einlesen, z.B.:
  //   _temp[0] = dallas.getTempCByIndex(0);
  //   _lux     = bh1750.readLightLevel();
}

float Sensorik::temperatur(uint8_t etage) { return _temp[etage < 3 ? etage : 0]; }
float Sensorik::helligkeit()              { return _lux; }
float Sensorik::feuchte()                 { return _feuchte; }

void Sensorik::simTemperatur(uint8_t etage, float t) {
  if (etage < 3) _temp[etage] = t;
}
