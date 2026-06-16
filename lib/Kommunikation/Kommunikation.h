// =============================================================================
//  Kommunikation  –  serieller Link zum Raspberry Pi (Dashboard)
// -----------------------------------------------------------------------------
//  Protokoll: EINE JSON-Nachricht pro Zeile (mit '\n' abgeschlossen).
//   Pi  -> ESP : Befehle,  z.B. {"cmd":"heizung","raum":"og2","set":1}
//   ESP -> Pi  : Status,   z.B. {"typ":"status","temp_eg":21.4}
//
//  update() liest eingehende Zeichen nicht-blockierend und ruft bei einer
//  vollstaendigen, gueltigen JSON-Zeile den per onBefehl() gesetzten Handler.
//  Was mit einem Befehl passiert, entscheidet main (verarbeiteBefehl()).
// =============================================================================
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>

class Kommunikation {
public:
  using BefehlHandler = std::function<void(JsonDocument&)>;

  explicit Kommunikation(Stream& port);
  void begin();
  void update();                                 // eingehende Zeilen verarbeiten
  void onBefehl(BefehlHandler h) { _handler = h; }

  // Status an den Pi senden (eine JSON-Zeile).
  void sendeStatus(const char* feld, float wert);
  void sendeStatus(const char* feld, bool  an);

private:
  void _verarbeite(const String& zeile);

  Stream&       _port;
  String        _puffer;
  BefehlHandler _handler;
};
