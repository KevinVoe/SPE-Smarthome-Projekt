# SPE Smarthome – ESP32-Steuerung

Steuerung & Regelung eines Playmobil-Smarthomes (Ausbildungsprojekt SPE/Siemens).
Ein **ESP32 WROOM-32U** steuert Heizung, Beleuchtung, Beschattung/Dachfenster,
einen Aufzug und die Sicherheitstechnik. Bedient wird lokal über Taster **und**
über ein Dashboard auf einem **Raspberry Pi** (serielle Verbindung, JSON).

> Dieser Teil des Projekts (Elektronik + Steuerungs-/Regelungssoftware) wird von
> einem 2er-Team über GitHub gemeinsam entwickelt. Dashboard und Konstruktion
> machen andere Gruppen.

---

## 1. Architektur auf einen Blick

```
                 ┌─────────────────────────────────────────────┐
   Raspberry Pi  │                  ESP32                       │
   (Dashboard) ──┤ Serial2/UART  ── Kommunikation (JSON-Zeilen) │
                 │                        │                     │
                 │                     main.cpp                 │
                 │           (loop + REGELUNGSLOGIK)            │
                 │      ┌─────────────────┼───────────────┐     │
                 │   Module ............................. Module │
                 │   Heizung  Aufzug  Licht  Servoaktor  Alarm  │
                 │      │        │       │        │         │    │
                 │   ┌──┴────────┴───────┴────────┴─────────┴─┐  │
                 │   │            Io-Schicht                   │  │
                 │   │  einheitlich: ESP32-GPIO ODER MCP23017  │  │
                 │   └──────────────┬─────────────┬───────────┘  │
                 └──────────────────┼─────────────┼──────────────┘
                                 ESP32-GPIOs   I2C-Bus
                                              (MCP23017, PCA9685,
                                               BH1750, SHT31)
```

**Leitprinzipien**

- **Modular:** Jedes Steuerungselement ist ein eigenständiges Modul in
  `lib/<Modul>/`. Module kennen sich gegenseitig nicht – nur `main` verknüpft sie.
  → Zwei Leute können parallel an verschiedenen Modulen arbeiten, ohne sich in die
  Quere zu kommen.
- **Konfiguration zentral:** Alle Pins/Adressen/Parameter stehen ausschließlich in
  [`include/Config.h`](include/Config.h).
- **Hardware-Abstraktion (Io):** Ein Modul fragt nie direkt einen GPIO ab, sondern
  immer `io.digitalRead/Write(...)`. Ob der Pin am ESP32 oder am Port-Expander
  **MCP23017** sitzt, entscheidet allein die Config.
- **Kooperativ & nicht-blockierend:** Ein `loop()`, jedes Modul hat ein
  `update()` mit eigenem Timing über `millis()`. **Kein `delay()`** – sonst
  blockiert ein Modul alle anderen.

---

## 2. Ordnerstruktur

```
SPE-Smarthome-Projekt/
├─ include/
│  └─ Config.h            ← ZENTRALE Konfiguration (Pins, Adressen, Parameter)
├─ lib/
│  ├─ Io/                 ← Pin-Abstraktion ESP32 + MCP23017  (Basis)
│  ├─ Heizung/            ← Etagenheizung: Taster + Hysterese (Vorlage-Modul)
│  ├─ Aufzug/             ← Brushed-DC + Seilwinde, Zustandsautomat
│  ├─ Licht/              ← NeoPixel-Raumlicht
│  ├─ DiscoLight/         ← NeoPixel-Stimmungslicht (Animation)
│  ├─ Servoaktor/         ← Servos am PCA9685 (Dachfenster/Jalousie/Garage)
│  ├─ Alarm/              ← Alarmrelais + Buzzer
│  ├─ Sensorik/           ← Temperatur/Helligkeit/Feuchte (aktuell simuliert)
│  └─ Kommunikation/      ← JSON-Link zum Raspberry Pi
├─ src/
│  └─ main.cpp            ← Objekte + setup()/loop() + REGELUNGSLOGIK
└─ platformio.ini
```

