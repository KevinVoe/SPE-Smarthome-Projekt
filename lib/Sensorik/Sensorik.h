// =============================================================================
//  Sensorik  –  buendelt alle Sensoren hinter einer einfachen Schnittstelle
// -----------------------------------------------------------------------------
//  WICHTIG: Liefert aktuell SIMULIERTE Werte, damit die gesamte Regelung
//  schon getestet werden kann, BEVOR die Sensoren verdrahtet sind.
//  Die Module/Regelung fragen nur temperatur()/helligkeit()/feuchte() ab und
//  merken nicht, ob der Wert simuliert oder echt ist.
//  TODO: echte Treiber einbauen (siehe Hinweise in Sensorik.cpp).
// =============================================================================
#pragma once
#include <Arduino.h>

class Sensorik {
public:
  void begin();
  void update();                       // zyklisch Sensoren einlesen

  float temperatur(uint8_t etage);     // °C je Etage (0=EG, 1=OG1, 2=OG2)
  float helligkeit();                  // Lux  (BH1750)
  float feuchte();                     // %rF  (SHT31)

  // Nur fuer die Simulation/Tests: Werte von aussen setzen
  // (z.B. spaeter per Pi-Befehl "simuliere Temperatur").
  void simTemperatur(uint8_t etage, float t);
  void simHelligkeit(float lux) { _lux = lux; }

private:
  float    _temp[3] = { 21.0f, 21.0f, 21.0f };   // Startwerte (Simulation)
  float    _lux     = 100.0f;
  float    _feuchte = 45.0f;
  uint32_t _letzt   = 0;
};
