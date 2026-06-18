# Architektur – SPE Smarthome (ESP32-Steuerung)

Dieses Dokument beschreibt den **Software-Aufbau** der Steuerung/Regelung.
Es ist die gemeinsame Referenz fürs Team. Hardware-Pins stehen in
[`include/Config.h`](include/Config.h).

## 1. Grundprinzip

- **`lib/` = dumme Treiber.** Jedes Modul steuert genau ein Hauselement und
  enthält **keine** Entscheidungslogik (z. B. `Heizung::setState(true)`,
  `Licht::setKanal(k, stufe)`, `Aufzug::fahreZu(...)`). Sensoren/Taster liefern
  nur Werte (`Sensorik`, `Taster`).
- **Die gesamte Logik steckt in der Regelung** (`lib/Regelung` + sichtbarer
  Aufruf in `main.cpp`). Wer verstehen will, *was das Haus tut*, liest die
  Regelung – nicht 10 Module.
- **Kooperativ, nicht-blockierend** (`millis()`, jedes Modul `update()`,
  kein `delay()`).

## 2. Datenfluss pro `loop()`

```
1. EINGÄNGE → Kontext
   taster.update() · sensorik.update() · zeit = berechneTageszeit(...)
   + Flankenerkennung Klima-Taster → klimaModus[etage]

2. SOLL-ZUSTAND bilden  (jede Regel: setze(feld, wert, PRIO))
   tageszeitRegeln(k, soll)   // Basis (10) + Sperren (60)
   sensorRegeln(k, soll)      // 50
   handRegeln(k, soll)        // 100
   → höchste PRIO pro Feld gewinnt (Aufrufreihenfolge egal!)

3. INTERLOCKS  interlocks(soll)   // Konflikte prioritätsbewusst auflösen

4. ANWENDEN  anwenden(soll)       // Soll → dumme Module (Hardware)

5. aufzug.update(endschalter…) · kommunikation …
```

## 3. Soll-Zustand (Blackboard)

Jedes Aktorziel ist ein `Feld { int16_t wert; uint8_t prio; }`. Geschrieben
wird über:

```cpp
inline void setze(Feld& f, int16_t wert, uint8_t prio) {
  if (prio >= f.prio) { f.wert = wert; f.prio = prio; }  // höchste Prio gewinnt
}
```

Der `Soll` wird **jede Loop neu gebaut** (alle Felder default 0). Felder:

| Feld | Bedeutung | Werte |
|------|-----------|-------|
| `heizung[3]` | rote Heiz-LED je Etage | 0/1 |
| `kuehlLed[3]` | blaue Kühl-LED je Etage | 0/1 |
| `klappe[3]` | Luftklappe (Servo) je Etage | 0=zu, 100=auf |
| `klimaanlage` | zentrale Anlage auf dem Dach | 0/1 |
| `dachfensterOG2` | Dachfenster (nur OG2, Servo) | 0=zu, 100=auf |
| `jalousie[3][2]` | je Etage Links/Rechts (Servo) | 0=offen, 100=beschattet |
| `licht[6]` | PWM-Kanäle (s.u.) | Stufe 0..3 |
| `disco` | Disco-Licht (nur OG2) | 0/1 |

**Licht-Kanäle:** K0=EG, K1=OG1, K2=OG2, K3=Terrasse, K4=Eingang, K5=frei.

## 4. Prioritäten

Nicht die Schicht-Reihenfolge entscheidet, sondern eine **Prio-Zahl pro Regel**.
So kann die Tageszeit den Sensor in *einem* Fall schlagen (Jalousie nachts) und
ihm in einem *anderen* unterliegen (Heiz-Sollwert). Bänder (frei justierbar):

| Prio | Quelle | Beispiel |
|------|--------|----------|
| 100 | Handeingriff (Etagen-Taster) | Modus „Heizen" hart an |
| 60 | Tageszeit-**Sperre** | Jalousie nachts zu (schlägt Sensor) |
| 50 | Sensor | Hysterese, „zu dunkel → Licht", Beschattung |
| 10 | Tageszeit-**Basis** | Grundlicht, Tag/Nacht-Sollwert |

(Reserviert: Dashboard/Pi-Befehle später ~90/110.)

## 5. Interlocks (Konflikte)

Laufen **nach** den Schichten auf dem fertigen Soll. Eine abgeleitete Maßnahme
schreibt mit der **Prio ihres Auslösers** – so kann eine niedrigere Schicht sie
nicht aushebeln.

- **Heizen[e]** → `klappe[e]=zu`, `kuehlLed[e]=aus`.
- **Kühlen[e]** → `kuehlLed[e]=an`, `klappe[e]=auf`, `dachfensterOG2=auf`.
- **Heizen ⇔ Kühlen** je Etage gegenseitig ausgeschlossen (niedrigere Prio weicht).
- **Zentrale Klimaanlage = ODER** über alle Etagen mit offener Klappe (Luftbedarf).
- **Disco(OG2)** an → `licht[K2]=aus`.

## 6. Klima-Logik

- **Automatik:** Sollwert aus Tageszeit (`SOLL_TEMP_TAG`/`_NACHT`), Zweipunkt mit
  `HYSTERESE`: unter Schwelle heizen, über Schwelle kühlen, im Band neutral.
- **Hand (Taster, 3 Modi Heizen/Kühlen/Automatik):** „hart an" mit Prio 100;
  Interlocks erzwingen die passenden Begleitaktionen.
- **Kühlen physisch:** blaue LED (Etage) + zentrale Klimaanlage + Etagen-Klappe +
  OG2-Dachfenster. Luft kommt nur an, wenn Anlage läuft **und** Klappe offen.

## 7. Tageszeit-Simulation

`berechneTageszeit(millis(), TAG_LAENGE_MS)` → Uhrzeit `0..24` h, dazu
`phaseAus()` (Nacht/Morgen/Tag/Abend) und `sonnenSeiteAus()` (Ost→West-Wanderung
für die seitenabhängige Beschattung). `TAG_LAENGE_MS` = Länge eines simulierten
Tages.

## 8. Beleuchtung & Beschattung

- **Licht-Automatik:** Bewegung (PIR) **und** dunkel (Sensor), plus Tageszeit-Basis.
- **Beschattung:** Sonnenrichtung aus Tageszeit **kombiniert mit** Helligkeit;
  die sonnenzugewandte Seite jeder Etage wird einzeln beschattet.

## 9. Dateien

```
include/Config.h        Pins, Adressen, (später) Tuning-Parameter
lib/<Modul>/            dumme Treiber: Heizung, Licht, DiscoLight, Servoaktor,
                        Aufzug, Taster, Sensorik, Io, Kommunikation
lib/Regelung/           Soll-Struct + setze() + Schichten + interlocks (HW-frei)
src/main.cpp            Kontext bauen → Schichten → interlocks → anwenden()
```

`anwenden()` lebt in `main.cpp` (dort sind die Modul-Objekte) und ist die einzige
Stelle, die den Soll-Zustand auf echte Hardware überträgt.

## 10. Offene Punkte

- Alarm (noch nicht geplant).
- Echte Sensoren statt Simulation; MCP23017 + Pin-Entflechtung bei Vollintegration.
- Servo-/Klappen-Endlagen einmessen.
- Tuning-Parameter aus `lib/Regelung/Regelung.h` ggf. nach `Config.h` zentralisieren.
- Dashboard/Pi-Befehle als eigene Prio-Quelle.
