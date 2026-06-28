# SPE Smarthome – ESP32-Steuerung

Steuerung & Regelung eines Playmobil-Smarthomes (Ausbildungsprojekt SPE/Siemens).
Ein **ESP32 WROOM-32U** steuert Klima (Heizen/Kühlen), Beleuchtung, Beschattung,
Dachfenster, Disco, einen Schrittmotor-Aufzug und weitere Aktoren. Bedient wird
**lokal über Taster** und über ein **Dashboard auf einem Raspberry Pi**
(serielle Verbindung, JSON-Zeilen).

> Stand: laufende Entwicklung (Branch `RefactorWeek3`). Diese README beschreibt den
> **tatsächlichen aktuellen Code-Stand**; offene Punkte stehen in Abschnitt 8.

---

## 1. Hardware / Pin-Belegung

Alle Pins/Adressen/Parameter stehen zentral in [`include/Config.h`](include/Config.h).

**I2C-Bus** (SDA=21, SCL=22), gemeinsam genutzt:
| Baustein | Adresse | Rolle |
|----------|---------|-------|
| MCP23017 #1 | `0x20` | **Eingänge** (alle `INPUT_PULLUP`): Taster, Reeds |
| MCP23017 #2 | `0x24` | **Ausgänge** (Transistoren, active-high): Heiz-/Kühl-LEDs |
| PCA9685 | `0x40` | **Servos** (Jalousien, Dachfenster, Garage) |

**ESP32-native Pins:**
| Funktion | Pin(s) |
|----------|--------|
| Licht (6 PWM-MOSFET-Kanäle) | K0=2 Außen, K1=4 Tür, K2=5 EG, K3=18 OG1, K4=19 OG2, K5=23 Reserve |
| Aufzug-Stepper 28BYJ-48 (IN1-4) | 25 / 26 / 27 / 14 |
| DHT11 (Temp/Feuchte) | 13 |
| Solarpanel (= zugleich Helligkeit), 50/50-Teiler an ADC | 36 (Sensor VP) |
| Gewächshaus-Wassersensor (Bodenfeuchte, analog ADC1) | 34 |
| DiscoLight WS2813 (FastLED) | 0 |
| UART2 → Raspberry Pi (RX/TX) | 16 / 17 |

