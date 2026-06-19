// =============================================================================
//  SBT0811 / ULN2003 Stepper-Test – Richtungswechsel alle 2 Sekunden
// -----------------------------------------------------------------------------
//  Verkabelung:
//    IN1 -> GPIO 25
//    IN2 -> GPIO 26
//    IN3 -> GPIO 27
//    IN4 -> GPIO 14
//    GND -> ESP32 GND
//    VCC -> separate 5V-Versorgung (NICHT vom ESP32!)
// =============================================================================
#include <Arduino.h>

// ── Pinbelegung ───────────────────────────────────────────────────────────────
constexpr int IN1 = 25;
constexpr int IN2 = 26;
constexpr int IN3 = 27;
constexpr int IN4 = 14;

// ── Halbschritt-Sequenz (8 Schritte) – Standard fuer ULN2003/28BYJ-48 ────────
const uint8_t SCHRITTE[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1},
};

// ── Zeittakte (nicht-blockierend, kein delay() im Hauptablauf) ───────────────
constexpr uint32_t SCHRITT_INTERVALL_US   = 2000;   // Zeit zwischen Einzelschritten
constexpr uint32_t RICHTUNGSWECHSEL_MS    = 2000;   // alle 2 s Richtung wechseln

uint32_t letzterSchritt    = 0;
uint32_t letzterWechsel    = 0;
int8_t   schrittIndex      = 0;
bool     vorwaerts         = true;

// ── Eine Halbschritt-Phase ausgeben ──────────────────────────────────────────
void schreibeSchritt(uint8_t index) {
  digitalWrite(IN1, SCHRITTE[index][0]);
  digitalWrite(IN2, SCHRITTE[index][1]);
  digitalWrite(IN3, SCHRITTE[index][2]);
  digitalWrite(IN4, SCHRITTE[index][3]);
}

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(115200);
  Serial.println("=== Stepper Richtungswechsel-Test ===");

  letzterSchritt = micros();
  letzterWechsel = millis();
}

void loop() {
  uint32_t jetztMs = millis();
  uint32_t jetztUs = micros();

  // ── Richtung alle 2 Sekunden umschalten ────────────────────────────────────
  if (jetztMs - letzterWechsel >= RICHTUNGSWECHSEL_MS) {
    letzterWechsel = jetztMs;
    vorwaerts = !vorwaerts;
    Serial.println(vorwaerts ? "Richtung: VORWAERTS" : "Richtung: RUECKWAERTS");
  }

  // ── Naechsten Einzelschritt ausgeben, wenn Intervall erreicht ─────────────
  if (jetztUs - letzterSchritt >= SCHRITT_INTERVALL_US) {
    letzterSchritt = jetztUs;

    if (vorwaerts) {
      schrittIndex = (schrittIndex + 1) % 8;
    } else {
      schrittIndex = (schrittIndex - 1 + 8) % 8;
    }

    schreibeSchritt(schrittIndex);
  }
}