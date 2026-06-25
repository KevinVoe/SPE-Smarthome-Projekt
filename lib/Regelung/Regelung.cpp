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

// ─── Dashboard-Regeln (Befehle vom Pi anwenden) ──────────────────────────────
static bool dashAktiv(DashBefehl& b) {
  if (b.wert < 0) return false;
  if ((int32_t)(millis() - b.deadline) >= 0) { b.wert = -1; return false; }  // TTL abgelaufen
  return true;
}

void dashSetze(DashBefehl& b, int wert) {
  b.wert     = (int8_t)wert;
  b.deadline = millis() + DASHBOARD_TTL_MS;
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
    setze(s.disco, 0, PRIO_ZEIT_BASIS);
}
else if (tag) {
    setze(s.licht[0], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 0, PRIO_ZEIT_BASIS);
    setze(s.disco, 0, PRIO_ZEIT_BASIS);
}
else if (abend) {
    setze(s.licht[0], 2, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 3, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 3, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 3, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 0, PRIO_ZEIT_BASIS);
    setze(s.disco, 1, PRIO_ZEIT_BASIS);
}
else if (nacht) {
    setze(s.licht[0], 1, PRIO_ZEIT_BASIS);
    setze(s.licht[1], 1, PRIO_ZEIT_BASIS);
    setze(s.licht[2], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[3], 0, PRIO_ZEIT_BASIS);
    setze(s.licht[4], 0, PRIO_ZEIT_BASIS);
    setze(s.disco, 1, PRIO_ZEIT_BASIS);
}
}


// =============================================================================
//  Schicht: SENSOREN  (Prio 40)
// =============================================================================
void sensorRegeln(const Kontext& k, Soll& s) {
 
}

// =============================================================================
//  Schicht: DASHBOARD  (Befehle vom Pi, Prio 80)
// =============================================================================
void dashboardRegeln(Soll& s, DashboardState& dash) {
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    for (uint8_t seite = 0; seite < ANZ_SEITEN; seite++)
      if (dashAktiv(dash.blind[e][seite]))
        setze(s.jalousie[e][seite], dash.blind[e][seite].wert ? 0 : 100, PRIO_DASHBOARD);

    if (dashAktiv(dash.heat[e]))  setze(s.heizung[e], dash.heat[e].wert,  PRIO_DASHBOARD);
    if (dashAktiv(dash.light[e])) setze(s.licht[e+2],   dash.light[e].wert, PRIO_DASHBOARD);
  }
  if (dashAktiv(dash.ext_light))  setze(s.licht[0], dash.ext_light.wert,  PRIO_DASHBOARD);
  if (dashAktiv(dash.door_light)) setze(s.licht[1], dash.door_light.wert, PRIO_DASHBOARD);
  if (dashAktiv(dash.party))     setze(s.disco,          dash.party.wert,               PRIO_DASHBOARD);
  if (dashAktiv(dash.ac))        setze(s.klimaanlage,    dash.ac.wert,                  PRIO_DASHBOARD);
  if (dashAktiv(dash.skylight2)) setze(s.dachfensterOG2, dash.skylight2.wert ? 100 : 0, PRIO_DASHBOARD);
  // TODO: mode/tv/skylight1/garage/front_door/elevator haben (noch) kein Soll-Feld
}

// =============================================================================
//  Schicht: HANDEINGRIFF  (Etagen-Klima-Taster, "hart an", Prio 100)
// =============================================================================
void handRegeln(const Kontext& k, Soll& s) {
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++) {
    switch (k.klimaModus[e]) {
      case KlimaModus::HEIZEN:    setze(s.heizung[e],  1, PRIO_HAND); break;
      case KlimaModus::KUEHLEN:   setze(s.kuehlLed[e], 1, PRIO_HAND); break;
      case KlimaModus::AUTOMATIK: break;   // nichts -> Sensor/Tageszeit entscheiden
    }
  }
}




// ─── Etagen-Modus per Taster (Hand) mit TTL ──────────────────────────────────
// Bei jedem Tastendruck einen Modus weiterschalten und die TTL neu aufziehen.
void tasterWeiterschalten(TasterState& ts, uint8_t e) {
  switch (ts.modus[e]) {                                   // Zyklus:
    case KlimaModus::AUTOMATIK: ts.modus[e] = KlimaModus::HEIZEN;    break;  // Auto -> Heizen
    case KlimaModus::HEIZEN:    ts.modus[e] = KlimaModus::KUEHLEN;   break;  // Heizen -> Kuehlen
    case KlimaModus::KUEHLEN:   ts.modus[e] = KlimaModus::AUTOMATIK; break;  // Kuehlen -> Auto
  }
  ts.deadline[e] = millis() + TASTER_TTL_MS;
}

// Abgelaufene Hand-Modi fallen automatisch auf Automatik zurueck.
void tasterTick(TasterState& ts) {
  for (uint8_t e = 0; e < ANZ_ETAGEN; e++)
    if (ts.modus[e] != KlimaModus::AUTOMATIK &&
        (int32_t)(millis() - ts.deadline[e]) >= 0)
      ts.modus[e] = KlimaModus::AUTOMATIK;
}

KlimaModus tasterModus(const TasterState& ts, uint8_t e) {
  return ts.modus[e];
}

// =============================================================================
//  INTERLOCKS  –  Konflikte prioritaetsbewusst aufloesen
//  (abgeleitete Schreibvorgaenge mit der Prio ihres Ausloesers)
// =============================================================================
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

  // Disco (OG2) und OG2-Raumlicht schliessen sich aus (selber Raum). Der hoeher
  // Priorisierte gewinnt; bei Gleichstand gewinnt die Disco.
  // ACHTUNG: OG2-Raumlicht ist Soll-Kanal 4 (Config: K4 = OG2_Licht),
  //          NICHT licht[OG2]=licht[2] (das waere das EG-Licht)!
  constexpr uint8_t LICHT_OG2 = 4;
  bool discoAn    = (s.disco.wert != 0);
  bool og2LichtAn = (s.licht[LICHT_OG2].wert != 0);
  if (discoAn && og2LichtAn) {
    if (s.disco.prio >= s.licht[LICHT_OG2].prio)
      setze(s.licht[LICHT_OG2], 0, s.disco.prio);   // Disco gewinnt -> Licht aus
    else
      setze(s.disco, 0, s.licht[LICHT_OG2].prio);   // Licht gewinnt -> Disco aus
  }
}
