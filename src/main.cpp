#include <Arduino.h>
#include "config.h"
#include "Heizung.h"

//Objekte anlegen

Heizung heizungEG(HEIZUNG_EG_TASTER_PIN, HEIZUNG_EG_LED_PIN);
Heizung heizung1OG(HEIZUNG_1OG_TASTER_PIN, HEIZUNG_1OG_LED_PIN);
Heizung heizung2OG(HEIZUNG_2OG_TASTER_PIN, HEIZUNG_2OG_LED_PIN);

void setup() {

  //Heizung
  heizungEG.begin();
  heizung1OG.begin();
  heizung2OG.begin();


}

void loop() {

 //Heizung
  heizungEG.update();
  heizung1OG.update();
  heizung2OG.update();


}