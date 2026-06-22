#include "Regelung.h"
#include "Config.h"

// =============================================================================
//  Tageszeit-Helfer
// =============================================================================
float berechneTageszeit(uint32_t jetztMs, uint32_t tagLaengeMs) {
  if (tagLaengeMs == 0) return 0.0f;
  uint32_t imTag = jetztMs % tagLaengeMs;
  return 24.0f * (float)imTag / (float)tagLaengeMs;
}

Phase phaseAus(float h) {
  if (h <  6.0f) return Phase::NACHT;
  if (h < 10.0f) return Phase::MORGEN;
  if (h < 18.0f) return Phase::TAG;
  if (h < 22.0f) return Phase::ABEND;
  return Phase::NACHT;
}

int8_t sonnenSeiteAus(float h) {
  if (h >=  7.0f && h < 12.0f) return 0;   // vormittags: Sonne links (Ost)
  if (h >= 12.0f && h < 18.0f) return 1;   // nachmittags: Sonne rechts (West)
  return -1;                               // sonst keine relevante Sonne
}

float sollTemperatur(Phase p) {
  return (p == Phase::NACHT) ? SOLL_TEMP_NACHT : SOLL_TEMP_TAG;
}

// =============================================================================
//  Schicht: TAGESZEIT  (Basis Prio 10 + Sperren Prio 60)
// =============================================================================
void tageszeitRegeln(const Kontext& k, Soll& s) {
  bool abendNacht = (k.phase == Phase::ABEND || k.phase == Phase::NACHT);
  bool morgen     = (k.phase == Phase::MORGEN);
  bool tag        = (k.phase == Phase::TAG);
  bool abend      = (k.phase == Phase::ABEND);
  bool nacht      = (k.phase == Phase::NACHT);

 if (morgen) {
    setze(s.licht[0], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 1, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 1, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 1, PRIO_ZEIT_BASIS);
}
else if (tag) {
    setze(s.licht[0], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 0, PRIO_ZEIT_BASIS);
}
else if (abend) {
    setze(s.licht[0], 2, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 3, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 3, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 3, PRIO_ZEIT_BASIS);
}
else if (nacht) {
    setze(s.licht[0], 1, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 1, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 0, PRIO_ZEIT_BASIS);
}

  // Sperre (schlaegt Sensor): nachts bleiben ALLE Jalousien zu.
  /*
  if (k.phase == Phase::NACHT)
    for (uint8_t e = 0; e < ANZ_ETAGEN; e++)
      for (uint8_t seite = 0; seite < ANZ_SEITEN; seite++)
        setze(s.jalousie[e][seite], 100, PRIO_ZEIT_SPERRE);
  */
  // TODO: morgens Jalousie auf, feinere Sollwert-Profile, Nachtruhe (Disco aus) ...
}
/*
// =============================================================================
//  Schicht: SENSOREN  (Prio 50)
// =============================================================================
void sensorRegeln(const Kontext& k, Soll& s) {
  // Klima-Hysterese je Etage - NUR im Automatik-Modus.
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    if (k.klimaModus[e] != KlimaModus::AUTOMATIK) continue;
    float soll = sollTemperatur(k.phase);
    float ist  = k.temperatur[e];
    if (ist < soll - HYSTERESE)      setze(s.heizung[e],  1, PRIO_SENSOR);
    else if (ist > soll + HYSTERESE) setze(s.kuehlLed[e], 1, PRIO_SENSOR);
    // im Hysterese-Band: nichts setzen -> bleibt aus (Basiswert)
  }
  */
  /* Licht: Bewegung UND dunkel -> Etagenlicht voll an.
  if (k.bewegung && k.helligkeit < DUNKEL_LUX)
    for (uint8_t e = 0; e < ANZ_ETAGEN; e++)
      setze(s.licht[e], 3, PRIO_SENSOR);

  // Beschattung: sonnenzugewandte Seite bei viel Licht beschatten.
  if (k.helligkeit > SONNE_HELL_LUX && k.sonnenSeite >= 0)
    for (uint8_t e = 0; e < ANZ_ETAGEN; e++)
      setze(s.jalousie[e][k.sonnenSeite], 100, PRIO_SENSOR);
  
  // TODO: weitere Sensor-Regeln (Feuchte, PIR pro Etage, CO2, ...).
}
  */
// =============================================================================
//  Schicht: HANDEINGRIFF  (Etagen-Klima-Taster, "hart an", Prio 100)
// =============================================================================
/*
void handRegeln(const Kontext& k, Soll& s) {
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    switch (k.klimaModus[e]) {
      case KlimaModus::HEIZEN:    setze(s.heizung[e],  1, PRIO_HAND); break;
      case KlimaModus::KUEHLEN:   setze(s.kuehlLed[e], 1, PRIO_HAND); break;
      case KlimaModus::AUTOMATIK: break;   // nichts -> Sensor/Tageszeit entscheiden
    }
  }

  if (k.discoWunsch) setze(s.disco, 1, PRIO_HAND);
 
  // TODO: weitere Handeingriffe / Dashboard-Befehle (eigene Prio).
}
   */
// =============================================================================
//  INTERLOCKS  –  Konflikte prioritaetsbewusst aufloesen
//  (abgeleitete Schreibvorgaenge mit der Prio ihres Ausloesers)
// =============================================================================
/*
void interlocks(Soll& s) {
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    bool heizen  = s.heizung[e].wert  != 0;
    bool kuehlen = s.kuehlLed[e].wert != 0;

    // Heizen und Kuehlen schliessen sich aus: der niedriger Priorisierte weicht.
    if (heizen && kuehlen) {
      if (s.heizung[e].prio >= s.kuehlLed[e].prio) {
        setze(s.kuehlLed[e], 0, s.heizung[e].prio);  kuehlen = false;
      } else {
        setze(s.heizung[e],  0, s.kuehlLed[e].prio); heizen  = false;
      }
    }

    // Heizen -> Klappe zu (mit Prio des Heizens, damit der Sensor sie nicht oeffnet).
    if (heizen) setze(s.klappe[e], 0, s.heizung[e].prio);

    // Kuehlen -> Klappe auf + zentrales Dachfenster auf (Prio des Kuehlens).
    if (kuehlen) {
      setze(s.klappe[e], 100, s.kuehlLed[e].prio);
      setze(s.dachfensterOG2, 100, s.kuehlLed[e].prio);
    }
  }

  // Zentrale Klimaanlage = ODER ueber alle Etagen mit offener Klappe (= Luftbedarf).
  bool acAn = false; uint8_t acPrio = 0;
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++)
    if (s.klappe[e].wert != 0) {
      acAn = true;
      if (s.klappe[e].prio > acPrio) acPrio = s.klappe[e].prio;
    }
  setze(s.klimaanlage, acAn ? 1 : 0, acPrio);

  // Disco (OG2) an -> Licht OG2 (Kanal 2) aus.
  if (s.disco.wert != 0) setze(s.licht[OG2], 0, s.disco.prio);
}
  */
