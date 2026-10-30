# Änderungen

Projektversion (Veröffentlichung) und je Spiel/Mod die Coop-Fassung `uNN`. Vorgänger: das Othello-Coop
(github.com/corshaegar/Othello-Coop, 0.1.0–0.1.2, nur Othello). Die Fassungen dazwischen (u1–u61) stehen im Entwicklungsverlauf;
hier beginnt der Verlauf mit u62.

## 0.3.0 – Coop-Inventar, gemeinsame Missions-Gegenstände, Gilden je Held (Gothic I u106, Gothic II u106, Othello u106)

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