> **Wichtig:** In PlatformIO muss jedes Modul einen **eigenen Unterordner** in
> `lib/` haben. Lose `.cpp`-Dateien direkt in `lib/` werden **nicht** kompiliert.

---

## 3. Konfiguration & der MCP23017

Da sehr viele GPIOs gebraucht werden, kommt der I2C-Port-Expander **MCP23017**
(16 zusätzliche Pins) zum Einsatz. Damit Module nicht wissen müssen, wo ein Pin
sitzt, gibt es die **Io-Schicht** und den Typ `IoPin`:

```cpp
// in include/Config.h:
constexpr IoPin HEIZUNG_EG_TASTER = mcpPin(0);   // Pin 0 am MCP23017
constexpr IoPin AUFZUG_MOTOR_ENABLE = espPin(26); // GPIO 26 direkt am ESP32
```

```cpp
// im Modul (egal ob ESP- oder MCP-Pin):
io.pinMode(_tasterPin, INPUT_PULLUP);
int x = io.digitalRead(_tasterPin);
```

**Pin umstecken = nur eine Zeile in `Config.h` ändern.** Aktuelle Belegung
(Auszug, Stand Gerüst – bei realem Aufbau anpassen):

| Funktion                     | Ort        | Pin/Kanal |
|------------------------------|------------|-----------|
| I2C SDA / SCL                | ESP32      | 21 / 22   |
| Link zum Pi (RX/TX, UART2)   | ESP32      | 16 / 17   |
| DS18B20 (1-Wire)             | ESP32      | 4         |
| Aufzug Endschalter oben/unten| ESP32      | 32 / 33   |
| Aufzug Motor Richtung/Enable | ESP32      | 25 / 26   |
| NeoPixel Licht / Disco       | ESP32      | 5 / 18    |
| Heizung Taster EG/OG1/OG2    | MCP23017   | 0 / 1 / 2 |
| Heizung LED EG/OG1/OG2       | MCP23017   | 8 / 9 /10 |
| PIR Wohnzimmer / Schlafz.    | MCP23017   | 3 / 4     |
| Reed Tür EG / Fenster OG     | MCP23017   | 5 / 6     |
| Alarmrelais / Buzzer         | MCP23017   | 11 / 12   |
| Servos (Dachfenster/Jal./Tor)| PCA9685    | 0–4       |

> **Konvention:** Taster/Endschalter als `INPUT_PULLUP` gegen GND verdrahten
> (gedrückt = LOW). Reed/PIR im Gerüst als „offen/aktiv = HIGH“ angenommen –
> bei der realen Verdrahtung in `main.cpp` (`regelung()`) ggf. invertieren.

---

## 4. Serielles Protokoll (ESP32 ↔ Raspberry Pi)

Eine **JSON-Nachricht pro Zeile** (`\n`-terminiert), 115200 Baud auf UART2.

**Pi → ESP (Befehle)**

```json
{"cmd":"heizung","raum":"og2","set":1}     // Heizung OG2 manuell an (set:0 = Automatik)
{"cmd":"aufzug","etage":2}                  // Aufzug zu Etage fahren (0=EG,1=OG1,2=OG2)
{"cmd":"disco","set":1}                     // Disco-Licht an/aus
{"cmd":"alarm","set":1}                     // Alarm scharf/unscharf
```

**ESP → Pi (Status)**

```json
{"typ":"status","temp_eg":21.4}
{"typ":"status","heizung_og2":true}
{"typ":"status","aufzug_etage":2}
```

Neue Befehle werden in `verarbeiteBefehl()` (in `main.cpp`) ergänzt, neue
Statuswerte in `statusSenden()`. Dieses Protokoll bitte mit der Dashboard-Gruppe
abstimmen.

---

## 5. Bauen, flashen, testen

```bash
pio run                 # kompilieren
pio run -t upload       # auf den ESP32 flashen
pio device monitor      # serielle Debug-Ausgabe (USB, 115200)
```

(In VS Code alternativ die PlatformIO-Buttons unten in der Statusleiste.)

