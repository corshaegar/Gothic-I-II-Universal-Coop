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
