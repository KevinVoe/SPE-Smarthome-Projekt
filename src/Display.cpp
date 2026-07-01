// =============================================================================
//  LCD-Tester – 20x4 HD44780 ueber PCF8574-I2C-Backpack
// -----------------------------------------------------------------------------
//  Prueft nacheinander:
//    1. I2C-Praesenz (ist das LCD am Bus erreichbar?)
//    2. Backlight an/aus (sichtbarer Hardware-Check)
//    3. Alle 4 Zeilen mit Text befuellen
//    4. Cursor-Positionierung auf jede Zeile
//  Gibt den gesamten Status zusaetzlich auf Serial aus (115200 Baud).
//
//  Falls das Display nicht reagiert:
//    - ADDR_LCD in Config.h pruefen (haeufig 0x27 oder 0x3F)
//    - I2C-Scanner unten einkommentieren -> gibt alle gefundenen Adressen aus
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Config.h"   // ADDR_LCD, LCD_SPALTEN, LCD_ZEILEN, PIN_SDA, PIN_SCL

LiquidCrystal_I2C lcd(ADDR_LCD, LCD_SPALTEN, LCD_ZEILEN);

// Zeile vollstaendig beschreiben (kein flackerndes clear() noetig)
void printZeile(uint8_t zeile, const char* text) {
  char buf[LCD_SPALTEN + 1];
  snprintf(buf, sizeof(buf), "%-*s", (int)LCD_SPALTEN, text);
  lcd.setCursor(0, zeile);
  lcd.print(buf);
}

void i2cScanner() {
  Serial.println("I2C-Scanner:");
  bool gefunden = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  Geraet gefunden: 0x%02X\n", addr);
      gefunden = true;
    }
  }
  if (!gefunden) Serial.println("  Kein I2C-Geraet gefunden!");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n[LCD-Tester] Start.");

  Wire.begin(PIN_SDA, PIN_SCL);

  // ── Schritt 1: I2C-Praesenz pruefen ────────────────────────────────────────
  Wire.beginTransmission(ADDR_LCD);
  bool ok = (Wire.endTransmission() == 0);
  Serial.printf("LCD an Adresse 0x%02X: %s\n", ADDR_LCD, ok ? "GEFUNDEN" : "NICHT GEFUNDEN");

  if (!ok) {
    Serial.println("Starte I2C-Scanner um echte Adresse zu finden...");
    i2cScanner();
    Serial.println("-> ADDR_LCD in Config.h anpassen, dann neu flashen.");
    // Programm laeuft weiter, lcd.init() kann trotzdem aufgerufen werden
    // (haengt sich nicht auf, zeigt nur nichts an)
  }

  // ── Schritt 2: LCD initialisieren ──────────────────────────────────────────
  lcd.init();
  lcd.backlight();
  Serial.println("LCD initialisiert, Backlight an.");

  // ── Schritt 3: Alle 4 Zeilen beschreiben ───────────────────────────────────
  printZeile(0, "SPE Smarthome Test");
  printZeile(1, "Zeile 1 ok?");
  printZeile(2, "Zeile 2 ok?");
  printZeile(3, "Zeile 3 ok?");
  Serial.println("Alle 4 Zeilen beschrieben.");

  delay(2000);

  // ── Schritt 4: Backlight-Blink (sichtbarer Hardware-Check) ─────────────────
  Serial.println("Backlight-Blink 3x...");
  for (uint8_t i = 0; i < 3; i++) {
    lcd.noBacklight(); delay(300);
    lcd.backlight();   delay(300);
  }

  // ── Schritt 5: Cursor-Test (jede Zeile einzeln hervorheben) ────────────────
  for (uint8_t z = 0; z < LCD_ZEILEN; z++) {
    char buf[LCD_SPALTEN + 1];
    snprintf(buf, sizeof(buf), ">>> Zeile %d aktiv <<<", z);
    printZeile(z, buf);
    delay(800);
  }

  printZeile(0, "Test abgeschlossen!");
  printZeile(1, ok ? "LCD OK" : "LCD nicht gefunden");
  printZeile(2, "Adresse:");
  char adrbuf[LCD_SPALTEN + 1];
  snprintf(adrbuf, sizeof(adrbuf), "0x%02X (Config: 0x%02X)", ADDR_LCD, ADDR_LCD);
  printZeile(3, adrbuf);

  Serial.println("Setup-Test abgeschlossen. loop() laeuft weiter (Sekundenanzeige).");
}

uint32_t sek = 0;

void loop() {
  // Einfache Sekundenanzeige damit man sieht, dass das Programm laeuft
  static uint32_t letzt = 0;
  if (millis() - letzt >= 1000) {
    letzt = millis();
    char buf[LCD_SPALTEN + 1];
    snprintf(buf, sizeof(buf), "Uptime: %lus", (unsigned long)sek++);
    printZeile(3, buf);
  }
}