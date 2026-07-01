// =============================================================================
//  Ultraschall  –  HC-SR04 Abstandssensor (Trigger/Echo)
// -----------------------------------------------------------------------------
//  Misst GEDROSSELT (alle ULTRASCHALL_INTERVALL_MS) den Abstand zum naechsten
//  Objekt. Genutzt fuer die Garagen-Automatik: Objekt naeher als GARAGE_OBJEKT_CM
//  -> Garage oeffnet (s. main::garageBedienen).
//
//  ACHTUNG Hardware: HC-SR04-Echo liefert 5 V -> Spannungsteiler auf 3,3 V noetig!
//  Echo-Pin sollte input-only sein (z.B. GPIO35).
//
//  Aufruf:
//    ultraschall.begin();     // einmal in setup()
//    ultraschall.update();    // in jeder loop() (intern gedrosselt)
//    float d = ultraschall.distanzCm();
// =============================================================================
#pragma once
#include <Arduino.h>

class Ultraschall {
public:
  Ultraschall(int trigPin, int echoPin);
  void  begin();
  void  update();                            // gedrosselt messen
  float distanzCm() const { return _cm; }    // letzte Messung in cm (999 = kein Echo)

private:
  int      _trig, _echo;
  float    _cm    = 999.0f;
  uint32_t _letzt = 0;
};
