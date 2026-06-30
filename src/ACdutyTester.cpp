// =============================================================================
//  AC-Duty-Tester – interaktiver Tester fuer KLIMA_AC_PIN (analogWrite)
// -----------------------------------------------------------------------------
//  Nutzt 1:1 denselben Pfad wie main.cpp::anwenden(): KLIMA_AC_PIN aus
//  Config.h, derselbe analogWrite()-Aufruf, dieselbe 0-255-Duty-Skala.
//
//  Bedienung ueber die serielle Konsole (115200 Baud):
//    40          -> setzt KLIMA_AC_PIN auf Duty 40 (wie AC_DUTY_1ETAGE)
//    255         -> setzt KLIMA_AC_PIN auf Duty 255 (volle Spannung)
//    0           -> AC aus
// =============================================================================
#include <Arduino.h>
#include "Config.h"   // KLIMA_AC_PIN, AC_DUTY_1ETAGE/2ETAGEN/3ETAGEN (zur Anzeige)

int16_t gLetzterDuty = -1;
String  gPuffer;

void zeigeStatus() {
  Serial.printf("[AC-Tester] Pin %d. Aktueller Duty: %d  (~%.2f V bei 3.3V-Logik)\n",
                KLIMA_AC_PIN, gLetzterDuty < 0 ? 0 : gLetzterDuty,
                (gLetzterDuty < 0 ? 0 : gLetzterDuty) / 255.0 * 3.3);
  Serial.printf("  Referenz aus eurer Config: 1 Etage=%d  2 Etagen=%d  3 Etagen=%d\n",
                AC_DUTY_1ETAGE, AC_DUTY_2ETAGEN, AC_DUTY_3ETAGEN);
}

void verarbeiteZeile(const String& zeile) {
  String z = zeile;
  z.trim();
  if (z.length() == 0) return;

  bool istZahl = z.length() > 0;
  for (uint8_t i = 0; i < z.length(); i++)
    if (!isDigit(z[i])) { istZahl = false; break; }

  if (!istZahl) {
    Serial.printf("\nUngueltige Eingabe '%s' - nur Zahlen 0-255 erlaubt.\n", z.c_str());
    return;
  }

  long wert = z.toInt();
  if (wert < 0 || wert > 255) {
    Serial.printf("\nDuty %ld ausserhalb 0-255 - ignoriert.\n", wert);
    return;
  }

  // Exakt derselbe Aufruf wie in main.cpp::anwenden():
  analogWrite(KLIMA_AC_PIN, (uint8_t)wert);
  gLetzterDuty = (int16_t)wert;

  Serial.println();
  zeigeStatus();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[AC-Duty-Tester] Start.");

analogWriteFrequency(20000);
  analogWrite(KLIMA_AC_PIN, 0);   // sicher aus beim Start (wie main.cpp::setup())
  gLetzterDuty = 0;

  gPuffer.reserve(16);
  Serial.println("Bereit. Zahl (0-255) + Enter setzt den Duty an KLIMA_AC_PIN.");
  zeigeStatus();
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    Serial.write(c);   // Echo

    if (c == '\n' || c == '\r') {
      verarbeiteZeile(gPuffer);
      gPuffer = "";
    } else {
      gPuffer += c;
    }
  }
}