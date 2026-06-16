#ifndef HEIZUNG_H
#define HEIZUNG_H

#include <Arduino.h>
#include "config.h"

class Heizung {
  public:
    Heizung(int tasterPin, int ledPin);
    void begin();
    void update();
    bool istAn();

  private:
    int _tasterPin;
    int _ledPin;
    bool _heizungAn;
    bool _letzterZustand;
    unsigned long _letzteAenderung;
    const unsigned long _entprellzeit = 50;
};

#endif