**MCP-Eingänge (#1):** Klima-Taster EG/OG1/OG2 = 0/1/2 · Aufzug-Ruftaster = 3/4/5 ·
Aufzug-Etagen-Reeds = 6/7/8 · oberer Überfahr-Schalter = 9 · Reed Tür = 10.
**MCP-Ausgänge (#2):** Heiz-LED EG/OG1/OG2 = 0/1/2 · Kühl-LED EG/OG1/OG2 = 3/4/5.

> Konvention: Eingänge `INPUT_PULLUP`, aktiv/geschlossen = LOW. Ausgänge active-high
> (HIGH = Transistor leitet). Aktuell ist im Test nur **ein** MCP (Ausgänge, `0x24`)
> angeschlossen; für den vollen Betrieb wird der Eingangs-MCP (`0x20`) benötigt.

---

## 2. Software-Architektur

**Trennung „Entscheiden ↔ Ausführen" über einen zentralen Soll-Zustand** (Blackboard).
Jede `loop()`:

```
Eingänge + Sensoren ─► Kontext
        │
        ▼   Regel-Schichten schreiben in den Soll (setze(feld, wert, PRIO) – höchste Prio gewinnt)
   tageszeitRegeln (20/60)  ·  sensorRegeln (40)  ·  handRegeln (100)  ·  dashboardRegeln (80)
        │
        ▼   interlocks(soll)   – Konflikte prioritätsbewusst auflösen
        ▼   anwenden(soll)     – EINZIGE Hardware-Stelle (Module ansteuern)
        ▼   aufzugBedienen()   – eigenes Zustands-Subsystem (nicht über Soll)
        ▼   telemetrie(k, s)   – Frame an den Pi (nur bei Änderung + Heartbeat)
```

**Leitprinzipien:** Module in `lib/` sind *dumme Treiber* (kein eigenes „Wissen"),
die Logik steht in `Regelung` + `main`. Konfiguration zentral in `Config.h`.
Kooperativ & nicht-blockierend (`millis()`, **kein `delay()`** in der Loop).
Jedes Modul besitzt seinen Chip selbst (DigitalInput → MCP-IN, DigitaleOutputs →
MCP-OUT, Servoaktor → PCA9685); `Wire.begin()` einmal in `setup()`.

**Automatik-Stopp (Freeze):** Dashboard-Kommando `auto_stop` friert Tageszeit+Sensor
beim letzten Stand ein (Snapshot), Taster + Dashboard bleiben aktiv. TTL 5 min
(`FREEZE_TTL_MS`), danach automatisch zurück in Automatik.

---

## 3. Module (`lib/`) – wer macht was

| Modul | Aufgabe |
|-------|---------|
| `Regelung` | Soll-Zustand + Schicht-Funktionen + Interlocks + Kontext + TasterState/Freeze |
| `DigitalInput` | besitzt MCP-IN; entprellt + Flankenerkennung (`gedrueckt`/`geradeGedrueckt`) |
| `DigitaleOutputs` | besitzt MCP-OUT; `heizen(Etage,an)` / `kuehlen(Etage,an)` |
| `Servoaktor` | PCA9685; `fahreJalousie/fahreDachfenster/fahreGarage` (sanft, `servosUpdate()`) |
| `Aufzug` | 28BYJ-48 / ULN2003 (IN1-4, Halbschritt); `fahreZu` / `update(reeds, oben)` |
| `Licht` | 6 PWM-Kanäle (IRLZ44N-MOSFET), `setKanal(kanal, stufe 0..3)` |
| `DiscoLight` | WS2813 / FastLED, nicht-blockierende Animation |
| `Sensorik` | 1× DHT11 (Temp/Feuchte) + Solar (= Helligkeit) + Wassersensor (Gewächshaus) |
| `Kommunikation` | JSON-Telemetrie + Befehlsempfang (UART2) |

---

## 4. Klima-Logik (pro Etage)

**Etagentaster (Hand):** rotiert lokal 3 Modi pro Etage — **Automatik / Heizen /
Kühlen** (`TasterState`, mit TTL). `handRegeln` macht daraus die Ausgänge:
Heizen → rote LED, Kühlen → blaue LED. Der Modus ist nur ein *Helfer für die
Tastenbedienung* (ein Knopf rotiert die Modi) und wird **nicht** in der Telemetrie
übertragen.

**Dashboard:** steuert `heat` und `ac` (Kühlung) **je Etage direkt** — rote bzw.
blaue LED pro Etage (genau wie der Taster, nur ohne Modus-Rotation).

**Zentrale Klimaanlage:** läuft, sobald **irgendeine** Etage kühlt
(`klimaanlage = ODER(kühlen)`). Das schaltet der **ESP32 selbst** (in `interlocks()`);
das Dashboard greift **nicht** direkt auf die AC zu. **OG2-Dachfenster** öffnen
**nur**, wenn die **OG2** kühlt (AC durch eine andere Etage ⇒ Dachfenster bleiben zu).

---

## 5. Dashboard / Telemetrie (UART2, JSON pro Zeile)

- **ESP → Pi:** Telemetrie-Frame (`type:"telemetry"`) mit `time` (10-Min-quantisiert),
  `temp`, `humidity`, `auto_active`, `greenhouse.soil_moisture`, `outdoor`,
  `roof` (inkl. `pv_voltage` + zentrale `ac`), `floors` (je Etage `heat`/`cool`/`light`/
  `blind1`/`blind2`) und `elevator.floor`. **Kein `mode`** mehr (Heiz-/Kühlzustand
  steht je Etage in `heat`/`cool`). Gesendet nur bei Änderung + 5-s-Heartbeat.
- **Pi → ESP:** Befehle `{"cmd":"...","value":..,"floor":".."}` → `behandleBefehl()`
  in `Kommunikation.cpp` (Overrides mit TTL in `DashboardState`).
- Schema-Referenz: `src/dashboardConnections.cpp` (frühe Skizze, dient als
  Protokoll-Dokumentation; Modi-Konvention etc.).

---

## 6. Bauen, flashen, testen

```bash
PIP_USER=0 pio run            # kompilieren  (PIP_USER=0 ist auf diesem Rechner nötig!)
PIP_USER=0 pio run -t upload  # flashen
pio device monitor            # serielle Ausgabe (USB, 115200)
```

**Welches Programm gebaut wird, steuert `build_src_filter` in `platformio.ini`:**
- `+<main.cpp>` → das echte Hauptprogramm
- `+<aufzugTest.cpp>` / `+<mcpTest.cpp>` → Bring-up-Tests (Aufzug bzw. 2×MCP+PCA)
- weitere Test-Sketches: `discoLightTest`, `ledDriver`, `integrationTest`

---

## 7. Aktueller Stand je Subsystem

| Subsystem | Stand |
|-----------|-------|
| Klima Heizen/Kühlen pro Etage (Taster) | ✅ funktioniert (Hand-Modus „hart an") |
| Licht (6 Kanäle, Tageszeit) | ✅ funktioniert |
| Disco (OG2, Disco↔Licht-Interlock) | ✅ funktioniert |
| Jalousien / Dachfenster / Garage (Servos) | ✅ Grundfunktion (Layout siehe §8) |
| Aufzug (Stepper, Ruftaster, Reeds, oberer Schalter, Richtungswechsel) | ✅ funktioniert |
| Sensorik DHT11 (Temp/Feuchte) + Solar/Helligkeit | ✅ eingebunden |
| Gewächshaus-Wassersensor (Bodenfeuchte) | ✅ eingebunden |
| Dashboard Telemetrie + Befehle + Freeze | ✅ funktioniert |
| Sensor-Regeln (`sensorRegeln`) | 🟡 leer – Logik folgt |
| Klimaanlage (= ODER Kühlen) / OG2-Dachfenster bei OG2-Kühlen | ✅ funktioniert |
| Dashboard: `heat` + `ac` (Kühlung) pro Etage direkt | ✅ funktioniert |
| Garage-Ultraschall, Whirlpool, TV-Display | 🔲 geplant |

---

## 8. Offene Punkte / nächste Schritte (Finalisierung)

**Klima & Lüftung**
- [x] Dashboard steuert `heat` **und** `ac` (Kühlung) **pro Etage direkt** (rote/blaue LED) – kein Klima-Modus übers Dashboard.
- [x] `klimaanlage = ODER(kühlen[Etage])`; **OG2-Dachfenster nur bei `kühlen[OG2]`** (ESP leitet die zentrale AC selbst ab).
- [x] Interlock entsprechend umgesetzt (`interlocks()`).

**Aufräumen (bestätigte Relikte)**
- [x] `klappe[]` aus dem Soll entfernt (keine Klappen verbaut – nur zentrale AC).
- [x] `bewegung` aus dem Kontext entfernt (kein PIR; Helligkeit kommt vom Solar).
- [x] `dash.mode`-Empfang entfernt (nicht genutzt).
- [x] Telemetrie `skylight1`+`skylight2` = **beide OG2** (kein EG-Dachfenster).

**Neue Features**
- [ ] **Garage:** Ultraschallsensor → Tor (Servo) auf + Licht (gleichgeschaltet mit **Türlicht**, Kanal K1). Pins/Schwelle in Config.
- [ ] **Whirlpool:** DC-Motor über PWM + MOSFET (eigener Kanal/Pin + Soll-Feld).
- [ ] **Fernseher:** I2C-Display (am gemeinsamen I2C-Bus).
- [ ] **Jalousien-Layout:** EG = **1**, OG1 = **2**, OG2 = **2** (gesamt 5 Servos) – Soll/Telemetrie anpassen (EG nur `blind1`).

**Telemetrie vervollständigen**
- [x] `floors[..].cool` ergänzt (blaue LED je Etage); `mode` **entfernt** (nicht mehr nötig).
- [x] `elevator.floor` aus `aufzug.aktuelleEtage()` gefüllt.
- [x] `greenhouse.soil_moisture` aus dem Wassersensor (0=trocken…100=nass).
- [ ] `whirlpool` / `tv` / `garage` an reale Quellen koppeln (sobald Aktoren da).

**Sensorik/Regelung**
- [x] `k.helligkeit` aus dem Solar-Wert abgeleitet.
- [x] Gewächshaus-Wassersensor (analog, GPIO34) → `k.bodenfeuchte` (kalibrierbar in Config).
- [ ] `sensorRegeln` füllen (Hysterese, Beschattung, Licht) – nutzt `temperatur`/`helligkeit`.

**Aufzug-Betriebssicherheit**
- [ ] `FEHLER` quittierbar (Dashboard/Taster) + Referenzfahrt nach EG beim Start/nach Fehler.

**Hardware/Config**
- `ADDR_MCP_OUT` bleibt auf **`0x24`** (keine Umstellung auf `0x21` geplant).
- [ ] Wassersensor-Rohwerte einmessen (`SENSORIK_WASSER_TROCKEN`/`_NASS` in `Config.h`).
- [ ] Servo-Endlagen je Servo einmessen; Aufzug-Schritttakt feinjustieren.

---

## 9. Zusammenarbeit (GitHub)
Entwicklung in Feature-Branches, Merge per Pull Request; `Config.h`/`main.cpp` werden
von beiden angefasst → kleine, klar getrennte Blöcke. Vor dem Push einmal
`PIP_USER=0 pio run` (baut es?).
