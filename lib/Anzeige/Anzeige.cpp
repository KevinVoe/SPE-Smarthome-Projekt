#include "Anzeige.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

namespace {
  LiquidCrystal_I2C lcd(ADDR_LCD, LCD_SPALTEN, LCD_ZEILEN);

  // Text linksbuendig auf die volle Displaybreite auffuellen -> alte Zeichen der
  // vorherigen Aktualisierung werden ueberschrieben (kein clear()-Flackern).
  void printZeile(uint8_t zeile, const char* text) {
    char buf[LCD_SPALTEN + 1];
    snprintf(buf, sizeof(buf), "%-*s", (int)LCD_SPALTEN, text);
    lcd.setCursor(0, zeile);
    lcd.print(buf);
  }

  const char* etageName(uint8_t e) {
    switch (e) {
      case 0: return "EG";
      case 1: return "OG1";
      case 2: return "OG2";
    }
    return "?";
  }
}

void Anzeige::begin() {
  // Praesenz-Check am I2C-Bus (Wire muss vorher in main/setup gestartet sein).
  Wire.beginTransmission(ADDR_LCD);
  _ok = (Wire.endTransmission() == 0);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  printZeile(0, "SPE Smarthome");
  printZeile(1, _ok ? "LCD OK" : "LCD nicht gefunden");
}

void Anzeige::update(const Kontext& k) {
  if (millis() - _letzt < LCD_UPDATE_INTERVALL_MS) return;   // Intervall aus Config
  _letzt = millis();
  _zeichneStatus(k);
}

void Anzeige::_zeichneStatus(const Kontext& k) {
  char zeile[LCD_SPALTEN + 1];

  // Zeile 0: Titel + Uhrzeit HH:MM (aus der simulierten Zeit 0..24 h).
  unsigned std = (unsigned)k.stunde;
  unsigned mn  = (unsigned)((k.stunde - (float)std) * 60.0f);
  snprintf(zeile, sizeof(zeile), "SPE Home    %02u:%02u", std, mn);
  printZeile(0, zeile);

  // Zeile 1: Lufttemperatur + Luftfeuchte.
  snprintf(zeile, sizeof(zeile), "Temp %.1f C  rF %.0f%%", k.temperatur, k.feuchte);
  printZeile(1, zeile);

  // Zeile 2: Solarspannung.
  snprintf(zeile, sizeof(zeile), "Solar %.1f V", k.pvSpannung);
  printZeile(2, zeile);

  // Zeile 3: Aufzugsposition.
  snprintf(zeile, sizeof(zeile), "Aufzug: %s", etageName(k.aufzugEtage));
  printZeile(3, zeile);
}
