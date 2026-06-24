// =============================================================================
//  integrationTest.cpp  –  prueft das Zusammenspiel der Module (ohne Io-Schicht)
// -----------------------------------------------------------------------------
//  Aktivieren in platformio.ini:  build_src_filter = +<integrationTest.cpp>
//  Deckt ab: Wire-Init + DigitaleOutputs (eigener MCP) + DigitalInput (eigener
//  MCP) + Servoaktor (PCA) + Regelung-Taster-Modus (mit TTL). Zugleich VORLAGE,
//  wie man die Module in main.cpp einhaengt (Wire.begin -> begin -> update).
// =============================================================================
#include <Arduino.h>
#include <Wire.h>
#include "Config.h"
#include "Servoaktor.h"
#include "DigitaleOutputs.h"
#include "DigitalInput.h"
#include "Regelung.h"

TasterState gTaster;   // Etagen-Modus (Hand) mit TTL

void setup() {
  Serial.begin(115200);
  delay(300);

  Wire.begin(PIN_SDA, PIN_SCL);   // I2C EINMAL starten - dann die Module
  digitaleOutputsBegin();         // besitzt + init Ausgangs-MCP
  digitalInputBegin();            // besitzt + init Eingangs-MCP
  servosBegin();                  // PCA9685 (Servos)

  Serial.printf("\n[int] MCP-IN=%s  MCP-OUT=%s\n",
                digitalInputOk() ? "OK" : "FEHLT",
                digitaleOutputsOk() ? "OK" : "FEHLT");
  Serial.println("[int] bereit.");
}

void loop() {
  // 1) Eingaenge -> Etagen-Modus (mit TTL)
  digitalInputUpdate();
  if (geradeGedrueckt(Eingang::TASTER_EG))  tasterWeiterschalten(gTaster, 0);
  if (geradeGedrueckt(Eingang::TASTER_OG1)) tasterWeiterschalten(gTaster, 1);
  if (geradeGedrueckt(Eingang::TASTER_OG2)) tasterWeiterschalten(gTaster, 2);
  tasterTick(gTaster);

  // 2) Modus -> digitale Ausgaenge (Demo)
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    KlimaModus m = tasterModus(gTaster, e);
    heizen((Etage)e,  m == KlimaModus::HEIZEN);
    kuehlen((Etage)e, m == KlimaModus::KUEHLEN);
  }

  // 3) Servos (Demo): Tuer-Reed offen -> Garage auf; je ein Beispiel
  fahreGarage(gedrueckt(Eingang::REED_TUER) ? Position::AUF : Position::ZU);
  fahreJalousie(Position::AUF, Etage::EG, Seite::LINKS);
  fahreDachfenster(Position::ZU, Seite::RECHTS);
  servosUpdate();

  // 4) Status
  static uint32_t t = 0;
  if (millis() - t >= 1000) {
    t = millis();
    Serial.printf("[int] Modus EG/OG1/OG2 = %d/%d/%d  (0=Auto,1=Heiz,2=Kuehl)\n",
                  (int)tasterModus(gTaster, 0), (int)tasterModus(gTaster, 1),
                  (int)tasterModus(gTaster, 2));
  }
}
