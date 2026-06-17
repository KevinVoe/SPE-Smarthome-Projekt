#include <Arduino.h>
#include "Config.h"
#include "Io.h"

//Ziel des Programms: Testen der PWM Mosfet ansteuerung der 12V LED Strips. Es sollen variable PWM duty cycles rotiert werden zum testen

void setup() {
    Serial.begin(115200);                              // Debug-Konsole (USB)
    pinMode(LICHT_STRIP_DG_PIN, OUTPUT);

}

void loop() {
    for (int i = 0; i <= 3; i ++) {
        if(i == 0 ){
            //analog write beenden
            analogWrite(LICHT_STRIP_DG_PIN, 0);
            Serial.println("LEDs aus");
        }else if(i == 1){
            analogWrite(LICHT_STRIP_DG_PIN, LOW_BRIGHTNESS_DUTY_CYCLE);
            Serial.println("LEDs low brightness");
        }else if(i == 2){
            analogWrite(LICHT_STRIP_DG_PIN, MEDIUM_BRIGHTNESS_DUTY_CYCLE);
            Serial.println("LEDs medium brightness");
        }else{ 
            analogWrite(LICHT_STRIP_DG_PIN, 255);
            Serial.println("LEDs high brightness");
        }
        delay(2000);
    }
}