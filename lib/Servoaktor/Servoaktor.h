// =============================================================================
//  Servoaktor  –  alle Servos am PCA9685 (Jalousien, Dachfenster, Garage)
// -----------------------------------------------------------------------------
//  Funktions-Modul (freie Funktionen) - direkt aus Regelung/anwenden aufrufbar.
//  Jeder Servo faehrt zwischen zwei in Config.h hinterlegten Endlagen (ZU/AUF),
//  und zwar SANFT (in kleinen Schritten) - dafuer MUSS servosUpdate() in jeder
//  loop() laufen. Die Funktionen setzen nur das Ziel; das Fahren macht update().
//
//  Kanal- und Endlagen-Tabellen stehen in Config.h (SERVO_JALOUSIE usw.).
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Config.h"   // Etage, Seite, Position, SERVO_*-Tabellen, ADDR_PCA9685

// EINMAL in setup() - NACH io.begin() (der I2C-Bus muss bereits laufen).
void servosBegin();

// EINMAL pro loop() - bewegt alle Servos einen Schritt Richtung Ziel.
void servosUpdate();

// Zielposition setzen (gefahren wird dann sanft in servosUpdate()):
void fahreJalousie(Position p, Etage etage, Seite seite);   // 6 Servos
void fahreDachfenster(Position p, Seite seite);             // 2 Servos (beide OG2)
void fahreGarage(Position p);                               // 1 Servo