**Ohne Hardware testen:** Das Modul `Sensorik` liefert vorerst **simulierte**
Messwerte. Dadurch läuft die komplette Regelungslogik schon, bevor die Sensoren
verbaut sind. Echte Sensoren werden in `lib/Sensorik/Sensorik.cpp` Schritt für
Schritt eingebaut (Anleitung steht dort im Kommentar; Bibliotheken in
`platformio.ini` einkommentieren).

---

## 6. Funktionsumfang (aus dem Projekt-Interview)

| Subsystem        | Umsetzung                                                        | Status im Gerüst |
|------------------|-----------------------------------------------------------------|------------------|
| Heizung (3 Et.)  | Taster = manuell an, sonst Zweipunkt-Hysterese (LED-simuliert)   | ✅ fertig        |
| Aufzug           | Brushed-DC, Referenzfahrt + 2 Endschalter + Zeit + Timeout       | ✅ fertig        |
| Beleuchtung      | NeoPixel, Automatik „Bewegung + Dunkelheit“ (Regel in `main`)    | ✅ fertig        |
| Disco-/Stimmung  | NeoPixel-Animation                                               | ✅ fertig        |
| Beschattung/Dach | Servos am PCA9685 (`Servoaktor`)                                 | ✅ Grundfunktion |
| Sicherheit       | Reed/PIR → Alarmrelais + Buzzer (scharf/unscharf)               | ✅ Grundfunktion |
| Sensorik         | DS18B20 / BH1750 / SHT31                                         | 🟡 simuliert     |
| Pi-Anbindung     | UART2, JSON-Zeilen                                               | ✅ Grundgerüst   |

**Automatik-Regeln** (zentral in `regelung()` in `main.cpp`):
1. Dachfenster OG2 schließen, wenn dort die Heizung läuft.
2. Licht an bei Bewegung **und** Dunkelheit.
3. Alarm auslösen bei Tür-/Fensterkontakt, wenn scharf geschaltet.
4. Jalousie schließen bei hoher Helligkeit.

---

## 7. Ein neues Modul hinzufügen

1. Ordner `lib/MeinModul/` anlegen, darin `MeinModul.h` + `MeinModul.cpp`.
2. Klasse nach dem Muster von `Heizung` bauen: Konstruktor nimmt Pins/Parameter
   (`IoPin`), dann `begin()` und `update()`. Hardware nur über `io.*`.
3. Pins/Parameter in `include/Config.h` eintragen (`espPin(...)`/`mcpPin(...)`).
4. In `src/main.cpp`: Objekt anlegen, in `setup()` `begin()`, in `loop()` `update()`.
5. Bei Bedarf Befehl in `verarbeiteBefehl()` und Status in `statusSenden()` ergänzen.

---

## 8. Zusammenarbeit über GitHub (Empfehlung)

- `main` bleibt immer baubar. Entwickelt wird in **Feature-Branches**
  (z. B. `feature/aufzug-tuning`), zusammengeführt per **Pull Request**.
- **Modul-Eigentümer:** Pro Modul möglichst eine Person, damit Merge-Konflikte
  selten sind. `Config.h` und `main.cpp` werden von beiden angefasst → dort kleine,
  klar getrennte Blöcke pflegen und Änderungen kurz absprechen.
- Aussagekräftige Commits, vor dem Push einmal `pio run` (baut es?).
```
git checkout -b feature/<thema>
# ... arbeiten ...
git add -A && git commit -m "Aufzug: Timeout-Schutz ergaenzt"
git push -u origin feature/<thema>
# danach Pull Request auf GitHub
```

---

## 9. Offene Punkte / TODO

- [ ] Echte Sensoren in `Sensorik` anbinden (DS18B20, BH1750, SHT31).
- [ ] Fahrzeiten des Aufzugs einmessen (`AUFZUG_ZEIT_*` in `Config.h`).
- [ ] Servo-Endlagen je Servo einmessen (`SERVO_TICK_ZU/AUF`).
- [ ] Reale Pin-Belegung & Logikpegel (Reed/PIR) gegen den Schaltplan prüfen.
- [ ] Protokoll mit der Dashboard-Gruppe final abstimmen.
- [ ] FEHLER-Zustand des Aufzugs quittierbar machen (z. B. per Pi-Befehl).
```
