# Änderungen

Projektversion (Veröffentlichung) und je Spiel/Mod die Coop-Fassung `uNN`. Vorgänger: das Othello-Coop
(github.com/corshaegar/Othello-Coop, 0.1.0–0.1.2, nur Othello). Die Fassungen dazwischen (u1–u61) stehen im Entwicklungsverlauf;
hier beginnt der Verlauf mit u62.

## 0.4.1 – Türen gemeinsam (Gothic I u143, Gothic II u143, Othello u143)

- **Türen** (u143): Öffnen, Schließen und Aufschließen durch einen Helden gelten jetzt für beide. 0.4.0 nannte Türen schon, tatsächlich kam der Abgleich dort nie an: Die Engine ruft bei Türen die Funktion nicht auf, an der er hing. NPCs benutzen Türen weiter je PC, offen oder zu kann also abweichen, wenn ein NPC durchgeht.

## 0.4.0 – Mechanismen gemeinsam, Weltwechsel ohne doppelte NPCs, Drachen auf beiden PCs (Gothic I u142, Gothic II u142, Othello u142; nur Tag `v0.4.0`, kein eigenes Release)

**Neu**

- **Mechanismen gemeinsam** (u137): Hebel, Steinplatten und Portale gelten jetzt für beide (Türen erst ab 0.4.1), samt den Skripten und Kamerafahrten dahinter. Bisher blieb zum Beispiel ein Portal beim Partner zu; nach dem Laden war es für beide wieder zu und der eingesetzte Gegenstand trotzdem weg. Feuerstellen bleiben je PC (u138).

**Korrekturen**

- **Weltwechsel** (u140, u141, Gothic 2): Wer eine Welt als Zweiter betrat, holte die Weltdatei des Partners, und das Einzugs-Skript lief trotzdem noch einmal. Im Minental standen dann NPCs doppelt, und die Doppelten liefen nicht im Abgleich. Die Merker kommen jetzt mit der Weltdatei, und was der Einzug setzt, bekommt auch der Partner. Spielstände, in denen schon doppelte NPCs stehen, bleiben so.
- **Drachen** (u139, u140): Das Gespräch macht einen Drachen sterblich, das galt aber nur auf dem PC des Sprechenden. Schutz, Gilde und Typ der NPCs laufen jetzt mit. Spielstände, in denen ein Drache dadurch unsterblich blieb, heilen beim Laden.
- **Abstürze:** Speichern bei offenem Zuschau-Fenster ohne `AtmosphericScattering` (u135, Gothic 2, Renderer coop9); Kastell im Minental, Raum mit Gilde -1 (u136, Gothic 2); Laden eines Spielstands, der während einer Kamerafahrt entstand (u138); Gegenstand einer laufenden Nachricht schon freigegeben, vor allem beim Client in Gothic 1 und Othello (u142, abgefangen); Kamera des Weltprozesses im ersten Bild nach einem Weltwechsel (u142).

## 0.3.1 – Gothic 1 und Othello automatisch geprüft, Korrekturen am Weltprozess (Gothic I u134, Gothic II u134, Othello u134)

