#include "Kommunikation.h"

// ── Konvertierungs-Hilfen: Soll-Prozentwerte -> Bool-Felder im Pi-Schema ────
// Jalousie: 0=offen,100=ganz beschattet (Soll)  ->  blind: 1=oben,0=unten (Pi)
static int jalousieZuBlind(int16_t prozent) {
  return (prozent < 50) ? 1 : 0;
}
// Klappe/Dachfenster: 0=zu,100=auf (Soll)  ->  0/1 (Pi)
static int klappeZuBool(int16_t prozent) {
  return (prozent > 50) ? 1 : 0;
}

Kommunikation::Kommunikation(Stream& port) : _port(port) {}

void Kommunikation::begin() {
  _puffer.reserve(128);
  _letzteTelemetrie.reserve(400);   // ~Frame-Groesse, vermeidet Heap-Reallocs
}

// ── Pi -> ESP: eingehende Zeilen ───────────────────────────────────────────
void Kommunikation::update_empfang() {
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

// ── ESP -> Pi: einzelne Statuswerte ────────────────────────────────────────
void Kommunikation::sendeStatus(const char* feld, float wert) {
  JsonDocument doc;
  doc["typ"] = "status";
  doc[feld]  = wert;
  serializeJson(doc, _port);
  _port.println();
}

void Kommunikation::sendeStatus(const char* feld, bool  an) {
  JsonDocument doc;
  doc["typ"] = "status";
  doc[feld]  = an;
  serializeJson(doc, _port);
  _port.println();
}

// ── ESP -> Pi: Telemetrie-Frame aus Soll + Kontext bauen ───────────────────
//  Felder OHNE Soll-/Kontext-Quelle sind Blanko-Platzhalter (// TODO).
String Kommunikation::_baueTelemetrieJson(const Soll& s, const Kontext& k) {
  JsonDocument doc;
  doc["type"] = "telemetry";

  // temp: Mittelwert ueber alle Etagen (Kontext). Falls eine bestimmte Etage
  // gewuenscht ist, hier anpassen.
  doc["temp"]     = (k.temperatur[0] + k.temperatur[1] + k.temperatur[2]) / 3.0f;
  doc["humidity"] = 0;      // TODO: keine Quelle in Soll/Kontext (Feuchtesensor)
  doc["tv"]       = 0;      // TODO: keine Quelle in Soll/Kontext (Fernseher)

  JsonObject roof = doc["roof"].to<JsonObject>();
  roof["pv_voltage"] = 0.0f;                                  // TODO: keine Quelle (PV-Sensor)
  roof["skylight1"]  = 0;                                     // TODO: kein Soll-Feld (EG-Dachfenster)
  roof["skylight2"]  = klappeZuBool(s.dachfensterOG2.wert);   // OG2/E2
  roof["ac"]         = s.klimaanlage.wert;                    // 0/1

  JsonObject floors = doc["floors"].to<JsonObject>();

  // ── EG (Soll-Index 0) ────────────────────────────────────────────────────
  JsonObject eg = floors["EG"].to<JsonObject>();
  eg["mode"]   = 0;   // TODO: keine 0..4-Quelle (klimaModus deckt nur 0..2)
  eg["blind1"] = jalousieZuBlind(s.jalousie[0][0].wert);
  eg["blind2"] = jalousieZuBlind(s.jalousie[0][1].wert);
  eg["heat1"]  = s.heizung[0].wert;   // Soll kennt nur 1 Heizwert/Etage ->
  eg["heat2"]  = s.heizung[0].wert;   //   auf beide Heizkreise gespiegelt
  eg["light"]  = s.licht[0].wert;     // Stufe 0..3

  // ── E1 (Soll-Index 1) ────────────────────────────────────────────────────
  JsonObject e1 = floors["E1"].to<JsonObject>();
  e1["mode"]   = 0;   // TODO
  e1["blind1"] = jalousieZuBlind(s.jalousie[1][0].wert);
  e1["blind2"] = jalousieZuBlind(s.jalousie[1][1].wert);
  e1["heat1"]  = s.heizung[1].wert;
  e1["heat2"]  = s.heizung[1].wert;
  e1["light"]  = s.licht[1].wert;
  e1["party"]  = 0;   // TODO: Soll.disco ist OG2/E2 zugeordnet, nicht E1.
                      //       Hier bewusst Blanko, damit das Schema (party @E1)
                      //       erfuellt ist; echte Quelle fuer E1-Party fehlt.

  // ── E2 / OG2 (Soll-Index 2) ──────────────────────────────────────────────
  JsonObject e2 = floors["E2"].to<JsonObject>();
  e2["mode"]   = 0;   // TODO
  e2["blind1"] = jalousieZuBlind(s.jalousie[2][0].wert);
  e2["blind2"] = jalousieZuBlind(s.jalousie[2][1].wert);
  e2["heat1"]  = s.heizung[2].wert;
  e2["heat2"]  = s.heizung[2].wert;
  e2["light"]  = s.licht[2].wert;
  // Hinweis: Im Zielschema hat E2 KEIN "party". s.disco (OG2/Disco) wird daher
  // aktuell NICHT uebertragen - bei Bedarf hier ergaenzen.

  doc["elevator"]["floor"] = 0;   // TODO: keine Quelle (Aufzug-Modul)

  String out;
  if (doc.overflowed()) return out;   // leer -> wird nie gesendet
  serializeJson(doc, out);
  return out;
}

// ── ESP -> Pi: Telemetrie senden (change-detected + Heartbeat) ─────────────
void Kommunikation::update(const Soll& s, const Kontext& k) {
  String json = _baueTelemetrieJson(s, k);
  if (json.length() == 0) return;     // Overflow -> nichts Kaputtes senden

  uint32_t jetzt    = millis();
  bool     geaendert = (json != _letzteTelemetrie);
  bool     heartbeat = (jetzt - _letzterSendeMs >= TELEMETRIE_MAX_INTERVALL_MS);

  if (_ersterFrame || geaendert || heartbeat) {
    _port.println(json);
    _letzteTelemetrie = json;
    _letzterSendeMs   = jetzt;
    _ersterFrame      = false;
  }
}
