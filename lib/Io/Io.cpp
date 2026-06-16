#include "Io.h"
#include <Wire.h>

// Die eine, globale IO-Instanz.
Io io;

bool Io::begin(int sda, int scl, uint8_t mcpAddr) {
  Wire.begin(sda, scl);
  _mcpOk = _mcp.begin_I2C(mcpAddr, &Wire);
  return _mcpOk;
}

void Io::pinMode(IoPin p, uint8_t mode) {
  switch (p.port) {
    case IoPort::Esp: ::pinMode(p.pin, mode);                break;
    case IoPort::Mcp: if (_mcpOk) _mcp.pinMode(p.pin, mode); break;
  }
}

int Io::digitalRead(IoPin p) {
  switch (p.port) {
    case IoPort::Esp: return ::digitalRead(p.pin);
    case IoPort::Mcp: return _mcpOk ? _mcp.digitalRead(p.pin) : LOW;
  }
  return LOW;
}

void Io::digitalWrite(IoPin p, uint8_t level) {
  switch (p.port) {
    case IoPort::Esp: ::digitalWrite(p.pin, level);                break;
    case IoPort::Mcp: if (_mcpOk) _mcp.digitalWrite(p.pin, level); break;
  }
}
