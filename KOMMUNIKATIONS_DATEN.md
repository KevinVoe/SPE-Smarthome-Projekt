# Kommunikations-Protokoll  ESP32 ⇄ Raspberry Pi

Diese Datei beschreibt **exakt**, welche Daten über die serielle Verbindung laufen –
für die Raspberry-Pi-Seite (Dashboard). Stand: entspricht dem aktuellen Firmware-Code
(`lib/Kommunikation/Kommunikation.cpp`, `lib/Regelung/Regelung.cpp`).

---

## 1. Physische Verbindung

| | |
|---|---|
| Schnittstelle | UART (TTL, **3,3 V** – kein 5 V an den Pi!) |
| Baudrate | **115200**, 8N1 |
| ESP32 | UART2: **TX = GPIO17**, **RX = GPIO16** |
| Verkabelung | ESP **TX17 → Pi RX** · ESP **RX16 ← Pi TX** · **GND ↔ GND** |
| Pi-Port | z. B. `/dev/serial0` (UART-Header) oder `/dev/ttyUSB0` (USB-Adapter) |

## 2. Rahmen-Format (für beide Richtungen)

- **Eine JSON-Nachricht pro Zeile**, abgeschlossen mit `\n` (LF).
- `\r` wird ignoriert, Leerzeilen werden ignoriert.
- Ungültiges JSON wird **still verworfen** (keine Fehlermeldung zurück).
- Max. Zeilenlänge beim Empfang am ESP: **200 Zeichen** (länger ⇒ Zeile verworfen).
- Zeichensatz: ASCII / UTF-8.

---

## 3. ESP32 → Pi : **Telemetrie** (Senden)

### 3.1 Wann wird gesendet?
- **Bei jeder Änderung** des Frames (sofort), und
- als **Heartbeat spätestens alle 5000 ms** (`TELEMETRIE_MAX_INTERVALL_MS`), auch
  wenn sich nichts geändert hat (so erkennt der Pi, dass die Verbindung lebt).
- Es gibt also kein festes Sende-Intervall; der Pi sollte **ereignisbasiert** lesen.

### 3.2 Vollständiges Beispiel
```json
{
  "type": "telemetry",
  "time": 13.5,
  "temp": 22.5,
  "humidity": 48.0,
  "auto_active": true,
  "outdoor": {
    "ext_light": 0,
    "door_light": 0,
    "whirlpool": 0,
    "garage": 0,
    "garden_light": 0,
    "humid_garden": 42.0
  },
  "roof": {
    "pv_voltage": 3.6,
    "skylight1": 0,
    "skylight2": 0,
    "ac": 0
  },
  "floors": {
    "EG": { "blind1": 1, "heat": 0, "cool": 0, "light": 0 },
    "E1": { "blind1": 1, "blind2": 1, "heat": 0, "cool": 0, "light": 0, "tv": 0 },
    "E2": { "blind1": 1, "blind2": 1, "heat": 0, "cool": 0, "light": 0, "party": 0 }
  },
  "elevator": { "floor": 0 }
}
```

### 3.3 Feld-Referenz (Typen & Wertebereiche)

