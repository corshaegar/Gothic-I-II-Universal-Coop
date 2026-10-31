# Gothic-I-II-Universal-Coop

**Koop-Mehrspieler für Gothic 1, Gothic 2 Die Nacht des Raben und Gothic 1 mit Othello.** Zwei oder mehr Spieler erleben
dieselbe Geschichte gemeinsam: Welt, NPCs, Kämpfe, Dialoge, Tagebuch, Truhen und Spielstände werden zwischen den PCs
abgeglichen. Ein Union-Plugin auf Grundlage von [GothicCoop / Gothic Together](https://gitlab.com/gothiccoop/gothiccoop)
(AGPL-3.0), Nachfolger des [Othello-Coops](https://github.com/corshaegar/Othello-Coop).

> *English summary: co-op multiplayer for Gothic 1, Gothic 2 Night of the Raven and Gothic 1 with the Othello mod. Download the
> installer (recommended) or the ZIP for your game from the [releases](../../releases), choose the game folder, install. Both
> players need the same coop version. Host via "COOP › Host game", join via IP (UDP port 9955) or Steam. Documentation below is
> in German; menus are available in German and English.*

**Inhalt:** [Versionen](#versionen) · [Was funktioniert](#was-funktioniert) · [Teststand](#teststand) ·
[Installation](#installation) · [Spielen](#spielen) · [Einstellungen](#einstellungen) · [Probleme](#probleme-lösen) ·
[Wie es funktioniert](#wie-es-funktioniert) · [Was noch kommt](#was-noch-kommt) · [Bauen](#selbst-bauen) · [Lizenz](#lizenz-und-herkunft)

## Versionen

Projektversion **0.4.0** – je Spiel/Mod die Coop-Fassung (sie zählt nur hoch, wenn sich an diesem Spiel etwas ändert):

| Spiel / Mod | Coop-Fassung |
|---|---|
| Gothic I | u142 |
| Gothic II | u142 |
| Othello | u142 |

**Beide PCs brauchen dieselbe Coop-Fassung** – beim Verbinden wird sie verglichen. Sie steht im Hauptmenü unten
(„GothicCoop uNN“). Änderungen je Version: [`CHANGELOG.md`](CHANGELOG.md).

## Was funktioniert

**Grundabgleich**
- Spieler und NPCs in der Nähe: Lage, Bewegung, Animationen, Waffen, Leben und Tod.
- Ein **Host** führt die Welt; Clients schicken ihre Handlungen an ihn, er verteilt das Ergebnis. Jeder spielt seinen eigenen
  Helden (Ausrüstung, Erfahrung, Gilde, Talente bleiben beim eigenen Helden).
- **Fern-Abgleich:** der Partner und die NPCs um ihn bleiben auch dann richtig, wenn er weit weg ist (Tode, Erfahrung).

**Kampf**
- Treffer auf den eigenen Helden entscheidet der eigene PC (kein „Schaden durch Verzögerung“).
- **Kampf-Übergabe:** NPCs, die gegen einen Client kämpfen, rechnet dessen PC – flüssiger Nahkampf auch beim Client.
- **COOP-Schwierigkeit:** Schaden, Leben der Gegner und Kampf-Erfahrung einstellbar; der Host gibt vor.

**Geschichte**
- **Dialoge:** einer spricht mit einem NPC, der andere hört zu; Fortschritt, Tagebuch, Kapitel und Quest-Erfahrung gelten
  für beide. Story-Videos laufen bei beiden; überspringt einer, endet es bei beiden.
- **Gilden je Held:** Gilden-Dialoge teilen nur Helden derselben Gilde (samt Belohnung).
- Was ein Dialog an NPCs ändert (Tagesablauf, Haltung, Schutz, Gilde) gilt für beide.
- Skript-Ereignisse (Gegenstände aus Skripten, Spawns, Uhrzeit und Spieltag) kommen vom Host.

**Welt und Gegenstände**
- Truhen, Gegenstände am Boden und Plündern – der Host führt den Bestand, nichts verdoppelt sich.
- **Coop-Inventar:** eine gemeinsame Truhe, die Taste (Vorgabe I, Einstellungen › COOP) öffnet sie direkt und ohne Bewegung. Beide können
  sie gleichzeitig offen haben; der Inhalt liegt beim Host und im Spielstand, Gegenstände daraus zählen bei Quests.
- **Gemeinsame Missions-Gegenstände:** bekommt einer einen, hat ihn der Partner auch; gibt einer ihn ab, verliert ihn der Partner.
  Im Namen steht „(gemeinsam)“.
- **Mechanismen:** Türen, Hebel, Steinplatten und Portale gelten für beide, samt den Skripten und Kamerafahrten dahinter.
- **Getrennte Welten:** jeder kann in einer anderen Welt sein (Khorinis, Minental, Jharkendar, Irdorath bzw. die Welten von
  Gothic 1). **Gemeinsamer Weltwechsel** auf Wunsch (Übergang bestätigen, der Partner kommt nach).
- **Weltprozess und Zuschau-Fenster:** ist der Partner in einer anderen Welt, zeigt ein unsichtbares zweites Gothic dessen Welt;
  das Zuschau-Fenster (Shift+F5) zeigt ihn dort (braucht den Coop-Renderer).
- **Teleport-Rune** zum Mitspieler (auch über Welten), Mitspieler-Markierung (Name über dem Kopf, Pfeil am Bildrand, auf der Karte).

**Spielstände**
- **Gemeinsames Speichern und Laden:** der Host wählt, beide laden denselben Stand, jeder mit seinem Helden; fehlende Weltdateien
  werden geprüft übertragen. Vorschau beider PCs in der Liste.
- **Sicherung beim Partner:** jeder bekommt den Stand des anderen als Kopie; fehlt einem sein Stand (Saves verloren), holt er
  ihn vom Partner zurück.
- **Gemeinsames neues Spiel:** Intro für beide, Start nebeneinander.
- **Auto-Sicherung** gemeinsam, auf Wunsch alle 3 Minuten.

**Bedienung**
- **COOP-Menü** im Hauptmenü: Figur, Aussehen (auch „wie ein NPC“) mit 3D-Vorschau, Verbinden über IP oder **Steam**
  (Freundesliste, Einladungen), Spielstand-Liste. Maus in allen Gothic-Menüs. Deutsch und Englisch.
- Im Esc-Menü „Spielstand wählen“ und „Hauptmenü“ (zurück zum Startbildschirm ohne Neustart).
- **Coop-Testlauf** (Shift+F11, nur auf einem Test-Spielstand): prüft beide Seiten automatisch und meldet OK/FEHLER je Bereich.

## Teststand

| | Stand |
|---|---|
| **Gothic 2 Die Nacht des Raben** (2.6) | Hauptspiel der Entwicklung. Automatischer Testlauf auf einem PC (Host und Client): Gilden, Coop-Inventar, Truhen und Mechanismen 48/48 auf beiden Seiten; getrennte Welten und Weltprozess Host 55/55, Client 52 OK, 0 Fehler (3 Schritte nur am Host). Lange automatische Läufe mit Prüfrunden (Vergleich beider Seiten), inzwischen bis in Kapitel 5 samt Drachenkampf – meist ohne Abweichung, gelegentlich einzelne Abweichungen. Das Coop-Inventar ist vor allem automatisch geprüft; ein Spieltest mit zwei Personen über das echte Netz steht noch aus. |
| **Gothic 1** (1.08k, Steam/GOG) | Derselbe automatische Testlauf: Gilden und Coop-Inventar je 37 OK, 0 Fehler (11 Schritte übersprungen, am Test-Spielstand stehen keine Truhe und kein passender Hebel); getrennte Welten und Weltprozess Host 51 OK, Client 48 OK, 0 Fehler (4 bzw. 7 übersprungen). Die Mechanismen sind in Gothic 1 damit noch nicht im Spiel geprüft. Von Hand zuletzt mit u44 gespielt; die Neuerungen seitdem (Coop-Inventar, Missions-Gegenstände, Mechanismen) noch nicht von Hand und nicht über das echte Netz. |
| **Othello** 4.x (Gothic 1) | Derselbe automatische Testlauf: Gilden, Coop-Inventar, Truhen und Mechanismen je Seite 38–46 OK, bis auf einen Schritt ohne Fehler (siehe unten); getrennte Welten und Weltprozess Host 51 OK, Client 48 OK, 0 Fehler. Von Hand zuletzt mit dem Othello-Coop v106 (= Othello-Coop 0.1.2) gespielt; seitdem sind die allgemeinen Änderungen dazugekommen. |
| **Mods** | Gothic 1/2 mit anderen Mods lassen sich installieren (der Installer erlaubt es), sind aber nicht gezielt geprüft. |

Bekannte Grenzen:
- Der **Weltprozess** braucht ein NTFS-Laufwerk (Verknüpfungen statt Kopien) und ein vorhandenes Speicherspiel.
- Zuschau-Fenster und 3D-Vorschau brauchen den **Coop-Renderer** (GD3D11 v17.9.7 + Coop-Patch) und einen Prozessor mit AVX2.
- „Über IP beitreten“ braucht beim Host einen offenen **UDP-Port 9955** (Router-Weiterleitung/Firewall) – oder Steam.
- **Othello, Tore mit Hebel:** Wird der Hebel ein zweites Mal umgelegt, während das Tor noch aufgeht, kehrte es beim Partner im Testlauf meist nicht um (`PSI_DOOR_TEMPLE_NYRAS_SECRECT_*`, 3 von 4 Durchgängen). Das Tor kann dann auf den beiden PCs verschieden stehen.
- **Gothic 2, alte Spielstände:** Stände aus Fassungen vor u140 können doppelte NPCs enthalten. Die bleiben; neue entstehen nicht mehr.
- Bitte jeden Absturz und jede Abweichung melden: Protokoll `system\GothicCoop.log` beider PCs.

## Installation

**Voraussetzungen:** Windows 10/11; Gothic 1 in Fassung 1.08k (Steam/GOG), Gothic 2 Die Nacht des Raben 2.6 (z. B. Gold-Edition
mit Patch 2.6) oder Gothic 1 mit Othello 4.x. Union 1.0m bringen Installer und ZIPs für Gothic 1/2 mit; Othello hat Union dabei.

### Mit dem Installer (empfohlen)

1. Unter [Releases](../../releases) `Gothic-I-II-Universal-Coop-0.4.0-Setup.exe` laden und starten.
2. Spiel wählen (Gothic 1, Gothic 2, Othello) und den **Spielordner** (den Ordner, in dem `system` liegt; Steam- und GOG-Ordner
   werden gesucht). Der Installer prüft am Inhalt, dass es wirklich dieses Spiel ist, und installiert in einen falschen Ordner nichts.
3. „Installieren“. Er schreibt den Coop, bringt GD3D11 v17.9.7 mit (falls es fehlt) und legt den Coop-Renderer darüber
   (abwählbar); fehlt Union bei Gothic 1/2, fragt er und installiert Union 1.0m mit.
4. Alles, was er ersetzt, sichert er vorher in `CoopBackup\<Zeit>\`. **„Letzte Installation rückgängig“** im selben Installer
   stellt den alten Stand wieder her (auch Union, falls er es installiert hat). Protokoll: `CoopLogs\Installer\`.

Ohne Fenster: `Setup.exe /still /spiel:g1|g2|othello /ordner:"<Spielordner>" [/union] [/ohnerenderer] [/rueckgaengig]`.

### Mit der ZIP

1. Die ZIP des Spiels laden (`…-Gothic-I-uNN.zip`, `…-Gothic-II-uNN.zip` oder `…-Othello-uNN.zip`).
2. In den Spielordner entpacken (dort, wo `system` liegt), vorhandene Dateien ersetzen.
3. Gothic 1/2: die ZIP enthält Union 1.0m. War Union vorher **nicht** installiert, danach `system\dinput.dll` entfernen (der Lader
   von SystemPack – Union lädt SystemPack selbst).
4. Der Coop-Renderer in der ZIP wirkt nur, wenn GD3D11 v17.9.7 installiert ist (sonst bringt es der Installer mit).

### Aktualisieren

Neue Fassung einfach darüber installieren (Installer oder ZIP). Eigene Einstellungen (`GothicCoopConfig.json`, `Gothic.ini`)
behält der Installer.

## Spielen

1. Beide starten Gothic wie gewohnt; im Hauptmenü **COOP**.
2. Figur wählen (Name, Aussehen), dann „Weiter: Verbinden“.
3. Der Host: **„Spiel hosten“**. Der andere: **„Über IP beitreten“** (IP des Hosts, Port 9955) oder über **Steam** (beide mit Steam
   angemeldet; Einladung über die Freundesliste).
4. Spielstand laden oder **gemeinsames neues Spiel** – der Host wählt.

Tasten: **Shift+F5** Zuschau-Fenster beim Mitspieler · **Shift+F11** Coop-Testlauf (nur auf einem Test-Spielstand – er ändert
Uhrzeit, Tagebuch und legt eine Sicherung an) · **Shift+F12** Weltprozess aus/an.

## Einstellungen

- Im Spiel unter **Einstellungen › COOP** (Abgleich, Weltwechsel gemeinsam/getrennt, Kamera, Schwierigkeit, Sprache); gespeichert in
  `Gothic.ini` Abschnitt `[GOTHICCOOP]`.
- Grundeinstellungen in `system\GothicCoopConfig.json` (Name, Adresse, Port, Tasten). **Ohne diese Datei startet der Coop nicht.**
  Einige Schalter dort: `coopSeparateWorlds` (getrennte Welten), `coopWorldProcessAuto` (Weltprozess vorstarten),
  `coopEnterTakeState` (beim Betreten einer Welt den Stand des Partners übernehmen).

## Probleme lösen

| Problem | Lösung |
|---|---|
| Gothic beendet sich mit „No config file found“ | `system\GothicCoopConfig.json` fehlt – aus der ZIP/dem Installer neu einspielen |
| Coop-Menü fehlt | Union fehlt oder ist nicht aktiv (`system\Shw32.dll`, `system\Union.patch`, `Data\Union.vdf`); bei SystemPack `system\dinput.dll` entfernen |
| Verbinden scheitert | gleiche Coop-Fassung auf beiden PCs? UDP-Port 9955 beim Host offen? Sonst Steam nutzen |
| Kein Zuschau-Fenster / keine 3D-Vorschau | Coop-Renderer und GD3D11 v17.9.7 nötig (Installer), Prozessor mit AVX2 |
| Absturz oder Abweichung | `system\GothicCoop.log` beider PCs (ältere Läufe in `CoopLogs\`) mit Beschreibung melden |

## Wie es funktioniert

- **Union-Plugin:** `system\Autorun\Gothic1Coop.dll` bzw. `Gothic2Coop.dll` hängt sich in die Engine (Union-SDK, Haken auf
  Engine-Funktionen). Dazu Skripte (`COOP_MENU.D` für die Menüs, `COOP_ITEMS.D`, `PC_MULTIPLAYER_HERO.D`), die zParserExtender lädt.
  Dieselbe Quelle wird je Engine gebaut; Othello erkennt die Gothic-1-DLL selbst und schaltet ihre Othello-Anpassungen ein.
- **Netz:** ENet (UDP), Inhalte als JSON/BSON; Steam als Tunnel darunter. **Der Host führt:** Clients schicken Anfragen, der Host
  handelt und verteilt das Ergebnis mit dem Verursacher; NPCs werden über eindeutige Namen angesprochen. Ausnahme Kampf gegen einen
  Client (dessen PC rechnet) und Treffer auf den eigenen Helden.
- **Getrennte Welten:** jedes Paket trägt seine Welt; ohne Host führt ein Client die NPCs seiner Welt selbst. Der Weltprozess ist
  ein zweites Gothic aus einem verknüpften Spielordner, auf einem eigenen Windows-Desktop.
- **Spielstände:** jeder gemeinsame Stand hat eine Kennung, ein Manifest mit Prüfsummen und ein Archiv je Durchlauf
  (`Saves\coop_archiv\`); Partnerkopien als Sicherung.
- **Renderer:** GD3D11 v17.9.7 mit Coop-Patch (zweite Kamera für das Zuschau-Fenster, 3D-Vorschau, Bild des Weltprozesses) –
  die Patches liegen in [`Renderer/`](Renderer).
- Mehr: [`docs/MOD-UEBERBLICK.md`](docs/MOD-UEBERBLICK.md) (technischer Überblick), [`docs/COOP-FIXES.md`](docs/COOP-FIXES.md)
  (alle Korrekturen mit Herleitung), [`docs/PLAN-ALLGEMEINES-COOP.md`](docs/PLAN-ALLGEMEINES-COOP.md) (Weg vom Othello-Coop zum
  allgemeinen Coop), [`docs/UNION-DATEIEN.md`](docs/UNION-DATEIEN.md).

## Was noch kommt

- Coop-Inventar, gemeinsame Missions-Gegenstände und Mechanismen im Spieltest mit zwei Personen über das echte Netz prüfen.
- Gothic 1 und Othello von Hand und über das echte Netz prüfen (der automatische Testlauf läuft dort jetzt).
- Erkennung einzelner Mods mit eigenen Anpassungen (bisher: Gothic 1/2 mit Mods installierbar, aber nicht gezielt unterstützt).
- Union für Gothic 1 vollständig mitliefern (Unions Setup schreibt dort zusätzlich `binkw32.dll`/`Mss32.dll`).
- Weitere Abgleich-Feinheiten aus den langen Testläufen (z. B. fallen gelassene Waffen getöteter Gegner).

## Selbst bauen

Visual Studio 2019 Build Tools (C++, Win32), Python 3:

```
powershell -File Werkzeuge\bau.ps1 -Config "G2A MD Release" -Copy Bau\Gothic2Coop.dll
powershell -File Werkzeuge\bau.ps1 -Config "G1 MD Release"  -Copy Bau\Gothic1Coop.dll
```

Für ZIPs und Installer (`Werkzeuge\makezip_g1.py`, `makezip_g2.py`, `installer_bau.py`, Test `installer_test.py`) zusätzlich in
`Bau\eingaben\`: die Coop-Renderer (GD3D11 v17.9.7 mit den Patches aus `Renderer\`: Gothic 1 bis `coop8.patch`, Gothic 2 bis `coop9.patch`), `steam_api.dll` (Steamworks) und unter
`Bau\eingaben\Union\` die Dateien von Union 1.0m (geprüft gegen ihre Prüfsummen, siehe `docs\UNION-DATEIEN.md`); für den Installer
das Release-ZIP von GD3D11 v17.9.7. Die DLLs je Fassung liegen unter `Bau\uNN\`.

| Ordner | Inhalt |
|---|---|
| `Kern/` | gemeinsamer Quelltext aller Spiele (`GothicCoop.vcxproj`, Unity-Build über `Sources.h`), Union-SDK in `ZenGin/` |
| `Gothic I/`, `Gothic II/`, `Othello/` | Spielskripte (`Skripte/`), Steam-Kennung, Bilder, Anleitung je Spiel |
| `Installer/` | Quelltext und Texte des Installers |
| `Renderer/` | Coop-Patches für GD3D11 v17.9.7 |
| `Werkzeuge/` | Bauen, Pakete, Installer-Test |
| `Lizenzen/`, `docs/` | Lizenztexte und Herkunft, Dokumentation |

## Lizenz und Herkunft

GNU Affero General Public License 3.0 ([`LICENSE`](LICENSE)) wie das Original GothicCoop / Gothic Together – geänderte Fassung,
Änderungen seit 16.10.2026. Wer die Dateien weitergibt, gibt den Quelltext mit oder verweist auf dieses Repo. Fremdbibliotheken:
ENet (MIT), nlohmann/json (MIT). Der Coop-Renderer ist eine geänderte Fassung von GD3D11 (GPL-3.0). Gothic-, Othello- und
Union-Dateien gehören ihren Rechteinhabern; Union 1.0m (Gratt, WOG RU Community) liegt den Paketen bei, wie es Mods üblicherweise
tun. Keinerlei Gewährleistung.
