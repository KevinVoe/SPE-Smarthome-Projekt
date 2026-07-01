#include "Ultraschall.h"
#include "Config.h"   // ULTRASCHALL_INTERVALL_MS

Ultraschall::Ultraschall(int trigPin, int echoPin) : _trig(trigPin), _echo(echoPin) {}

void Ultraschall::begin() {
  pinMode(_trig, OUTPUT);
  digitalWrite(_trig, LOW);
  pinMode(_echo, INPUT);
}

void Ultraschall::update() {
  if (millis() - _letzt < ULTRASCHALL_INTERVALL_MS) return;   // Messtakt drosseln
  _letzt = millis();

  // 10-us-Trigger-Puls.
  digitalWrite(_trig, LOW);  delayMicroseconds(2);
  digitalWrite(_trig, HIGH); delayMicroseconds(10);
  digitalWrite(_trig, LOW);

  // Echo-Dauer messen; Timeout ~4 ms (~60 cm) - reicht fuer die 20-cm-Schwelle und
  // haelt die Blockierzeit klein. (Der Aufzug taktet ohnehin per Hardware-Timer.)
  unsigned long dauer = pulseIn(_echo, HIGH, 4000UL);
  if (dauer == 0) { _cm = 999.0f; return; }       // kein Echo -> "sehr weit"
  _cm = dauer * 0.0343f / 2.0f;                    // Schallgeschwindigkeit (cm/us), Hin+Rueck
}
