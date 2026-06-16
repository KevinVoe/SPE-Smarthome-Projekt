#include "Kommunikation.h"

Kommunikation::Kommunikation(Stream& port) : _port(port) {}

void Kommunikation::begin() {
  _puffer.reserve(128);
}

void Kommunikation::update() {
  while (_port.available()) {
    char c = (char)_port.read();
    if (c == '\n') {                       // Zeile komplett
      _verarbeite(_puffer);
      _puffer = "";
    } else if (c != '\r') {                // '\r' ignorieren
      _puffer += c;
      if (_puffer.length() > 200) _puffer = "";   // Ueberlauf-Schutz
    }
  }
}

void Kommunikation::_verarbeite(const String& zeile) {
  if (zeile.length() == 0) return;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, zeile);
  if (err) return;                          // ungueltiges JSON ignorieren

  if (_handler) _handler(doc);              // an main weiterreichen
}

void Kommunikation::sendeStatus(const char* feld, float wert) {
  JsonDocument doc;
  doc["typ"] = "status";
  doc[feld]  = wert;
  serializeJson(doc, _port);
  _port.println();
}

void Kommunikation::sendeStatus(const char* feld, bool an) {
  JsonDocument doc;
  doc["typ"] = "status";
  doc[feld]  = an;
  serializeJson(doc, _port);
  _port.println();
}
