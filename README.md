# Gothic-I-II-Universal-Coop

**Co-op multiplayer for Gothic 1, Gothic 2 Night of the Raven and Gothic 1 with Othello.** Two or more players experience
the same story together: world, NPCs, fights, dialogues, diary, chests and saves are kept in sync between the PCs. A Union
plugin based on [GothicCoop / Gothic Together](https://gitlab.com/gothiccoop/gothiccoop) (AGPL-3.0), successor of the
[Othello Coop](https://github.com/corshaegar/Othello-Coop).

> *Deutsche Fassung: [README.md](README.md). The changelog and the technical documentation under `docs/` are in German; menus
> and the installer are available in German and English.*

**Contents:** [Versions](#versions) · [What works](#what-works) · [Test status](#test-status) ·
[Installation](#installation) · [Playing](#playing) · [Settings](#settings) · [Troubleshooting](#troubleshooting) ·
[How it works](#how-it-works) · [What's next](#whats-next) · [Building](#building-it-yourself) · [License](#license-and-origin)

## Versions

Project version **0.4.1** – for each game/mod the coop version (it only goes up when something changes for that game):

| Game / mod | Coop version |
|---|---|
| Gothic I | u143 |
| Gothic II | u143 |
| Othello | u143 |

**Both PCs need the same coop version** – it is compared when connecting. It is shown at the bottom of the main menu
("GothicCoop uNN"). Changes per version: [`CHANGELOG.md`](CHANGELOG.md) (in German).

## What works

**Basic sync**
- Players and nearby NPCs: position, movement, animations, weapons, health and death.
- A **host** runs the world; clients send their actions to it, and it distributes the result. Everyone plays their own
  hero (equipment, experience, guild and talents stay with your own hero).
- **Remote sync:** the partner and the NPCs around them stay correct even when they are far away (deaths, experience).

**Combat**
- Hits on your own hero are decided by your own PC (no "damage from lag").
- **Combat handover:** NPCs fighting a client are computed by that client's PC – smooth melee on the client too.
- **COOP difficulty:** damage, enemy health and combat experience are adjustable; the host sets them.

**Story**
- **Dialogues:** one player talks to an NPC, the other listens; progress, diary, chapters and quest experience count for
  both. Story videos play for both; if one player skips, the video ends for both.
- **Guilds per hero:** guild dialogues are only shared between heroes of the same guild (including the reward).
- What a dialogue changes on NPCs (daily routine, attitude, protection, guild) applies to both.
- Script events (items from scripts, spawns, time of day and game day) come from the host.

**World and items**
- Chests, items on the ground and looting – the host keeps the record, nothing is duplicated.
- **Coop inventory:** a shared chest. The key (default I, Settings › COOP) opens it directly and without moving. Both players
  can have it open at the same time; its contents are kept on the host and in the save, and items from it count for quests.
- **Shared mission items:** if one player gets one, the partner has it too; if one hands it in, the partner loses it.
  The name says "(shared)".
- **Mechanisms:** doors, levers, stone plates and portals used by a hero apply to both, including the scripts and camera
  runs behind them. NPCs use doors per PC.
- **Separate worlds:** each player can be in a different world (Khorinis, Valley of Mines, Jharkendar, Irdorath, or the
  worlds of Gothic 1). **Joint world change** on request (confirm the transition, the partner follows).
- **World process and watch window:** if the partner is in another world, an invisible second Gothic shows that world;
  the watch window (Shift+F5) shows them there (needs the coop renderer).
- **Teleport rune** to the partner (also across worlds), partner marker (name above the head, arrow at the edge of the
  screen, on the map).

**Saves**
- **Shared saving and loading:** the host chooses, both load the same save, each with their own hero; missing world files
  are transferred and verified. Preview of both PCs in the list.
- **Backup at the partner:** everyone gets a copy of the other player's save; if a player's save is missing (lost saves),
  they get it back from the partner.
- **Shared new game:** intro for both, start side by side.
- **Auto-backup** for both, every 3 minutes if you like.

**Controls**
- **COOP menu** in the main menu: character, appearance (also "like an NPC") with 3D preview, connecting via IP or **Steam**
  (friends list, invitations), save list. Mouse in all Gothic menus. German and English.
- In the Esc menu "Choose save" and "Main menu" (back to the start screen without restarting).
- **Coop test run** (Shift+F11, only on a test save): checks both sides automatically and reports OK or failure per area.

## Test status

| | Status |
|---|---|
| **Gothic 2 Night of the Raven** (2.6) | Main game of development. Automated test run on one PC (host and client): guilds, coop inventory, chests, doors and mechanisms 52/52 on both sides; separate worlds and world process host 55/55, client 52 OK, 0 errors (3 steps host only). Long automated runs with check rounds (comparing both sides), by now into chapter 5 including a dragon fight – mostly without deviation, occasionally single deviations. The coop inventory has mainly been tested automatically; a play test with two people over a real network is still outstanding. |
| **Gothic 1** (1.08k, Steam/GOG) | The same automated test run: guilds, coop inventory and doors 41 OK each, 0 errors (11 steps skipped, the test save has no chest and no suitable lever); separate worlds and world process host 51 OK, client 48 OK, 0 errors (4 and 7 skipped). So levers have not been tested in play in Gothic 1 yet. Last played by hand with u44; the additions since then (coop inventory, mission items, mechanisms) have not been tested by hand or over a real network. |
| **Othello** 4.x (Gothic 1) | The same automated test run: guilds, coop inventory, chests, doors and mechanisms host 49 OK, client 42 OK, 1 error each (see below); separate worlds and world process host 51 OK, client 48 OK, 0 errors. Last played by hand with Othello Coop v106 (= Othello Coop 0.1.2); the general changes have been added since. |
| **Mods** | Gothic 1/2 with other mods can be installed (the installer allows it), but they are not specifically tested. |

Known limits:
- The **world process** needs an NTFS drive (links instead of copies) and an existing save.
- Watch window and 3D preview need the **coop renderer** (GD3D11 v17.9.7 + coop patch) and a CPU with AVX2.
- "Join via IP" needs an open **UDP port 9955** on the host (router forwarding/firewall) – or Steam.
- **Othello, gates with levers:** if the lever is flipped a second time while the gate is still opening, the gate mostly did
  not reverse on the partner's PC in the test run (`PSI_DOOR_TEMPLE_NYRAS_SECRECT_*`, 5 of 6 runs). The gate can then be in
  different positions on the two PCs.
- **Gothic 2, old saves:** saves from versions before u140 can contain duplicate NPCs. Those stay; no new ones appear.
- Please report every crash and every deviation, with the log `system\GothicCoop.log` from both PCs.

## Installation

**Requirements:** Windows 10/11; Gothic 1 version 1.08k (Steam/GOG), Gothic 2 Night of the Raven 2.6 (e.g. Gold Edition with
patch 2.6) or Gothic 1 with Othello 4.x. Installer and ZIPs bring Union 1.0m for Gothic 1/2; Othello includes Union.

### With the installer (recommended)

1. Download `Gothic-I-II-Universal-Coop-0.4.1-Setup.exe` from [Releases](../../releases) and start it.
2. Choose the game (Gothic 1, Gothic 2, Othello) and the **game folder** (the folder containing `system`; Steam and GOG folders
   are searched). The installer checks by content that it really is this game and installs nothing into a wrong folder.
3. "Install". It writes the coop, brings GD3D11 v17.9.7 (if missing) and puts the coop renderer on top (can be deselected);
   if Union is missing for Gothic 1/2, it asks and installs Union 1.0m as well.
4. Everything it replaces is backed up first in `CoopBackup\<time>\`. **"Undo last installation"** in the same installer
   restores the old state (including Union, if it installed it). Log: `CoopLogs\Installer\`.

Without a window: `Setup.exe /still /spiel:g1|g2|othello /ordner:"<game folder>" [/union] [/ohnerenderer] [/rueckgaengig]`.

### With the ZIP

1. Download the ZIP for your game (`…-Gothic-I-uNN.zip`, `…-Gothic-II-uNN.zip` or `…-Othello-uNN.zip`).
2. Unpack it into the game folder (where `system` is) and replace existing files.
3. Gothic 1/2: the ZIP contains Union 1.0m. If Union was **not** installed before, remove `system\dinput.dll` afterwards (the
   SystemPack loader – Union loads SystemPack itself).
4. The coop renderer in the ZIP only works if GD3D11 v17.9.7 is installed (otherwise the installer brings it).

### Updating

Simply install the new version on top (installer or ZIP). The installer keeps your own settings (`GothicCoopConfig.json`,
`Gothic.ini`).

## Playing

1. Both players start Gothic as usual; in the main menu choose **COOP**.
2. Choose a character (name, appearance), then "Next: connect".
3. The host: **"Host game"**. The other player: **"Join via IP"** (the host's IP, port 9955) or via **Steam** (both logged in to
   Steam; invitation through the friends list).
4. Load a save or start a **shared new game** – the host chooses.

Keys: **Shift+F5** watch window at the partner · **Shift+F11** coop test run (only on a test save – it changes the time of
day and the diary and creates a backup) · **Shift+F12** world process off/on.

## Settings

- In the game under **Settings › COOP** (sync, world change together/separate, camera, difficulty, language); stored in
  `Gothic.ini`, section `[GOTHICCOOP]`.
- Basic settings in `system\GothicCoopConfig.json` (name, address, port, keys). **Without this file the coop does not start.**
  Some switches there: `coopSeparateWorlds` (separate worlds), `coopWorldProcessAuto` (start the world process in advance),
  `coopEnterTakeState` (take over the partner's state when entering a world).

## Troubleshooting

| Problem | Solution |
|---|---|
| Gothic exits with "No config file found" | `system\GothicCoopConfig.json` is missing – restore it from the ZIP/installer |
| COOP menu is missing | Union is missing or not active (`system\Shw32.dll`, `system\Union.patch`, `Data\Union.vdf`); with SystemPack remove `system\dinput.dll` |
| Connecting fails | Same coop version on both PCs? UDP port 9955 open on the host? Otherwise use Steam |
| No watch window / no 3D preview | Coop renderer and GD3D11 v17.9.7 needed (installer), CPU with AVX2 |
| Crash or deviation | Report `system\GothicCoop.log` from both PCs (older runs in `CoopLogs\`) with a description |

## How it works

- **Union plugin:** `system\Autorun\Gothic1Coop.dll` or `Gothic2Coop.dll` hooks into the engine (Union SDK, hooks on engine
  functions). Plus scripts (`COOP_MENU.D` for the menus, `COOP_ITEMS.D`, `PC_MULTIPLAYER_HERO.D`) loaded by zParserExtender.
  The same source is built per engine; the Gothic 1 DLL detects Othello itself and switches on its Othello adjustments.
- **Network:** ENet (UDP), contents as JSON/BSON; Steam as a tunnel underneath. **The host leads:** clients send requests, the
  host acts and distributes the result together with who caused it; NPCs are addressed by unique names. Exceptions: combat
  against a client (that client's PC computes it) and hits on your own hero.
- **Separate worlds:** each packet carries its world; without a host, a client runs the NPCs of its world itself. The world
  process is a second Gothic from a linked game folder, on its own Windows desktop.
- **Saves:** each shared save has an ID, a manifest with checksums and an archive per playthrough (`Saves\coop_archiv\`);
  partner copies serve as backup.
- **Renderer:** GD3D11 v17.9.7 with coop patch (second camera for the watch window, 3D preview, image of the world process) –
  the patches are in [`Renderer/`](Renderer).
- More, in German: [`docs/MOD-UEBERBLICK.md`](docs/MOD-UEBERBLICK.md) (technical overview), [`docs/COOP-FIXES.md`](docs/COOP-FIXES.md)
  (all fixes with their derivation), [`docs/PLAN-ALLGEMEINES-COOP.md`](docs/PLAN-ALLGEMEINES-COOP.md) (the path from the Othello
  Coop to the general coop), [`docs/UNION-DATEIEN.md`](docs/UNION-DATEIEN.md).

## What's next

- Test the coop inventory, shared mission items and mechanisms in a play test with two people over a real network.
- Test Gothic 1 and Othello by hand and over a real network (the automated test run works there now).
- Detection of individual mods with their own adjustments (so far: Gothic 1/2 with mods can be installed, but are not
  specifically supported).
- Ship Union for Gothic 1 completely (Union's setup additionally writes `binkw32.dll`/`Mss32.dll` there).
- More sync details from the long test runs (e.g. dropped weapons of killed enemies).

## Building it yourself

Visual Studio 2019 Build Tools (C++, Win32), Python 3:

```
powershell -File Werkzeuge\bau.ps1 -Config "G2A MD Release" -Copy Bau\Gothic2Coop.dll
powershell -File Werkzeuge\bau.ps1 -Config "G1 MD Release"  -Copy Bau\Gothic1Coop.dll
```

For ZIPs and the installer (`Werkzeuge\makezip_g1.py`, `makezip_g2.py`, `installer_bau.py`, test `installer_test.py`) you also need
in `Bau\eingaben\`: the coop renderers (GD3D11 v17.9.7 with the patches from `Renderer\`: Gothic 1 up to `coop8.patch`, Gothic 2 up
to `coop9.patch`), `steam_api.dll` (Steamworks) and under `Bau\eingaben\Union\` the files of Union 1.0m (checked against their
checksums, see `docs\UNION-DATEIEN.md`); for the installer the release ZIP of GD3D11 v17.9.7. The DLLs per version are in `Bau\uNN\`.

| Folder | Contents |
|---|---|
| `Kern/` | shared source code of all games (`GothicCoop.vcxproj`, unity build via `Sources.h`), Union SDK in `ZenGin/` |
| `Gothic I/`, `Gothic II/`, `Othello/` | game scripts (`Skripte/`), Steam ID, images, instructions per game |
| `Installer/` | source code and texts of the installer |
| `Renderer/` | coop patches for GD3D11 v17.9.7 |
| `Werkzeuge/` | building, packages, installer test |
| `Lizenzen/`, `docs/` | license texts and origin, documentation |

## License and origin

GNU Affero General Public License 3.0 ([`LICENSE`](LICENSE)) like the original GothicCoop / Gothic Together – modified version,
changes since 16.10.2026. Anyone passing on the files must include the source code or refer to this repo. Third-party libraries:
ENet (MIT), nlohmann/json (MIT). The coop renderer is a modified version of GD3D11 (GPL-3.0). Gothic, Othello and Union files
belong to their rights holders; Union 1.0m (Gratt, WOG RU Community) is included with the packages, as mods usually do. No
warranty of any kind.


# Gothic-I-II-Universal-Coop

**Koop-Mehrspieler für Gothic 1, Gothic 2 Die Nacht des Raben und Gothic 1 mit Othello.** Zwei oder mehr Spieler erleben
dieselbe Geschichte gemeinsam: Welt, NPCs, Kämpfe, Dialoge, Tagebuch, Truhen und Spielstände werden zwischen den PCs
abgeglichen. Ein Union-Plugin auf Grundlage von [GothicCoop / Gothic Together](https://gitlab.com/gothiccoop/gothiccoop)
(AGPL-3.0), Nachfolger des [Othello-Coops](https://github.com/corshaegar/Othello-Coop).

> *English version: [README.en.md](README.en.md). Menus and the installer are available in German and English.*

**Inhalt:** [Versionen](#versionen) · [Was funktioniert](#was-funktioniert) · [Teststand](#teststand) ·
[Installation](#installation) · [Spielen](#spielen) · [Einstellungen](#einstellungen) · [Probleme](#probleme-lösen) ·
[Wie es funktioniert](#wie-es-funktioniert) · [Was noch kommt](#was-noch-kommt) · [Bauen](#selbst-bauen) · [Lizenz](#lizenz-und-herkunft)

## Versionen

Projektversion **0.4.1** – je Spiel/Mod die Coop-Fassung (sie zählt nur hoch, wenn sich an diesem Spiel etwas ändert):

| Spiel / Mod | Coop-Fassung |
|---|---|
| Gothic I | u143 |
| Gothic II | u143 |
| Othello | u143 |

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
- **Mechanismen:** Türen, Hebel, Steinplatten und Portale, die ein Held benutzt, gelten für beide, samt den Skripten und Kamerafahrten dahinter. NPCs benutzen Türen je PC.
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
| **Gothic 2 Die Nacht des Raben** (2.6) | Hauptspiel der Entwicklung. Automatischer Testlauf auf einem PC (Host und Client): Gilden, Coop-Inventar, Truhen, Türen und Mechanismen 52/52 auf beiden Seiten; getrennte Welten und Weltprozess Host 55/55, Client 52 OK, 0 Fehler (3 Schritte nur am Host). Lange automatische Läufe mit Prüfrunden (Vergleich beider Seiten), inzwischen bis in Kapitel 5 samt Drachenkampf – meist ohne Abweichung, gelegentlich einzelne Abweichungen. Das Coop-Inventar ist vor allem automatisch geprüft; ein Spieltest mit zwei Personen über das echte Netz steht noch aus. |
| **Gothic 1** (1.08k, Steam/GOG) | Derselbe automatische Testlauf: Gilden, Coop-Inventar und Türen je 41 OK, 0 Fehler (11 Schritte übersprungen, am Test-Spielstand stehen keine Truhe und kein passender Hebel); getrennte Welten und Weltprozess Host 51 OK, Client 48 OK, 0 Fehler (4 bzw. 7 übersprungen). Hebel sind in Gothic 1 damit noch nicht im Spiel geprüft. Von Hand zuletzt mit u44 gespielt; die Neuerungen seitdem (Coop-Inventar, Missions-Gegenstände, Mechanismen) noch nicht von Hand und nicht über das echte Netz. |
| **Othello** 4.x (Gothic 1) | Derselbe automatische Testlauf: Gilden, Coop-Inventar, Truhen, Türen und Mechanismen Host 49 OK, Client 42 OK, je 1 Fehler (siehe unten); getrennte Welten und Weltprozess Host 51 OK, Client 48 OK, 0 Fehler. Von Hand zuletzt mit dem Othello-Coop v106 (= Othello-Coop 0.1.2) gespielt; seitdem sind die allgemeinen Änderungen dazugekommen. |
| **Mods** | Gothic 1/2 mit anderen Mods lassen sich installieren (der Installer erlaubt es), sind aber nicht gezielt geprüft. |

Bekannte Grenzen:
- Der **Weltprozess** braucht ein NTFS-Laufwerk (Verknüpfungen statt Kopien) und ein vorhandenes Speicherspiel.
- Zuschau-Fenster und 3D-Vorschau brauchen den **Coop-Renderer** (GD3D11 v17.9.7 + Coop-Patch) und einen Prozessor mit AVX2.
- „Über IP beitreten“ braucht beim Host einen offenen **UDP-Port 9955** (Router-Weiterleitung/Firewall) – oder Steam.
- **Othello, Tore mit Hebel:** Wird der Hebel ein zweites Mal umgelegt, während das Tor noch aufgeht, kehrte es beim Partner im Testlauf meist nicht um (`PSI_DOOR_TEMPLE_NYRAS_SECRECT_*`, 5 von 6 Durchgängen). Das Tor kann dann auf den beiden PCs verschieden stehen.
- **Gothic 2, alte Spielstände:** Stände aus Fassungen vor u140 können doppelte NPCs enthalten. Die bleiben; neue entstehen nicht mehr.
- Bitte jeden Absturz und jede Abweichung melden: Protokoll `system\GothicCoop.log` beider PCs.

## Installation

**Voraussetzungen:** Windows 10/11; Gothic 1 in Fassung 1.08k (Steam/GOG), Gothic 2 Die Nacht des Raben 2.6 (z. B. Gold-Edition
mit Patch 2.6) oder Gothic 1 mit Othello 4.x. Union 1.0m bringen Installer und ZIPs für Gothic 1/2 mit; Othello hat Union dabei.

### Mit dem Installer (empfohlen)

1. Unter [Releases](../../releases) `Gothic-I-II-Universal-Coop-0.4.1-Setup.exe` laden und starten.
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
