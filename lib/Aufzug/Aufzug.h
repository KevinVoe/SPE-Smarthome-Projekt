// =============================================================================
//  Aufzug  –  STEUERUNGSKLASSE fuer den Schrittmotor-Aufzug (STEP/DIR/ENABLE)
// -----------------------------------------------------------------------------
//  WICHTIG: Dieses Modul liest KEINE Taster und KEINE Endschalter selbst aus!
//  Es bietet nur Steuerungsfunktionen:
//
//      aufzug.fahreZu(Aufzug::Etage::OG2);     // Fahrt anfordern
//      aufzug.update(endschalterEg, endschalterOg1, endschalterOg2);  // 1x/loop
//
//  main.cpp liest die Endschalter ueber das Taster-Modul aus und reicht die
//  bereinigten Pegel bei JEDEM loop()-Durchlauf an update() weiter. Aufzug
//  haelt selbst Buch, in welche Richtung gerade gefahren wird, und stoppt den
//  Motor, sobald der zur Zieletage passende Endschalter auslöst.
//
//  ENDSCHALTER SIND DIE EINZIGE WAHRHEIT: es wird KEINE Schrittzaehlung
//  verwendet. Der Motor faehrt einfach in Zielrichtung, bis der passende
//  Endschalter (LOW = ausgeloest) anschlaegt. Ein Timeout schuetzt vor
//  Dauerlauf, falls ein Endschalter defekt ist oder das Seil haengt.
//
//  Vollschritt (kein Microstepping): ein STEP-Puls (HIGH->LOW) = ein Schritt.
//  DIR bestimmt die Richtung, ENABLE muss LOW sein, damit der Treiber den
//  Motor bestromt (A4988/DRV8825-typisch: ENABLE aktiv LOW).
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Io.h"

class Aufzug {
public:
  enum class Etage : uint8_t { EG = 0, OG1 = 1, OG2 = 2 };

  enum class Zustand : uint8_t {
    STEHT,        // keine Fahrt aktiv
    FAEHRT_AUF,
    FAEHRT_AB,
    FEHLER        // Timeout ausgeloest - Fahrt abgebrochen, Motor aus
  };

  Aufzug(IoPin pinStep, IoPin pinDir, IoPin pinEnable, uint32_t stepIntervalUs,
         uint32_t timeoutMs);

  void begin();

  // Steuerungsfunktion: Fahrt zur Zieletage anfordern. Wird ignoriert, wenn
  // bereits eine Fahrt laeuft oder die Zieletage = aktuelle Etage ist.
  void fahreZu(Etage ziel);

  // Einmal pro loop() mit den AKTUELLEN, bereinigten Endschalter-Pegeln
  // aufrufen (true = ausgeloest = Etage erreicht). Taktet den Motor, prueft
  // Zielerreichung und Timeout.
  void update(bool endschalterEg, bool endschalterOg1, bool endschalterOg2);

  Zustand zustand()        const { return _zustand; }
  Etage   aktuelleEtage()  const { return _aktuelleEtage; }
  bool    hatFehler()      const { return _zustand == Zustand::FEHLER; }

  // Fehlerzustand quittieren (z.B. per Pi-Befehl) - siehe README TODO.
  void fehlerQuittieren();

private:
  void _motorEin(bool richtungAufwaerts);
  void _motorAus();
  bool _endschalterFuer(Etage e, bool eg, bool og1, bool og2) const;

  IoPin _pinStep;
  IoPin _pinDir;
  IoPin _pinEnable;

  uint32_t _stepIntervalUs;   // Zeit zwischen zwei STEP-Pulsen (Geschwindigkeit)
  uint32_t _timeoutMs;        // max. Fahrzeit, danach Zustand FEHLER

  Zustand _zustand       = Zustand::STEHT;
  Etage   _aktuelleEtage = Etage::EG;     // Annahme: Aufzug startet unten
  Etage   _zielEtage     = Etage::EG;

  unsigned long _letzterStepUs = 0;
  unsigned long _fahrtStartMs  = 0;
};