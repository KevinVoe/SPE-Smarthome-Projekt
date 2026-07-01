# TODO — Umsetzung (Branch: TODO-Umsetzung)

1. [x] **Garage per Dashboard**: `cmd "garage"` → `Soll.garage` → Garagen-Servo; Zustand in Telemetrie (`outdoor.garage`).
2. [x] **Ultraschallsensor** (HC-SR04, Trig=GPIO12 / Echo=GPIO35): Objekt näher als `GARAGE_OBJEKT_CM` (20 cm) → Garage öffnet für `GARAGE_AUF_ZEIT_MS` (8 s, konfigurierbar). ⚠️ Echo liefert 5 V → **Spannungsteiler auf 3,3 V** nötig!
3. [x] **Aufzug-Laufruhe**: Schrittmotor wird jetzt über einen **ESP32-Hardware-Timer** (`esp_timer`) getaktet → gleichmäßig, unabhängig von Loop-Blockaden (LCD-Refresh, DHT-Lesen). Position bleibt reed-basiert. Diagnose: das Ruckeln kam vom Takten aus der blockierenden Haupt-Loop. Falls weiterhin unruhig → `AUFZUG_STEP_INTERVALL_US` erhöhen (28BYJ ist bei 1 ms nah am Limit) bzw. Mechanik prüfen.
4. [x] **Garten-/Gewächshaus-Feuchte**: war bereits eingelesen + in Telemetrie; jetzt zusätzlich auf dem **LCD** (Solar-Zeile: „Gart XX%").
5. [x] **Servo-Fiepen**: nach Erreichen der Zielposition wird das PWM-Signal abgeschaltet (Verzug `SERVO_ABSCHALT_VERZUG_MS` = 500 ms) → leise. **Ausnahme:** Garagen-Servo hält sein Signal (Haltekraft).
6. [x] **Tageszyklus 4 min**: `TAG_LAENGE_MS = 240000`.
7. [x] **Whirlpool + AC nur an/aus**: `digitalWrite` statt PWM (kein Duty/Anlauf-Kick mehr).
8. [x] **Handeingriff verlängert**: `DASHBOARD_TTL_MS` 3 s → 30 s.