Erstmals läuft der automatische Testlauf (Host und Client auf einem PC) auch in Gothic 1 und Othello: Ergebnisse im [README](README.md#teststand). Was er dabei gefunden hat:

- **Weltprozess** (allgemein, u134): Stand der Partner in der anderen Welt still, bekam der Weltprozess des Hosts keinen Auftrag, und das Zuschau-Fenster blieb leer. Schon eine kurze Paketpause setzte die Wartezeit zurück.
- **Othello** (u134): Weltprozess und Testinstanz legen den Spielstand im selben Ordner ab wie das Hauptspiel (`SAVES_OTHELLOZ`, bisher fest `Saves`). Der Testlauf prüft getrennte Welten jetzt auch unter Othello.
- **Gothic 1, Othello** (u134): Der Merker „Mine schon betreten“ gehört zum Weltstand des PCs und wird nicht mehr abgeglichen.

## 0.3.0 – Coop-Inventar, gemeinsame Missions-Gegenstände, Gilden je Held (Gothic I u106, Gothic II u106, Othello u106; nur Tag `v0.3.0`, kein eigenes Release)

**Neu**

- **Coop-Inventar** (u97–u103): eine gemeinsame Truhe. Die Taste (Vorgabe I, Einstellungen › COOP) öffnet sie direkt, ohne Bewegung;
  beide können sie gleichzeitig offen haben. Der Inhalt liegt beim Host und im Spielstand, Gegenstände daraus zählen bei Quests.
- **Gemeinsame Missions-Gegenstände** (u96, u104, u106): bekommt einer einen, hat ihn der Partner auch; gibt einer ihn ab, verliert ihn
  der Partner. Im Namen steht „(gemeinsam)“, die Beschreibung erklärt es.
- **Gilden je Held** (u85): Gilden-Dialoge teilen nur Helden derselben Gilde (samt Belohnung); die Aufnahme gilt beim Partner nie als erzählt.
- **Auto-Sicherung** auch alle 3 Minuten (u105).
- **Weltwechsel unter Othello** (u64, nur Othello): Othello-Coop v107–v114 nachgezogen – GT lädt selbst, wenn Union den Wechsel nicht
  meldet; drei Menü-Haken unter Othello aus.

**Korrekturen** (allgemein)

- **Abstürze und Hänger:** Absturz beim Beenden über das Esc-Menü (u79); gelöschte Gegenstände in NPC-Slots und mitten beim Ablegen
  (u88–u90); gelöschtes Schwert in der Hand nach der Übernahme eines Spielstands (u91, u93); ein hängender Dialog blockiert das Laden nicht
  mehr (u100); nach dem Tod an der Weltgrenze von Gothic 2 lädt der Coop über das Hauptmenü, statt ewig zu warten (u91).
- **Spieluhr:** der Host schickt die Uhrzeit alle 10 statt 60 s (u69); die Uhr des Clients und des Weltprozesses geht jede Sekunde genau wie
  beim Host, Tagesabläufe stellen sich bei großen Sprüngen neu ein (u83).
- **NPCs beim Client:** kleine Abweichungen weich nachziehen statt springen, weite Sprünge sauber setzen, damit niemand hängen bleibt (u72, u74).
- **Gegenstände und Truhen:** verspätete Entfernungen werden nachgeholt, keine Geistergegenstände bis zum nächsten Laden (u70); Welt-Truhen
  nehmen Gegenstände sauber heraus (u98).
- **Story-Videos:** Überspringen geht auch bei 10-stelligen Nummern (u87).

## 0.2.0 – erste Veröffentlichung als Gothic-I-II-Universal-Coop (Gothic I u62, Gothic II u62, Othello u62)

- Coop für Gothic 1 (1.08k), Gothic 2 Die Nacht des Raben (2.6) und Gothic 1 mit Othello 4.x aus einem gemeinsamen Quelltext:
  Spieler und NPCs, Kampf mit Treffer-Hoheit und Kampf-Übergabe, Dialoge, Story und Tagebuch (Quest-EP für beide),
  Truhen, Gegenstände und Plündern, gemeinsame Spielstände mit Vorschau und Sicherung beim Partner, getrennte Welten mit
  Weltprozess und Zuschau-Fenster, gemeinsames neues Spiel, Story-Videos für beide überspringen, COOP-Menü mit Figur und
  3D-Vorschau, Coop über Steam, COOP-Schwierigkeit, Teleport-Rune zum Mitspieler, Coop-Testlauf (Shift+F11).
- u62: Union 1.0m liegt als Dateien bei (Installer und ZIPs für Gothic 1/2); der Installer installiert Union mit, wenn es
  fehlt, legt `system\dinput.dll` von SystemPack nach `CoopBackup` und nimmt beides beim Rückgängig wieder heraus (bis u61
  startete er das Union-Setup). Neu: ZIP für Othello.
- u56–u61: am Coop unverändert gegenüber u55.
