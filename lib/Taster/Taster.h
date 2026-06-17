// =============================================================================
//  Taster  –  zentrale STEUERUNGSKLASSE fuer ALLE Taster im Haus
// -----------------------------------------------------------------------------
//  WICHTIG: Dieses Modul ist die EINZIGE Stelle im Projekt, die Taster-/
//  Endschalter-Pins abfragt. Es kuemmert sich ausschliesslich um "Sicherheits-
//  features" wie Entprellung - es kennt KEINE Modi, KEINE Aufzugslogik usw.
//
//  Aufruf aus main.cpp:
//      taster.begin();        // einmal in setup()
//      taster.update();       // einmal pro loop() - liest alle Pins entprellt
//      bool gedrueckt = taster.status().eg;   // bereinigter, aktueller Pegel
//
//  "Bereinigter Pegel" bedeutet hier: true = Taster/Schalter wird JETZT
//  gehalten bzw. ausgeloest (entprellt). KEINE Flankenerkennung - das (z.B.
//  fuer Moduswechsel oder Aufzugsfahrten) macht main.cpp selbst, indem es den
//  Status zweier aufeinanderfolgender update()-Aufrufe vergleicht.
//
//  ERWEITERBARKEIT: Neuer Taster/Schalter im Haus =
//   1. Feld in TasterStatus ergaenzen (unten)
//   2. IoPin + Eintrag in der Pin-Liste im Konstruktor (Taster.cpp) ergaenzen
//   3. Zeile in update() ergaenzen, die das neue Feld setzt
// =============================================================================
#pragma once
#include <Arduino.h>
#include "Io.h"

// Bereinigte (entprellte) Zustaende ALLER Taster/Endschalter im Haus.
struct TasterStatus {
  bool eg  = false;   // Modus-Taster Erdgeschoss
  bool og1 = false;   // Modus-Taster 1. Obergeschoss
  bool og2 = false;   // Modus-Taster 2. Obergeschoss

  bool aufzugEg  = false;   // Aufzug-Ruftaste Erdgeschoss
  bool aufzugOg1 = false;   // Aufzug-Ruftaste 1. Obergeschoss
  bool aufzugOg2 = false;   // Aufzug-Ruftaste 2. Obergeschoss

  bool endschalterEg  = false;   // Aufzug-Endschalter Erdgeschoss (Position erreicht)
  bool endschalterOg1 = false;   // Aufzug-Endschalter 1. Obergeschoss
  bool endschalterOg2 = false;   // Aufzug-Endschalter 2. Obergeschoss
  // weitere Taster hier ergaenzen
};

class Taster {
public:
  Taster(IoPin pinEg, IoPin pinOg1, IoPin pinOg2,
         IoPin pinAufzugEg, IoPin pinAufzugOg1, IoPin pinAufzugOg2,
         IoPin pinEndschalterEg, IoPin pinEndschalterOg1, IoPin pinEndschalterOg2);

  void begin();
  void update();                          // einmal pro loop() aufrufen

  const TasterStatus& status() const { return _status; }

private:
  // Eine entprellte Einzelabfrage. Liefert den AKTUELLEN, bereinigten Pegel
  // (true = gehalten/ausgeloest) - keine Flankenerkennung.
  bool _entprelltLesen(IoPin pin, int& letzterRohwert, unsigned long& letzteAenderung,
                        bool& letzterBereinigterWert);

  IoPin _pinEg, _pinOg1, _pinOg2;
  IoPin _pinAufzugEg, _pinAufzugOg1, _pinAufzugOg2;
  IoPin _pinEndschalterEg, _pinEndschalterOg1, _pinEndschalterOg2;

  TasterStatus _status;

  // Pro Eingang eigener Entprell-Zustand (roh gelesener Pegel, Zeitpunkt der
  // letzten Aenderung, zuletzt bestaetigter bereinigter Wert).
  int           _rohEg = HIGH,  _rohOg1 = HIGH,  _rohOg2 = HIGH;
  unsigned long _tEg   = 0,     _tOg1   = 0,     _tOg2   = 0;
  bool          _bEg   = false, _bOg1   = false, _bOg2   = false;

  int           _rohAufzugEg = HIGH,  _rohAufzugOg1 = HIGH,  _rohAufzugOg2 = HIGH;
  unsigned long _tAufzugEg   = 0,     _tAufzugOg1   = 0,     _tAufzugOg2   = 0;
  bool          _bAufzugEg   = false, _bAufzugOg1   = false, _bAufzugOg2   = false;

  int           _rohEndEg = HIGH,  _rohEndOg1 = HIGH,  _rohEndOg2 = HIGH;
  unsigned long _tEndEg   = 0,     _tEndOg1   = 0,     _tEndOg2   = 0;
  bool          _bEndEg   = false, _bEndOg1   = false, _bEndOg2   = false;

  static constexpr unsigned long _entprellMs = 50;
};