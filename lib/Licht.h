#ifndef LICHT_H
#define LICHT_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

class Licht {
  public:
    Licht();
    void begin();
    void update();

  private:
    Adafruit_NeoPixel _strip;
    void _ledsAn();
    void _ledsAus();
};

#endif