#include <cstring>
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
  // Simulierte Uhrzeit (Stunden), quantisiert auf 10 Sim-Minuten: aendert sich
  // nur alle 10 Sim-Min -> der Frame-Vergleich loest NICHT jede Loop ein Senden
  // aus, sondern hoechstens alle 10 Sim-Minuten (plus Heartbeat).
  doc["time"] = floorf(k.stunde * 6.0f) / 6.0f;   // 6 Slots/h = 10-Minuten-Raster
  doc["temp"]     = k.temperatur;   // EIN Sensor fuers ganze Haus
  doc["humidity"] = k.feuchte;
  doc["auto_active"] = !k.automatikAus;   // true = Automatik laeuft, false = eingefroren (Handbetrieb)

  JsonObject outdoor = doc["outdoor"].to<JsonObject>();
  outdoor["ext_light"] = s.licht[0].wert;
  outdoor["door_light"] = s.licht[1].wert;
  outdoor["whirlpool"] = s.whirlpool.wert;   // 0/1 (an/aus)
  outdoor["garage"] = 0;
  outdoor["garden_light"] = s.licht[5].wert;   // Stufe 0..3
  outdoor["humid_garden"] = k.bodenfeuchte;   // Gewaechshaus-Bodenfeuchte (% 0..100)

  JsonObject roof = doc["roof"].to<JsonObject>();
  roof["pv_voltage"] = k.pvSpannung;                          // Solar-Spannung (V)
  roof["skylight1"]  = klappeZuBool(s.dachfensterOG2.wert);   // beide Dachfenster sind im OG2
  roof["skylight2"]  = klappeZuBool(s.dachfensterOG2.wert);   // (ein Soll-Feld steuert beide)
  roof["ac"]         = (s.klimaanlage.wert > 0) ? 1 : 0;      // 0/1 (Duty intern, hier nur an/aus)
 
  JsonObject floors = doc["floors"].to<JsonObject>();
 
  // ── EG (Soll-Index 0) ────────────────────────────────────────────────────
  JsonObject eg = floors["EG"].to<JsonObject>();
  eg["blind1"] = jalousieZuBlind(s.jalousie[0][0].wert);
  eg["heat"]  = s.heizung[0].wert;    // 0/1  rote LED (Heizung)
  eg["cool"]  = s.kuehlLed[0].wert;   // 0/1  blaue LED (Kuehlung)
  eg["light"]  = s.licht[2].wert;     // Stufe 0..3
 
  // ── E1 (Soll-Index 1) ────────────────────────────────────────────────────
  JsonObject e1 = floors["E1"].to<JsonObject>();
  e1["blind1"] = jalousieZuBlind(s.jalousie[1][0].wert);
  e1["blind2"] = jalousieZuBlind(s.jalousie[1][1].wert);
  e1["heat"]  = s.heizung[1].wert;
  e1["cool"]  = s.kuehlLed[1].wert;
  e1["light"]  = s.licht[3].wert;
  e1["tv"]     = 0;   // TODO: keine Quelle (Fernseher)
 
  // ── E2 / OG2 (Soll-Index 2) ──────────────────────────────────────────────
  JsonObject e2 = floors["E2"].to<JsonObject>();
  e2["blind1"] = jalousieZuBlind(s.jalousie[2][0].wert);
  e2["blind2"] = jalousieZuBlind(s.jalousie[2][1].wert);
  e2["heat"]  = s.heizung[2].wert;
  e2["cool"]  = s.kuehlLed[2].wert;
  e2["light"]  = s.licht[4].wert;
  e2["party"]  = s.disco.wert;
 
  doc["elevator"]["floor"] = k.aufzugEtage;   // 0=EG, 1=OG1, 2=OG2
 
  String out;
  if (doc.overflowed()) return out;   // leer -> wird nie gesendet
  serializeJson(doc, out);
  return out;
}
 
// ── ESP -> Pi: Telemetrie senden (change-detected + Heartbeat) ─────────────
bool Kommunikation::update(const Soll& s, const Kontext& k) {
  String json = _baueTelemetrieJson(s, k);
  if (json.length() == 0) return false;   // Overflow -> nichts Kaputtes senden
 
  uint32_t jetzt    = millis();
  bool     geaendert = (json != _letzteTelemetrie);
  bool     heartbeat = (jetzt - _letzterSendeMs >= TELEMETRIE_MAX_INTERVALL_MS);
 
  if (_ersterFrame || geaendert || heartbeat) {
    _port.println(json);
    _letzteTelemetrie = json;
    _letzterSendeMs   = jetzt;
    _ersterFrame      = false;
    return true;                          // hat gesendet
  }
  return false;                           // nichts zu tun
}

static int etageAusName(const char* f) {
  if (!f) return -1;
  if (!strcmp(f, "EG")) return 0;
  if (!strcmp(f, "E1")) return 1;
  if (!strcmp(f, "E2")) return 2;
  return -1;
}

void behandleBefehl(JsonDocument& doc, DashboardState& dash) {
  const char* cmd = doc["cmd"] | "";
  if (!*cmd) return;
  int val = doc["value"] | 0;
  int et  = etageAusName(doc["floor"] | (const char*)nullptr);

  if      (!strcmp(cmd, "blind1")) { if (et >= 0) dashSetze(dash.blind[et][0], val); }
  else if (!strcmp(cmd, "blind2")) { if (et >= 0) dashSetze(dash.blind[et][1], val); }
  else if (!strcmp(cmd, "heat"))   { if (et >= 0) dashSetze(dash.heat[et],  val); }
  else if (!strcmp(cmd, "light"))  { if (et >= 0) dashSetze(dash.light[et], val); }
  else if (!strcmp(cmd, "ext_light"))  dashSetze(dash.ext_light,  val);
  else if (!strcmp(cmd, "door_light")) dashSetze(dash.door_light, val);
  else if (!strcmp(cmd, "garden_light")) dashSetze(dash.garden_light, val);
  else if (!strcmp(cmd, "party"))      dashSetze(dash.party,      val);
  else if (!strcmp(cmd, "whirlpool"))  dashSetze(dash.whirlpool,  val);
  else if (!strcmp(cmd, "ac"))     { if (et >= 0) dashSetze(dash.ac[et], val); }
  else if (!strcmp(cmd, "skylight2"))  dashSetze(dash.skylight2,  val);
  else if (!strcmp(cmd, "skylight1"))  dashSetze(dash.skylight1,  val);
  else if (!strcmp(cmd, "tv"))         dashSetze(dash.tv,         val);
  else if (!strcmp(cmd, "garage"))     dashSetze(dash.garage,     val);
  else if (!strcmp(cmd, "elevator"))   dashSetze(dash.elevator,   val);   // value = Zieletage 0/1/2 (Fahrt-Anbindung folgt)
  else if (!strcmp(cmd, "hand"))  freezeSetze(dash.autostop, val != 0);  // Automatik einfrieren/freigeben
}