Verstanden — großer Magnet + langsame Kabine heißt: der Reed ist lange geschlossen, die Entprellung war also nicht die Ursache. Ich nehme den Roh-Read zurück (sonst kann Rauschen fälschlich triggern), verkleinere die Entprellung, und habe einen deutlich wahrscheinlicheren Verursacher gefunden.

Zuerst die Reverts + kleinere Entprellung:


Edited 3 files
Jetzt der wahrscheinliche Hauptverdächtige — der Timeout. Ich erhöhe ihn und erkläre warum:


Edited a file, read a file
Stopp — die Config wurde bereits angepasst: AUFZUG_TIMEOUT_MS = 600000 (10 min) und STEP_INTERVALL_US = 800. Damit ist der Timeout nicht die Ursache (war meine erste Vermutung, hat sich erledigt). Ich lasse die Werte unverändert und suche weiter. Ich schaue mir die Debug-Ausgabe an, um dir ein Diagnose-Werkzeug einzubauen:

Read
main.cpp
(244–273, 272–283)
Es gibt schon eine Aufzug-Debugzeile. Ich erweitere sie um Zieletage + Live-Reed-Zustände — das ist genau das Diagnose-Fenster, um zu sehen, ob der Ziel-Reed während der Fahrt erkannt wird. Dafür ein zielEtage()-Getter:


Edited 2 files, ran 2 commands
Erledigt, gebaut, gepusht (3bee196). Reverts sind drin (Reeds wieder entprellt, ENTPRELL_MS 50→15 ms), Timeout/Step lasse ich wie von dir gesetzt.

Was der Aufzug WÄHREND DER FAHRT macht — genau
Ja, im Kern wartet er einfach darauf, dass der Reed der Zieletage schließt. Detailliert, Schritt für Schritt (Aufzug::update(), läuft jede Loop):

Ist keine Fahrt aktiv (STEHT/FEHLER) → tut er nichts.
Oberer Überfahr-Schalter zu? → sofort _motorAus() + Zustand FEHLER.
Timeout überschritten? (AUFZUG_TIMEOUT_MS, aktuell 600 s) → FEHLER.
Position nachführen: Er schaut, welcher Etagen-Reed gerade geschlossen ist, und setzt _aktuelleEtage darauf (auch beim Durchfahren von Zwischenetagen). Es wird nicht gezählt — der Reed ist die einzige Wahrheit.
Ziel erreicht? if (_aktuelleEtage == _zielEtage) → _motorAus() + STEHT. Genau hier stoppt er.
Sonst: einen Halb-/Vollschritt weiter (alle AUFZUG_STEP_INTERVALL_US) in Fahrtrichtung.
Die Richtung wird einmal beim Start aus Ziel vs. _aktuelleEtage bestimmt (fahreZu()), danach läuft er stur in diese Richtung, bis der Ziel-Reed kommt.

Daraus ergeben sich die wahrscheinlichen Ursachen (Timeout ausgeschlossen)
Ziel-Reed wird nie als „geschlossen" gelesen → er stoppt nie an der Etage und fährt durch (bis oben oder Timeout → FEHLER). Mögliche Gründe: Reed/Magnet-Versatz, falsche Pin-Zuordnung, oder der Eingangs-MCP antwortet nicht (→ neues i2c.mcp_in:false im JSON). Das ist mein Hauptverdacht — passt auch dazu, dass die Servos gar nicht laufen (gemeinsame I2C-Ursache).
FEHLER-Falle: Ist er einmal in FEHLER (z. B. durch den alten 15-s-Timeout in einem früheren Test oder den oberen Schalter), ignoriert er alle weiteren Rufe — fahreZu() kehrt sofort zurück, bis fehlerQuittieren() aufgerufen wird (passiert nirgends, nur ein Reset/Neustart räumt es weg). Im Debug an z=3 erkennbar.
Startposition: Er nimmt beim Boot an, er stehe im EG (_aktuelleEtage = EG). Steht die Kabine beim Einschalten woanders, kann die erste Fahrt in die falsche Richtung gehen → Ziel-Reed kommt nie. Sauber löst das eine Referenzfahrt beim Start (runter bis EG-Reed) — ist noch nicht drin.
Drehrichtung verdrahtungsabhängig: Wenn die ULN2003-Spulen anders verdrahtet sind, fährt „auf" physisch nach unten.
Dein Diagnose-Werkzeug
Die Debug-Zeile (USB-Seriell, 1×/s) zeigt jetzt:

Aufzug z=1 ist=0 ziel=2 reeds[EG=1 OG1=0 OG2=0 ob=0]
z = Zustand (0 STEHT, 1 fährt auf, 2 fährt ab, 3 FEHLER)
ist/ziel = aktuelle/Zieletage
reeds[...] = Live-Zustand der vier Reeds
Damit siehst du beim Test sofort: Fährt er (z=1/2)? Wird beim Erreichen der Zieletage reeds[OGx]=1? Wenn der Ziel-Reed beim Vorbeifahren nie auf 1 geht → Hardware/Reed/MCP. Wenn er auf 1 geht, aber nicht stoppt → meld dich, dann ist es ein Logikfehler, den wir gezielt fangen.

Meine Fragen an dich
Zeigt das i2c-Feld im JSON mcp_in, mcp_out, pca — sind die true oder false? Das engt die Ursache (Verdrahtung vs. Logik) sofort ein.
Beim Reed-Test (Magnet langsam vorbeiführen): geht das entsprechende reeds[...] auf 1?
Soll ich die FEHLER-Falle test-freundlich machen (z. B. bei einem neuen Ruf automatisch quittieren) und/oder eine Referenzfahrt beim Start einbauen? Beides würde die typischen „hängt fest"-Effekte beim Testen beseitigen.