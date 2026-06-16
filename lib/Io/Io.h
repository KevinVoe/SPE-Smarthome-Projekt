// =============================================================================
//  Io  –  Abstraktion fuer digitale Pins (ESP32 ODER MCP23017)
// -----------------------------------------------------------------------------
//  Ein Modul arbeitet nur mit dem Typ "IoPin" und ruft io.digitalRead(),
//  io.digitalWrite(), io.pinMode() auf. Ob der Pin physisch am ESP32 oder am
//  Port-Expander MCP23017 haengt, steht ALLEIN in der Config (espPin/mcpPin).
//  -> Pins lassen sich spaeter umstecken, ohne ein Modul anzufassen.
// =============================================================================
#pragma once
#include <Arduino.h>
#include <Adafruit_MCP23X17.h>

// Auf welchem "Port" sitzt ein Pin?
// Hinweis: NICHT "ESP32" verwenden - das ist ein vordefiniertes Makro des Cores!
enum class IoPort : uint8_t {
  Esp,         // direkter GPIO am ESP32
  Mcp          // Pin am I2C-Port-Expander MCP23017
};

// Beschreibt EINEN logischen Pin - unabhaengig vom Ort.
struct IoPin {
  IoPort  port;
  uint8_t pin;
};

// Komfort-Factories fuer die Config (constexpr -> keine Laufzeitkosten).
constexpr IoPin espPin(uint8_t p) { return IoPin{ IoPort::Esp, p }; }
constexpr IoPin mcpPin(uint8_t p) { return IoPin{ IoPort::Mcp, p }; }

// Sentinel fuer "nicht belegt".
constexpr IoPin IO_NICHT_BELEGT = IoPin{ IoPort::Esp, 255 };

class Io {
public:
  // I2C-Bus starten und MCP23017 initialisieren. Einmal in setup() aufrufen,
  // BEVOR Module ihr begin() ausfuehren. Gibt true zurueck, wenn der MCP da ist.
  bool begin(int sda, int scl, uint8_t mcpAddr);

  void pinMode(IoPin p, uint8_t mode);          // INPUT / INPUT_PULLUP / OUTPUT
  int  digitalRead(IoPin p);
  void digitalWrite(IoPin p, uint8_t level);

  bool mcpOk() const { return _mcpOk; }

private:
  Adafruit_MCP23X17 _mcp;
  bool _mcpOk = false;
};

// Eine globale Instanz - in Io.cpp angelegt, von allen Modulen genutzt.
extern Io io;
