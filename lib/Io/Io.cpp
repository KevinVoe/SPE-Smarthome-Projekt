#include "Io.h"
#include <Wire.h>

// Die eine, globale IO-Instanz.
Io io;

bool Io::begin(int sda, int scl, uint8_t addrIn, uint8_t addrOut) {
  Wire.begin(sda, scl);

  _inOk  = _mcpIn.begin_I2C(addrIn,  &Wire);
  _outOk = _mcpOut.begin_I2C(addrOut, &Wire);

  // Eingangs-MCP: alle 16 Pins als INPUT_PULLUP (interner Pull-up ~100k).
  if (_inOk)
    for (uint8_t p = 0; p < 16; p++) _mcpIn.pinMode(p, INPUT_PULLUP);

  // Ausgangs-MCP: alle 16 Pins als OUTPUT und LOW (Transistoren aus, active-high).
  if (_outOk)
    for (uint8_t p = 0; p < 16; p++) {
      _mcpOut.pinMode(p, OUTPUT);
      _mcpOut.digitalWrite(p, LOW);
    }

  return _inOk && _outOk;
}

void Io::pinMode(IoPin p, uint8_t mode) {
  switch (p.port) {
    case IoPort::Esp:    ::pinMode(p.pin, mode);                   break;
    case IoPort::McpIn:  if (_inOk)  _mcpIn.pinMode(p.pin, mode);  break;
    case IoPort::McpOut: if (_outOk) _mcpOut.pinMode(p.pin, mode); break;
  }
}

int Io::digitalRead(IoPin p) {
  switch (p.port) {
    case IoPort::Esp:    return ::digitalRead(p.pin);
    case IoPort::McpIn:  return _inOk  ? _mcpIn.digitalRead(p.pin)  : HIGH;  // HIGH = idle (Pullup)
    case IoPort::McpOut: return _outOk ? _mcpOut.digitalRead(p.pin) : LOW;
  }
  return LOW;
}

void Io::digitalWrite(IoPin p, uint8_t level) {
  switch (p.port) {
    case IoPort::Esp:    ::digitalWrite(p.pin, level);                    break;
    case IoPort::McpIn:  if (_inOk)  _mcpIn.digitalWrite(p.pin, level);   break;  // selten sinnvoll
    case IoPort::McpOut: if (_outOk) _mcpOut.digitalWrite(p.pin, level);  break;
  }
}
