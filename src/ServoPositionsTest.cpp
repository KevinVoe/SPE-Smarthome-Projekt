// =============================================================================
//  Servo-Kalibrierung – interaktiver Tester fuer PCA9685-Endlagen
// -----------------------------------------------------------------------------
//  Bedienung ueber die serielle Konsole (115200 Baud, EGAL welches Zeilenende
//  euer Serial Monitor sendet - sowohl '\n' als auch '\r' werden akzeptiert):
//
//    150        -> setzt den AKTUELLEN Kanal auf Tick 150
//    500        -> setzt den AKTUELLEN Kanal auf Tick 500
//    c          -> Kanalwechsel: naechste Eingabe wird als neuer Kanal (0-15)
//                  interpretiert, danach geht es normal mit Pulsweiten weiter
//
//  Funktioniert auch OHNE angeschlossenen PCA9685 (z.B. um die Bedienung
//  blanko zu testen) - ein fehlendes Board fuehrt nur zu einer Warnung beim
//  Start, blockiert das Programm aber nicht.
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "Config.h"   // ADDR_PCA9685, PIN_SDA, PIN_SCL

Adafruit_PWMServoDriver pwm(ADDR_PCA9685);

uint8_t gKanal         = 0;       // aktuell ausgewaehlter PCA9685-Kanal
bool    gWarteAufKanal = false;   // true direkt nach 'c', bis die naechste Zahl kommt
bool    gBoardOk       = false;   // PCA9685 beim Start am Bus erkannt?
String  gPuffer;

void zeigeStatus() {
  Serial.printf("[Kanal %2u] bereit. Zahl=Tickwert senden, 'c'=Kanal wechseln.\n", gKanal);
}

void verarbeiteZeile(const String& zeile) {
  String z = zeile;
  z.trim();
  if (z.length() == 0) return;   // leere Zeile ignorieren (z.B. CRLF-Rest)

  // 'c'/'C' -> Kanalwechsel-Modus aktivieren, naechste Zahl ist der Kanal
  if (z.equalsIgnoreCase("c")) {
    gWarteAufKanal = true;
    Serial.println("\nKanalwechsel: bitte Kanalnummer (0-15) eingeben.");
    return;
  }

  // Eingabe muss eine Zahl sein - sonst klare Fehlermeldung statt stillem Nichtstun
  bool istZahl = z.length() > 0;
  for (uint8_t i = 0; i < z.length(); i++)
    if (!isDigit(z[i])) { istZahl = false; break; }

  if (!istZahl) {
    Serial.printf("\nUngueltige Eingabe '%s' - nur Zahlen oder 'c' erlaubt.\n", z.c_str());
    return;
  }

  long wert = z.toInt();

  if (gWarteAufKanal) {
    if (wert < 0 || wert > 15) {
      Serial.printf("\nKanal %ld ungueltig - erlaubt sind 0-15. Bitte erneut eingeben.\n", wert);
      return;   // bleibt im Kanalwechsel-Modus
    }
    gKanal = (uint8_t)wert;
    gWarteAufKanal = false;
    Serial.println();
    zeigeStatus();
    return;
  }

  // Normalfall: Zahl ist eine Pulsweite (Tick, 0-4095 beim PCA9685, 12-Bit).
  if (wert < 0 || wert > 4095) {
    Serial.printf("\nTickwert %ld ausserhalb 0-4095 - ignoriert.\n", wert);
    return;
  }

  pwm.setPWM(gKanal, 0, (uint16_t)wert);
  Serial.printf("\nKanal %2u -> Tick %4ld gesendet.%s\n", gKanal, wert,
                gBoardOk ? "" : "  (PCA9685 nicht erkannt - ohne Wirkung, nur Logik-Test)");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[ServoKalibrierung] Start.");

  Wire.begin(PIN_SDA, PIN_SCL);
  pwm.begin();
  pwm.setPWMFreq(50);   // 50 Hz, wie im Hauptprojekt

  // Reiner Praesenz-Check (wie servosOk() im Hauptprojekt) - nur Diagnose,
  // blockiert NICHTS, falls das Board fehlt.
  Wire.beginTransmission(ADDR_PCA9685);
  gBoardOk = (Wire.endTransmission() == 0);
  Serial.printf("PCA9685: %s\n", gBoardOk ? "gefunden" : "NICHT gefunden (Eingaben funktionieren trotzdem, nur ohne Hardware-Wirkung)");

  gPuffer.reserve(16);
  Serial.println("Bereit. 'c' + Enter wechselt den Kanal, danach Kanalnummer (0-15) senden.");
  Serial.println("Anschliessend beliebige Tickwerte (0-4095) senden, um den Servo zu bewegen.");
  zeigeStatus();
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    Serial.write(c);   // Echo: sichtbar machen, was ankommt (Terminal echot oft nicht selbst)

    if (c == '\n' || c == '\r') {   // EGAL welches Zeilenende - beides wird akzeptiert
      verarbeiteZeile(gPuffer);
      gPuffer = "";
    } else {
      gPuffer += c;
    }
  }
}