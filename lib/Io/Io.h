// =============================================================================
//  Io  –  Abstraktion fuer digitale Pins (ESP32 ODER MCP23017)
// -----------------------------------------------------------------------------
//  Es gibt drei "Ports":
//     Esp     -> direkter GPIO am ESP32
//     McpIn   -> Pin am EINGANGS-MCP23017  (alle Pins INPUT_PULLUP)
//     McpOut  -> Pin am AUSGANGS-MCP23017  (alle Pins OUTPUT, schalten
//                Transistoren; active-high: HIGH = Transistor leitet)
//
//  Ein Modul arbeitet nur mit dem Typ "IoPin" und ruft io.digitalRead(),
//  io.digitalWrite(), io.pinMode() auf. WO der Pin sitzt, steht ALLEIN in der
//  Config (espPin/mcpInPin/mcpOutPin) -> Umstecken ohne Modul-Aenderung.
//
//  Hinweis: Der PCA9685 (PWM/Servos) liegt am selben I2C-Bus, gehoert aber
//  NICHT in diese Schicht - er wird ueber Adafruit_PWMServoDriver / Servoaktor
//  angesteuert (PWM ist kein digitales IO).
// =============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

// Auf welchem "Port" sitzt ein Pin?
// Hinweis: NICHT "ESP32" verwenden - das ist ein vordefiniertes Makro des Cores!
enum class IoPort : uint8_t {
  Esp,      // direkter GPIO am ESP32
  McpIn,    // Pin am Eingangs-MCP23017  (INPUT_PULLUP)
  McpOut    // Pin am Ausgangs-MCP23017  (OUTPUT, Transistoren, active-high)
};

// Beschreibt EINEN logischen Pin - unabhaengig vom Ort.
struct IoPin {
  IoPort  port;
  uint8_t pin;
};

// Komfort-Factories fuer die Config (constexpr -> keine Laufzeitkosten).
constexpr IoPin espPin   (uint8_t p) { return IoPin{ IoPort::Esp,    p }; }
constexpr IoPin mcpInPin (uint8_t p) { return IoPin{ IoPort::McpIn,  p }; }
constexpr IoPin mcpOutPin(uint8_t p) { return IoPin{ IoPort::McpOut, p }; }

// Sentinel fuer "nicht belegt".
constexpr IoPin IO_NICHT_BELEGT = IoPin{ IoPort::Esp, 255 };

class Io {
public:
  // I2C starten und BEIDE MCP23017 initialisieren. EINMAL in setup() aufrufen,
  // BEVOR Module ihr begin() ausfuehren (und vor pwm.begin() des PCA9685).
  // Initialisiert automatisch:
  //   - Eingangs-MCP: alle 16 Pins INPUT_PULLUP
  //   - Ausgangs-MCP: alle 16 Pins OUTPUT, LOW (Transistoren aus)
  // Gibt true zurueck, wenn BEIDE MCP geantwortet haben.
  bool begin(int sda, int scl, uint8_t addrIn, uint8_t addrOut);

  void pinMode(IoPin p, uint8_t mode);          // INPUT / INPUT_PULLUP / OUTPUT
  int  digitalRead(IoPin p);
  void digitalWrite(IoPin p, uint8_t level);

  bool mcpInOk()  const { return _inOk; }
  bool mcpOutOk() const { return _outOk; }

private:
  Adafruit_MCP23X17 _mcpIn;     // Eingaenge  (Adresse addrIn)
  Adafruit_MCP23X17 _mcpOut;    // Ausgaenge  (Adresse addrOut)
  bool _inOk  = false;
  bool _outOk = false;
};

// Eine globale Instanz - in Io.cpp angelegt, von allen Modulen genutzt.
extern Io io;
