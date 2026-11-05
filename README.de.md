# Gothic-I-II-Universal-Coop

**Koop-Mehrspieler für Gothic 1 und Gothic 2 Die Nacht des Raben samt Mods – unterstützt sind Othello (Gothic 1) und Legend of
Ahssun (Gothic 2).** Zwei oder mehr Spieler erleben
dieselbe Geschichte gemeinsam: Welt, NPCs, Kämpfe, Dialoge, Tagebuch, Truhen und Spielstände werden zwischen den PCs
abgeglichen. Ein Union-Plugin auf Grundlage von [GothicCoop / Gothic Together](https://gitlab.com/gothiccoop/gothiccoop)
(AGPL-3.0), Nachfolger des [Othello-Coops](https://github.com/corshaegar/Othello-Coop).

> *English version: [README.md](README.md). Menus and the installer are available in German and English.*

**Inhalt:** [Versionen](#versionen) · [Was funktioniert](#was-funktioniert) · [Teststand](#teststand) ·
[Installation](#installation) · [Spielen](#spielen) · [Einstellungen](#einstellungen) · [Probleme](#probleme-lösen) ·
[Wie es funktioniert](#wie-es-funktioniert) · [Was noch kommt](#was-noch-kommt) · [Bauen](#selbst-bauen) · [Lizenz](#lizenz-und-herkunft)

## Versionen

Projektversion **0.6.0** – je Spiel/Mod die Coop-Fassung (sie zählt nur hoch, wenn sich an diesem Spiel etwas ändert):

| Spiel / Mod | Coop-Fassung |
|---|---|
| Gothic I | u197 |
| Gothic II | u197 |
| Othello | u197 |
| Legend of Ahssun | u197 |

**Beide PCs brauchen dieselbe Coop-Fassung** – beim Verbinden wird sie verglichen. Sie steht im Hauptmenü unten
(„GothicCoop uNN“). Änderungen je Version: [`CHANGELOG.md`](CHANGELOG.md).

## Was funktioniert

**Spiele und Mods**
- **Ein Coop für alle:** je Engine eine DLL mit den Skripten aller Varianten. Wechselt die Mod (GothicStarter, Othello dazu oder
  weg), stellt sich der Coop beim Start selbst um (Log „Skript-Weiche“) – neu installieren muss man nicht.
- **Unterstützte Mods:** Othello (Gothic 1) und Legend of Ahssun (Gothic 2) mit eigenem Profil. Andere Mods, die mit
  `-game:<ini>` starten, erkennt der Coop; was sie brauchen, kommt aus einer Profil-Datei (siehe [Einstellungen](#einstellungen)).

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
- **Gilden je Held:** Gilden-Dialoge teilen nur Helden derselben Gilde (samt Belohnung). Der Gildenfortschritt (Werte,
  Gespräche, Tagebuch-Themen, Haltungen von NPCs aus Gilden-Quests und Beitritten) bleibt beim eigenen Helden, auch beim
  gemeinsamen Laden.
- Was ein Dialog an NPCs ändert (Tagesablauf, Haltung, Schutz, Gilde, Inventar wie die Händlerware) gilt für beide; ebenso
  Teleport, Skript-Tod und Entfernen eines NPCs, auch wenn ein Dialog beim Client sie auslöst.
- Skript-Ereignisse (Gegenstände aus Skripten, Spawns, Uhrzeit und Spieltag) kommen vom Host.
- **Spielende** (Gothic 2): Nach dem Abspann stehen beide im Hauptmenü.

**Welt und Gegenstände**
- Truhen, Gegenstände am Boden und Plündern – der Host führt den Bestand, nichts verdoppelt sich.
- **Gegenstände der NPCs:** Was NPCs fallen lassen oder werfen (Waffen bei Tod oder Bewusstlosigkeit, Werkzeuge), liegt bei
  beiden an derselben Stelle. Pfeile und Bolzen bleiben je PC (sie stecken im Ziel oder verschwinden nach Sekunden).
- **Coop-Inventar:** eine gemeinsame Truhe, die Taste (Vorgabe I, Einstellungen › COOP) öffnet sie direkt und ohne Bewegung. Beide können
  sie gleichzeitig offen haben; der Inhalt liegt beim Host und im Spielstand, Gegenstände daraus zählen bei Quests.
- **Gemeinsame Missions-Gegenstände:** bekommt einer einen, hat ihn der Partner auch; gibt einer ihn ab, verliert ihn der Partner.
  Im Namen steht „(gemeinsam)“.
- **Mechanismen:** Türen, Hebel, Steinplatten und Portale, die ein Held benutzt, gelten für beide, samt den Skripten und Kamerafahrten dahinter; ziehen beide zugleich am selben Hebel, zählt das einmal. Auch bewegliche Teile, die selbst auf Berührung reagieren (Zielsteine im Schläfertempel, Vinos Kellertür), bewegen sich bei beiden (im automatischen Testlauf geprüft). NPCs benutzen Türen je PC.
- **Auslöser**, die ein Held, ein NPC oder ein Pfeil oder Zauber berührt (Tore, Fallen, die Brücke auf Irdorath, der
  Schläfertempel in Gothic 1), lösen bei beiden aus. Im automatischen Testlauf geprüft, im echten Spielablauf noch nicht gesehen.
- **Getrennte Welten:** jeder kann in einer anderen Welt sein (Khorinis, Minental, Jharkendar, Irdorath bzw. die Welten von
  Gothic 1). **Gemeinsamer Weltwechsel** auf Wunsch (Übergang bestätigen, der Partner kommt nach).
- **Weltprozess und Zuschau-Fenster:** ist der Partner in einer anderen Welt, zeigt ein unsichtbares zweites Gothic dessen Welt;
  das Zuschau-Fenster (Shift+F5) zeigt ihn dort (braucht den Coop-Renderer).
- **Teleport-Rune** zum Mitspieler (auch über Welten), Mitspieler-Markierung (Name über dem Kopf, Pfeil am Bildrand, auf der Karte).

**Spielstände**
- **Gemeinsames Speichern und Laden:** der Host wählt, beide laden denselben Stand, jeder mit seinem Helden; fehlende Weltdateien
  werden geprüft übertragen. Vorschau beider PCs in der Liste. Wer einen Stand mit der Welt des Partners allein lädt (etwa nach
  einem Neustart), bekommt ebenfalls seinen eigenen Helden.
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
| **Gothic 2 Die Nacht des Raben** (2.6) | Hauptspiel der Entwicklung. Automatischer Testlauf auf einem PC (Host und Client) mit Coop u197 (Entwicklungs-DLL derselben Fassung): Gilden, Coop-Inventar, Truhen, Türen, Mechanismen, Auslöser, Gegenstände der NPCs, Speichern und Laden Host 109 OK, Client 107 OK, 0 Fehler; getrennte Welten und Weltprozess zuletzt mit u171 Host 57 OK, Client 52 OK, 0 Fehler. Lange automatische Läufe mit Prüfrunden (Vergleich beider Seiten), inzwischen bis zum Spielende (mit erhöhtem Leben, also kein echter Test der Kämpfe) – meist ohne Abweichung, gelegentlich einzelne Abweichungen. Das Coop-Inventar ist vor allem automatisch geprüft; ein Spieltest mit zwei Personen über das echte Netz steht noch aus. |
| **Gothic 1** (1.08k, Steam/GOG) | Derselbe automatische Testlauf, zuletzt vollständig mit u166: Gilden, Coop-Inventar, Türen, Gegenstände der NPCs, Speichern und Laden Host 77 OK, Client 73 OK, 0 Fehler (25 bzw. 29 Schritte übersprungen, am Test-Spielstand stehen keine Truhe und kein passender Hebel oder Auslöser); getrennte Welten und Weltprozess Host 53 OK, Client 48 OK, 0 Fehler (4 bzw. 9 übersprungen). Der Testlauf mit genau der Release-DLL 0.6.0 folgt. Hebel und Auslöser sind in Gothic 1 damit noch nicht im Spiel geprüft. Von Hand zuletzt mit u44 gespielt; die Neuerungen seitdem (Coop-Inventar, Missions-Gegenstände, Mechanismen, Auslöser) noch nicht von Hand und nicht über das echte Netz. |
| **Othello** 4.x (Gothic 1) | Derselbe automatische Testlauf mit genau der Release-DLL, gestartet mit den Gothic-1-Skripten (die Skript-Weiche stellte beim Start auf Othello um): Gilden, Coop-Inventar, Truhen, Türen, Mechanismen, Auslöser, Gegenstände der NPCs, Speichern und Laden Host 99 OK, Client 97 OK, je 4 Fehler – 3 an einem Tor (siehe unten), 1 im Testwerkzeug selbst (ein Diagnose-Schritt sucht den Apfel unter seinem Gothic-2-Namen); getrennte Welten und Weltprozess zuletzt mit 0.5.0 (u160) Host 53 OK, Client 48 OK, 0 Fehler. Von Hand zuletzt mit dem Othello-Coop v106 (= Othello-Coop 0.1.2) gespielt; seitdem sind die allgemeinen Änderungen dazugekommen. |
| **Legend of Ahssun** 1.2.0 (Gothic 2) | Eigenes Profil in der Gothic-2-DLL – derselbe Coop wie für Gothic 2, kein eigenes Paket. Voller automatischer Testlauf mit einem Spielstand der Mod (Entwicklungs-DLL derselben Fassung u197): Host 237 OK, Client 231 OK, je 1 Fehler (tote Fleischwanze, siehe unten); gemeinsames Laden, Weltwechsel, Weltprozess und Zuschau-Fenster darin bestanden. Dazu automatische Läufe in der Mod seit u170. |
| **Andere Mods** | Gothic 1/2 mit anderen Mods lassen sich installieren; der Installer zeigt die Mods im Ordner an. Es läuft das Fundament des Coops; was eine Mod an Anpassungen braucht, kommt über eine Profil-Datei (siehe [Einstellungen](#einstellungen)). Nicht gezielt geprüft. |

Bekannte Grenzen:
- Der **Weltprozess** braucht ein NTFS-Laufwerk (Verknüpfungen statt Kopien) und ein vorhandenes Speicherspiel.
- Zuschau-Fenster und 3D-Vorschau brauchen den **Coop-Renderer** (GD3D11 v17.9.7 + Coop-Patch) und einen Prozessor mit AVX2.
- „Über IP beitreten“ braucht beim Host einen offenen **UDP-Port 9955** (Router-Weiterleitung/Firewall) – oder Steam.
- **Othello, Tore mit Hebel:** Wird ein Tor bewegt, während es noch fährt (Hebel ein zweites Mal umgelegt, beide zugleich am Hebel, ein Pfeil an seinem Auslöser), steht es im Testlauf danach auf den beiden PCs verschieden (`PSI_DOOR_TEMPLE_NYRAS_SECRECT_2`, 3 Schritte je Seite).
- **Gothic 2, alte Spielstände:** Stände aus Fassungen vor u140 können doppelte NPCs enthalten. Die bleiben; neue entstehen nicht mehr.
- **Legend of Ahssun:** Im Testlauf fehlt nach dem Betreten einer Welt beim Partner eine tote Fleischwanze, die der Host
  gerade getötet hat (ein Schritt je Seite, offen).
- **Skript-Ereignisse beim Client** (Teleport, Skript-Tod, Entfernen eines NPCs) gelten beim Host nur, wenn er in derselben Welt ist.
- Bitte jeden Absturz und jede Abweichung melden: Protokoll `system\GothicCoop.log` beider PCs (unter Othello
  `system\GothicCoopOthello.log`).

## Installation

**Voraussetzungen:** Windows 10/11; Gothic 1 in Fassung 1.08k (Steam/GOG), Gothic 2 Die Nacht des Raben 2.6 (z. B. Gold-Edition
mit Patch 2.6) oder Gothic 1 mit Othello 4.x. Union 1.0m bringen Installer und ZIPs für Gothic 1/2 mit; Othello hat Union dabei.

### Mit dem Installer (empfohlen)

1. Unter [Releases](../../releases) `Gothic-I-II-Universal-Coop-0.6.0-Setup.exe` laden und starten.
2. Den **Spielordner** wählen (den Ordner, in dem `system` liegt; Steam- und GOG-Ordner werden gesucht). Ein Spiel wählt man
   nicht: Der Installer erkennt am Inhalt Gothic 1, Othello, Gothic 2 und die Mods im Ordner (z. B. Legend of Ahssun) und zeigt,
   welche unterstützt sind. Er installiert den Coop der Engine mit den Skripten aller Varianten – wechselt die Mod später
   (GothicStarter, Othello dazu oder weg), stellt sich der Coop beim Start selbst um. In einen falschen Ordner installiert er nichts.
3. „Installieren“. Er schreibt den Coop, bringt GD3D11 v17.9.7 mit (falls es fehlt) und legt den Coop-Renderer darüber
   (abwählbar); fehlt Union bei Gothic 1/2, fragt er und installiert Union 1.0m mit.
4. Alles, was er ersetzt, sichert er vorher in `CoopBackup\<Zeit>\`. **„Letzte Installation rückgängig“** im selben Installer
   stellt den alten Stand wieder her (auch Union, falls er es installiert hat). Protokoll: `CoopLogs\Installer\`.

Ohne Fenster: `Setup.exe /still /ordner:"<Spielordner>" [/spiel:g1|g2|othello|loa als Prüfung] [/union] [/ohnerenderer] [/rueckgaengig]`.

### Mit der ZIP

1. `Gothic-I-II-Universal-Coop-0.6.0.zip` laden – eine ZIP für alle Spiele.
2. Den Inhalt **eines** ihrer Ordner in den Spielordner kopieren (dort, wo `system` liegt), vorhandene Dateien ersetzen:
   `Gothic 1 + Othello` für Gothic 1 (auch mit Othello), `Gothic 2 + Mods` für Gothic 2 (auch mit Legend of Ahssun und anderen Mods).
3. `Union 1.0m (nur wenn Union fehlt)` nur, wenn im Spielordner noch kein Union ist (`system\Shw32.dll` fehlt) – nie bei Othello.
   Danach `system\dinput.dll` entfernen (der Lader von SystemPack – Union lädt SystemPack selbst).
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
- **Mod-Profil** (ab u194): Anpassungen für eine Mod ohne neue DLL in `system\GothicCoopMod-<Ini>.json` (`<Ini>` = Spiel-Ini
  der Mod ohne „.ini“, z. B. `GothicCoopMod-LoA.json` beim Start mit `-game:LoA.ini`; ohne `-game`: `system\GothicCoopMod.json`).
  Darin: `perPcVars` (Skript-Ganzzahlen, die je PC bleiben, `NAME_*` für eine Vorsilbe), `heroVars` (ab u197: Werte, die mit dem
  eigenen Helden reisen – beim Verbinden und Laden bleiben die eigenen, etwa Talente oder Rezepte einer Mod), `configDefaults` (Vorgaben für die
  `coop…`-Schalter, solange `GothicCoopConfig.json` sie nicht setzt), `notes` (Hinweise im Log), `name`, `engine`. Kommentare sind
  erlaubt. Eine Vorlage mit Erklärung liegt jedem Paket bei (`system\GothicCoopMod.vorlage.json`). Beide PCs brauchen dieselbe
  Datei; das Log nennt beim Start, was gelesen wurde.

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
  Dieselbe Quelle wird je Engine gebaut; Othello erkennt die Gothic-1-DLL selbst und schaltet ihre Othello-Anpassungen ein,
  ebenso Legend of Ahssun die Gothic-2-DLL. Andere Mods bekommen ihre Anpassungen über eine Profil-Datei – eine eigene DLL je Mod
  gibt es nicht. Die Skripte jeder Variante liegen unter `system\GothicCoop\Skripte\<g1|othello|g2>`; beim Start legt die DLL den
  passenden Satz nach `system\Autorun`, bevor zParserExtender ihn lädt (Skript-Weiche).
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
- Weitere Mods mit Profil-Dateien aufnehmen und prüfen (eingebaut bisher: Othello, Legend of Ahssun).
- Legend of Ahssun: die letzte Abweichung im Testlauf (tote Fleischwanze nach dem Betreten einer Welt).
- Union für Gothic 1 vollständig mitliefern (Unions Setup schreibt dort zusätzlich `binkw32.dll`/`Mss32.dll`).
- Weitere Abgleich-Feinheiten aus den langen Testläufen.

## Selbst bauen

Visual Studio 2019 Build Tools (C++, Win32), Python 3:

```
powershell -File Werkzeuge\bau.ps1 -Config "G2A MD Release" -Copy Bau\Gothic2Coop.dll
powershell -File Werkzeuge\bau.ps1 -Config "G1 MD Release"  -Copy Bau\Gothic1Coop.dll
```

Für ZIPs und Installer (`Werkzeuge\makezip_g1.py`, `makezip_g2.py`, daraus die Universal-ZIP mit `makezip_universal.py`;
`installer_bau.py`, Test `installer_test.py`) zusätzlich in
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
