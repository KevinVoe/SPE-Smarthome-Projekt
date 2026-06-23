// =============================================================================
//  Kommunikation  –  serieller Link zum Raspberry Pi (Dashboard)
// -----------------------------------------------------------------------------
//  Protokoll: EINE JSON-Nachricht pro Zeile (mit '\n' abgeschlossen).
//   Pi  -> ESP : Befehle,  z.B. {"cmd":"heizung","raum":"og2","set":1}
//   ESP -> Pi  : Telemetrie (vollstaendiger Hauszustand) + einzelne Status.
//
//  TELEMETRIE (ESP -> Pi):
//   * update(s, k) wird OFT aufgerufen (z.B. jede Loop-Iteration aus main).
//   * Gesendet wird nur, wenn sich der Frame tatsaechlich GEAENDERT hat
//     ODER seit dem letzten Senden >= TELEMETRIE_MAX_INTERVALL_MS vergangen
//     sind (Heartbeat - der Pi sieht so, dass die Verbindung noch lebt).
//
//  EMPFANG (Pi -> ESP):
//   * update_empfang() liest eingehende Zeichen nicht-blockierend und ruft bei
//     einer vollstaendigen, gueltigen JSON-Zeile den per onBefehl() gesetzten
//     Handler. Was mit einem Befehl passiert, entscheidet main.
// =============================================================================
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include "Regelung.h"   // Soll + Kontext fuer update()
 
// Spaetestens nach dieser Zeit wird ein Frame gesendet, auch ohne Aenderung
// (Heartbeat). Bei Aenderung wird sofort gesendet.
constexpr uint32_t TELEMETRIE_MAX_INTERVALL_MS = 5000;
 
class Kommunikation {
public:
  using BefehlHandler = std::function<void(JsonDocument&)>;
 
  explicit Kommunikation(Stream& port);
  void begin();
 
  // Pi -> ESP: eingehende Befehlszeilen verarbeiten (nicht-blockierend).
  void update_empfang();
  void onBefehl(BefehlHandler h) { _handler = h; }
 
  // ESP -> Pi: einzelnen Status senden (eine JSON-Zeile).
  void sendeStatus(const char* feld, float wert);
  void sendeStatus(const char* feld, bool  an);
 
  // ESP -> Pi: Telemetrie. Oft aufrufbar (z.B. jede Loop).
  // Sendet NUR bei tatsaechlicher Aenderung des Frames, mindestens aber
  // alle TELEMETRIE_MAX_INTERVALL_MS (Heartbeat).
  // Rueckgabe: true, wenn in DIESEM Aufruf tatsaechlich gesendet wurde.
  bool update(const Soll& s, const Kontext& k);
 
  // Der zuletzt GESENDETE Telemetrie-Frame (zum Mitloggen/Pruefen auf USB).
  const String& letzteTelemetrie() const { return _letzteTelemetrie; }
 
private:
  void   _verarbeite(const String& zeile);
  String _baueTelemetrieJson(const Soll& s, const Kontext& k);
 
  Stream&       _port;
  String        _puffer;            // Empfangspuffer (Pi -> ESP)
  BefehlHandler _handler;
 
  String        _letzteTelemetrie;  // zuletzt gesendeter Frame (Vergleichsbasis)
  uint32_t      _letzterSendeMs = 0;
  bool          _ersterFrame    = true;   // erzwingt erstes Senden
};