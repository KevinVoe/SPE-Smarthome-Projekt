/*
 * ESP32 -> Raspberry Pi Dashboard  (Gebaeude-Digital-Twin)
 *
 * Sendet 1x/s einen Telemetrie-Frame als EINE JSON-Zeile ueber UART (115200 Baud).
 * Das Python-Backend liest diesen Datenstrom live ein (keine Datei noetig).
 *
 * Werte / Konventionen:
 *   Bedienfeld mode: 0 Auto | 1 Heizung an,Klima aus | 2 Klima an,Heizung aus
 *                    | 3 Alles aus | 4 Dachfenster auf (nur EG/E2)
 *   Jalousie blind1/2 : 1 oben  / 0 unten
 *   Heizung  heat1/2  : 1 an    / 0 aus
 *   Beleuchtung light : 0 aus / 1 / 2 / 3 (Dimmstufe)
 *   Dachfenster sky1/2: 1 offen / 0 zu
 *   Klima ac          : 1 an / 0 aus
 *   Party (nur E1)    : 1 ja / 0 nein
 *   Fernseher tv      : 1 an / 0 aus
 *   Aufzug floor      : 0 EG / 1 E1 / 2 E2
 *   PV pv_voltage     : Solarspannung in Volt (float)
 *
 * Verkabelung Pi <-> ESP32 (3.3V! kein 5V an Pi-RX):
 *   ESP32 TX (GPIO17) -> Pi RX (GPIO15 / Pin 10)
 *   ESP32 RX (GPIO16) <- Pi TX (GPIO14 / Pin 8)
 *   GND <-> GND
 * Auf dem Pi: ESP_PORT=/dev/serial0  (UART-Header)  oder /dev/ttyUSB0 (USB).
 *
 * Bibliothek: ArduinoJson v7 (im Library Manager installieren / lib_deps).
 *   Hinweis: API hat sich gegenueber v6 geaendert (JsonDocument statt
 *   StaticJsonDocument, to<JsonObject>() statt createNestedObject()).
 */
#include <Arduino.h>
#include <ArduinoJson.h>

// ── Sensor-/Aktor-Platzhalter ─────────────────────────────────────────────────
//  (echte Implementierung folgt, sobald die Hardware-Module feststehen)
void sendTelemetry();
float readPvVoltage()               { return 3.6f; }
int   readTV()                      { return 0; }
int   readSky1()                    { return 0; }
int   readSky2()                    { return 0; }
int   readAC()                      { return 0; }
int   readElevator()                { return 1; }   // 0/1/2
int   readParty()                   { return 0; }
int   readMode(const char* floor)   { return 0; }   // 0..4
int   readBlind(const char* f, int n) { return 1; } // 1 oben / 0 unten
int   readHeat(const char* f, int n)  { return 0; } // 0/1
int   readLight(const char* floor)  { return 0; }   // 0..3

// ── UART zum Raspberry Pi ─────────────────────────────────────────────────────
HardwareSerial Link(2);   // UART2

uint32_t lastSend = 0;
constexpr uint32_t SEND_INTERVALL_MS = 1000;

void setup() {
  Serial.begin(115200);                                  // USB-Debug
  Link.begin(115200, SERIAL_8N1, 16, 17);                // RX=16, TX=17
  // pinMode(PIN_EG_BLIND1, INPUT_PULLUP); ...            // Sensorpins konfigurieren
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVALL_MS) {
    lastSend = millis();
    sendTelemetry();
  }
}

// ── Hilfsfunktion: ein Etagen-Objekt fuellen ─────────────────────────────────
void fillFloor(JsonObject f, int mode, int b1, int b2, int h1, int h2, int light) {
  f["mode"]   = mode;
  f["blind1"] = b1;
  f["blind2"] = b2;
  f["heat1"]  = h1;
  f["heat2"]  = h2;
  f["light"]  = light;
}

void sendTelemetry() {
  // ArduinoJson v7: JsonDocument ersetzt StaticJsonDocument/DynamicJsonDocument
  // und waechst dynamisch im Heap - keine Kapazitaet mehr angeben.
  JsonDocument doc;
  doc["type"] = "telemetry";

  doc["tv"] = readTV();                     // 0/1

  JsonObject roof = doc["roof"].to<JsonObject>();
  roof["pv_voltage"] = readPvVoltage();     // float (V)
  roof["skylight1"]  = readSky1();          // 0/1
  roof["skylight2"]  = readSky2();          // 0/1
  roof["ac"]         = readAC();            // 0/1

  JsonObject floors = doc["floors"].to<JsonObject>();

  fillFloor(floors["EG"].to<JsonObject>(),
            readMode("EG"), readBlind("EG", 1), readBlind("EG", 2),
            readHeat("EG", 1), readHeat("EG", 2), readLight("EG"));

  JsonObject e1 = floors["E1"].to<JsonObject>();
  fillFloor(e1, readMode("E1"), readBlind("E1", 1), readBlind("E1", 2),
            readHeat("E1", 1), readHeat("E1", 2), readLight("E1"));
  e1["party"] = readParty();                // nur E1, 0/1

  fillFloor(floors["E2"].to<JsonObject>(),
            readMode("E2"), readBlind("E2", 1), readBlind("E2", 2),
            readHeat("E2", 1), readHeat("E2", 2), readLight("E2"));

  doc["elevator"]["floor"] = readElevator(); // 0/1/2

  // Overflow-Check statt des in v7 entfernten capacity()-Checks
  if (doc.overflowed()) {
    Serial.println("WARN: JsonDocument overflowed - Telemetrie unvollstaendig!");
  }

  serializeJson(doc, Link);
  Link.print('\n');     // wichtig: Zeilenende
}