| Feld | Typ | Wertebereich | Bedeutung |
|------|-----|--------------|-----------|
| `type` | string | `"telemetry"` | Frame-Kennung (immer gesetzt) |
| `time` | float | `0.0 … 24.0` | simulierte Uhrzeit (Stunden), Raster **10 Sim-Minuten** (0,1666…) |
| `temp` | float | °C | Temperatur (ein DHT-Sensor fürs ganze Haus) |
| `humidity` | float | `0 … 100` | rel. Luftfeuchte in % (DHT) |
| `auto_active` | bool | `true`/`false` | `true` = Automatik läuft, `false` = **eingefroren** (Hand/Autostop), s. §5 |
| **`outdoor`** | object | | Außenbereich |
| `outdoor.ext_light` | int | `0 … 3` | Außenlicht-Dimmstufe (Licht-Kanal K0) |
| `outdoor.door_light` | int | `0 … 3` | Türlicht-Dimmstufe (K1) |
| `outdoor.whirlpool` | int | `0 / 1` | Whirlpool an/aus |
| `outdoor.garage` | int | `0 / 1` | **Platzhalter, derzeit immer 0** (noch keine Quelle) |
| `outdoor.garden_light` | int | `0 … 3` | Gewächshaus-Licht (Kanal K5) |
| `outdoor.humid_garden` | float | `0 … 100` | Gewächshaus-Bodenfeuchte in % (0 = trocken, 100 = nass) |
| **`roof`** | object | | Dach |
| `roof.pv_voltage` | float | Volt | Solarpanel-Spannung (dient zugleich als Helligkeit) |
| `roof.skylight1` | int | `0 / 1` | OG2-Dachfenster (1 = offen) |
| `roof.skylight2` | int | `0 / 1` | OG2-Dachfenster (1 = offen) – **gleicher Wert wie skylight1**, beide Fenster im OG2 |
| `roof.ac` | int | `0 / 1` | zentrale Klimaanlage an/aus (intern PWM-Duty, hier nur an/aus) |
| **`floors`** | object | | je Etage `EG` / `E1` / `E2` (= EG / OG1 / OG2) |
| `floors.*.blind1` | int | `0 / 1` | Jalousie 1: **1 = oben/offen, 0 = unten/zu** |
| `floors.*.blind2` | int | `0 / 1` | Jalousie 2 – **nur E1 und E2** (EG hat nur `blind1`) |
| `floors.*.heat` | int | `0 / 1` | Heizung der Etage (rote LED) |
| `floors.*.cool` | int | `0 / 1` | Kühlung der Etage (blaue LED) |
| `floors.*.light` | int | `0 … 3` | Raumlicht-Dimmstufe der Etage |
| `floors.E1.tv` | int | `0 / 1` | **Platzhalter, derzeit immer 0** (nur E1) |
| `floors.E2.party` | int | `0 / 1` | Disco/Partylicht im OG2 (nur E2) |
| **`elevator`** | object | | Aufzug |
| `elevator.floor` | int | `0 / 1 / 2` | aktuelle Etage (0 = EG, 1 = OG1, 2 = OG2) |

> **Hinweis:** Es gibt **kein** `mode`-Feld mehr. Heiz-/Kühlzustand steht je Etage
> direkt in `heat` / `cool`.

---

## 4. Pi → ESP32 : **Befehle** (Empfang)

### 4.1 Format
Eine JSON-Zeile pro Befehl:
```json
{"cmd":"<name>","value":<zahl>,"floor":"<EG|E1|E2>"}
```
| Schlüssel | Typ | Pflicht | Bedeutung |
|-----------|-----|---------|-----------|
| `cmd` | string | **ja** | Befehlsname (s. Tabelle). Unbekannt ⇒ ignoriert |
| `value` | int | nein (Default `0`) | Zielwert (Bedeutung je Befehl) |
| `floor` | string | nur bei Etagen-Befehlen | `"EG"`, `"E1"` oder `"E2"`. Fehlt/ungültig ⇒ Etagen-Befehl wird **ignoriert** |

### 4.2 Etagen-Befehle (brauchen `floor`)

| `cmd` | `value` | Wirkung (Sollwert) |
|-------|---------|--------------------|
| `blind1` | `1` = auf/oben, `0` = zu/unten | Jalousie 1 der Etage |
| `blind2` | `1` / `0` | Jalousie 2 der Etage (**nur E1/E2**) |
| `heat` | `0` / `1` | Heizung der Etage (rote LED) |
| `ac` | `0` / `1` | **Kühlung der Etage** (blaue LED). Die zentrale AC schaltet der ESP32 selbst (Duty nach Anzahl kühlender Etagen) |
| `light` | `0 … 3` | Raumlicht-Dimmstufe der Etage |

### 4.3 Globale Befehle (kein `floor`)

| `cmd` | `value` | Wirkung |
|-------|---------|---------|
| `ext_light` | `0 … 3` | Außenlicht (K0) |
| `door_light` | `0 … 3` | Türlicht (K1) |
| `garden_light` | `0 … 3` | Gewächshaus-Licht (K5) |
| `party` | `0 / 1` | Disco/Partylicht OG2 |
| `whirlpool` | `0 / 1` | Whirlpool an/aus |
| `skylight1` | `0 / 1` | OG2-Dachfenster (1 = auf) |
| `skylight2` | `0 / 1` | OG2-Dachfenster (1 = auf) – wirkt aufs selbe Fenster wie skylight1 |
| `hand` | `0 / 1` | **Autostop / Automatik einfrieren**, s. §5 |

### 4.4 (Noch) ohne Wirkung
Diese Befehle werden **angenommen** (kein Fehler), haben aber derzeit **keinen
Aktor / kein Soll-Feld**, d. h. sie bewirken nichts:
`tv`, `garage`, `front_door`, `elevator`.
(Der Aufzug fährt aktuell nur über die physischen Ruftaster am Haus.)

### 4.5 Beispiele
```json
{"cmd":"light","value":2,"floor":"E1"}
{"cmd":"heat","value":1,"floor":"EG"}
{"cmd":"ac","value":1,"floor":"E2"}
{"cmd":"garden_light","value":3}
{"cmd":"whirlpool","value":1}
{"cmd":"hand","value":1}
```

---

## 5. Autostop / „hand" — genau erklärt

**Es ist KEIN Toggle.** Der Befehl `hand` **setzt einen Wert**:

- `{"cmd":"hand","value":1}` (oder jeder Wert ≠ 0) → Automatik wird **eingefroren**.
- `{"cmd":"hand","value":0}` → Automatik wird **sofort wieder freigegeben**.

Verhalten im ESP (`freezeSetze` / `automatikEingefroren`):
- Beim Einfrieren wird der **letzte Automatik-Stand gehalten** (Tageszeit- und
  Sensorregeln werden „eingefroren") – er wird **nicht auf 0 gesetzt**.
- **Hand-Taster am Haus und Dashboard-Befehle bleiben aktiv** und können im
  eingefrorenen Zustand weiter steuern.
- Der Freeze hat eine eigene, lange TTL von **5 Minuten** (`FREEZE_TTL_MS`):
  Danach taut die Automatik **automatisch** wieder auf. Soll der Freeze länger
  halten, muss der Pi den Befehl vor Ablauf erneut senden.
- Im Telemetrie-Frame ist `auto_active = false`, solange eingefroren.

> **Wichtig (war vermutlich der Grund, warum es bisher nicht ging):** Der
> Befehlsname ist exakt **`hand`** – nicht `auto_stop`/`autostop`. Und es muss ein
> `value` mitgeschickt werden (`1` = einfrieren, `0` = freigeben).

---

## 6. Prioritäten & Gültigkeitsdauer (wichtig fürs Dashboard!)

Der ESP baut **jede Loop** einen frischen Soll-Zustand. Mehrere Quellen schreiben
mit einer **Priorität**; die höchste gewinnt pro Feld:

| Quelle | Priorität |
|--------|-----------|
| Tageszeit-Automatik | 20 |
| Sensor-Regeln | 40 |
| **Dashboard-Befehle** | **80** |
| Hand-Taster am Haus | 100 |

⇒ **Alle `cmd`-Befehle vom Pi setzen den Sollwert mit Priorität 80.** Sie schlagen
damit Tageszeit + Sensorik, werden aber vom Hand-Taster am Haus (100) übersteuert.

### Befristung der Befehle (TTL)
- Ein normaler Dashboard-Befehl ist **nur `DASHBOARD_TTL_MS` lang aktiv** und fällt
  danach zurück auf die Automatik. **Aktueller Wert im Code: `3000` ms (= 3 s).**
  → Um einen Zustand dauerhaft zu halten, muss der Pi den Befehl **periodisch
  wiederholen** (z. B. alle 1–2 s).
- Ausnahme: `hand` (Autostop) hat seine eigene TTL von **5 min** (s. §5).

---

## 7. Verifikation der `dash.*` → `s` Zuordnung (Stand: geprüft)

Alle Empfangsbefehle setzen den korrekten Sollwert mit **Prio 80**:

| `cmd` | DashboardState-Feld | gesetzter Sollwert |
|-------|---------------------|--------------------|
| `blind1/2` | `dash.blind[etage][0/1]` | `s.jalousie[etage][0/1]` |
| `heat` | `dash.heat[etage]` | `s.heizung[etage]` |
| `ac` | `dash.ac[etage]` | `s.kuehlLed[etage]` |
| `light` | `dash.light[etage]` | `s.licht[etage+2]` |
| `ext_light` | `dash.ext_light` | `s.licht[0]` |
| `door_light` | `dash.door_light` | `s.licht[1]` |
| `garden_light` | `dash.garden_light` | `s.licht[5]` |
| `party` | `dash.party` | `s.disco` |
| `whirlpool` | `dash.whirlpool` | `s.whirlpool` |
| `skylight1` | `dash.skylight1` | `s.dachfensterOG2` |
| `skylight2` | `dash.skylight2` | `s.dachfensterOG2` |
| `hand` | `dash.autostop` | Freeze-Flag (s. §5) |

Korrigiert in diesem Stand: `garden_light` schrieb fälschlich in `dash.light[5]`
(Array-Überlauf, Größe 3) und wurde nie angewandt → nutzt jetzt `dash.garden_light`
→ `s.licht[5]`. `skylight1` wurde empfangen, aber nie angewandt → wirkt jetzt
(wie `skylight2`) auf das OG2-Dachfenster.
