# Gothic-I-II-Universal-Coop

**Co-op multiplayer for Gothic 1 and Gothic 2 Night of the Raven and their mods – Othello (Gothic 1), Legend of Ahssun and
The Chronicles of Myrtana: Archolos (Gothic 2) are supported.** Two or more players experience
the same story together: world, NPCs, fights, dialogues, diary, chests and saves are kept in sync between the PCs. A Union
plugin based on [GothicCoop / Gothic Together](https://gitlab.com/gothiccoop/gothiccoop) (AGPL-3.0), successor of the
[Othello Coop](https://github.com/corshaegar/Othello-Coop).

> *Deutsche Fassung: [README.de.md](README.de.md). The changelog and the technical documentation under `docs/` are in German; menus
> and the installer are available in German and English.*

**Contents:** [Versions](#versions) · [What works](#what-works) · [Test status](#test-status) ·
[Installation](#installation) · [Playing](#playing) · [Settings](#settings) · [Troubleshooting](#troubleshooting) ·
[How it works](#how-it-works) · [What's next](#whats-next) · [Building](#building-it-yourself) · [License](#license-and-origin)

## Versions

Project version **0.10.3** – for each game/mod the coop version (it only goes up when something changes for that game):

| Game / mod | Coop version |
|---|---|
| Gothic I | u243 |
| Gothic II | u243 |
| Othello | u243 |
| Legend of Ahssun | u243 |
| The Chronicles of Myrtana: Archolos | u243 |

**Both PCs need the same coop version** – it is compared when connecting. It is shown at the bottom of the main menu
("GothicCoop uNN"). Changes per version: [`CHANGELOG.md`](CHANGELOG.md) (in German).
## What works

**Games and mods**
- **One coop for all:** one DLL per engine with the scripts of all variants. If the mod changes (GothicStarter, Othello added
  or removed), the coop switches by itself at start (log "Skript-Weiche") – no new installation needed.
- **Supported mods:** Othello (Gothic 1), Legend of Ahssun and The Chronicles of Myrtana: Archolos (Gothic 2) with their own profile. Other mods started with
  `-game:<ini>` are detected; what they need comes from a profile file (see [Settings](#settings)).

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
- **Guilds per hero:** guild dialogues are only shared between heroes of the same guild (including the reward). Guild
  progress (values, dialogues, diary topics, NPC attitudes from guild quests and joining) stays with your own hero, also
  when loading together.
- What a dialogue changes on NPCs (daily routine, attitude, protection, guild, inventory such as merchant goods) applies to
  both; so do teleports, script deaths and removals of NPCs, also when a dialogue on the client triggers them.
- Script events (items from scripts, spawns, time of day and game day) come from the host. NPCs and items that a script on the
  client inserts or spawns (also `Wld_SpawnNpcRange`: summons, graves, traps) exist on the client right away and appear for both.
- **Companions and summons** that a client recruits or summons follow that client (the NPC AI runs on the host).
- **Spells of the partner** are replayed for the visuals only – their scripts no longer act a second time on your PC (in
  Gothic 2 a partner's summon used to create a hostile copy).
- **NPC states that mean "the player"** (mod profile `heroStates`): summons around the hero, arena rules and boss targets act
  on the player who is there, not always on the host (Legend of Ahssun).
- Companions, summons, spells and `heroStates` are new in u199 – checked against the code (the mechanism of `heroStates` also in
  the automated test run), not yet seen in a real playthrough.
- **End of the game** (Gothic 2): after the credits both players are back in the main menu.

**World and items**
- Chests, items on the ground and looting – the host keeps the record, nothing is duplicated.
- **Trade** (new in u201): buying and selling at a merchant applies for both – the merchant's goods and gold stay the same on both PCs
  (not yet seen in a real playthrough).
- **Items of NPCs:** what NPCs drop or throw (weapons when they die or are knocked out, tools) lies at the same spot for
  both. Arrows and bolts stay per PC (they stick in the target or disappear after a few seconds).
- **Coop inventory:** a shared chest. The key (default I, Settings › COOP) opens it directly and without moving. Both players
  can have it open at the same time; its contents are kept on the host and in the save, and items from it count for quests.
- **Shared mission items:** if one player gets one, the partner has it too; if one hands it in, the partner loses it.
  The name says "(shared)".
- **Mechanisms:** doors, levers, stone plates and portals used by a hero apply to both, including the scripts and camera
  runs behind them; if both pull the same lever at once, it counts once. Movers that react to being touched themselves (target
  stones in the Sleeper temple, Vino's cellar door) move for both as well (checked in the automated test run). NPCs use doors per PC.
- **Triggers** that a hero, an NPC or an arrow or spell touches (gates, traps, the bridge on Irdorath, the Sleeper temple in
  Gothic 1) fire for both. Checked in the automated test run, not yet seen in a real playthrough.
- **Separate worlds:** each player can be in a different world (Khorinis, Valley of Mines, Jharkendar, Irdorath, or the
  worlds of Gothic 1). **Joint world change** on request (confirm the transition, the partner follows).
- **World process and watch window:** if the partner is in another world, an invisible second Gothic shows that world;
  the watch window (Shift+F5) shows them there (needs the coop renderer).
- **Teleport rune** to the partner (also across worlds) – every hero always carries exactly one (u238: added if missing, in the spell book after loading, a second one picked up or lying in the world disappears); partner marker (name above the head, arrow at the edge of the
  screen, on the map).

**Saves**
- **Saving only together** (u237): both PCs save at the same moment (measured: within a tenth of a second). If the partner
  cannot save right now (dialogue, open inventory, loading), saving is refused and the reason is shown ("Diego: dialogue");
  the auto-backup waits for it.
- **Shared saving and loading:** the host chooses, both load the same save, each with their own hero; missing world files
  are transferred and verified. Preview of both PCs in the list. Loading a save that holds the partner's world on your own
  (for example after a restart) also gives you your own hero.
- **Backup at the partner:** everyone gets a copy of the other player's save; if a player's save is missing (lost saves),
  they get it back from the partner.
- **Shared new game:** intro for both, start side by side.
- **Auto-backup** for both, every 3 minutes if you like.

**Controls**
- **COOP menu** in the main menu: character, appearance (also "like an NPC") with 3D preview, connecting via IP or **Steam**
  (friends list, invitations), save list. Mouse in all Gothic menus. German and English.
- In the Esc menu "Choose save" and "Main menu" (back to the start screen without restarting).
- **Coop keys** (new in u226): every coop key can be changed under Settings › COOP › Coop keys (any key, also with
  Shift/Ctrl/Alt); if Gothic uses the key itself, it says so. **Help** in the game (H, or F1 if Gothic uses H) with all keys,
  **coop window** (Shift+K) for hosting, joining and disconnecting during the game, a short **introduction** before a new game.
- **Partner display** (u226): name above the head, distance, behind walls and the arrow at the edge of the screen are adjustable
  (Settings › COOP › Partner display).

## Test status

| | Status |
|---|---|
| **Gothic 2 Night of the Raven** (2.6) | Main game of development. Automated test run on one PC (host and client) with the development DLL u200 (the release packages are built from the same source and were not tested separately): guilds, coop inventory, chests, doors, mechanisms, triggers, items of NPCs, saving and loading host 110 OK, client 109 OK, 0 errors (7 and 8 steps only on the other side); separate worlds and world process (u199) host 60 OK, client 55 OK, 0 errors. Long automated runs with check rounds (comparing both sides), by now up to the end of the game (with boosted health, so not a real test of the fights) – mostly without deviation, occasionally single deviations. The general changes of u201 to u217 were tested in the runs of Legend of Ahssun and Archolos, not again in Gothic 2 itself. The coop inventory has mainly been tested automatically; a play test with two people over a real network is still outstanding. With u237 (0.10.0): automated worlds test on one PC (connecting, separate worlds, world process, saving and loading together, teleport) host 60 OK, client 56 OK, 0 errors; saving only together checked in the log ("cannot save right now: loading / dialogue", then "can save again"). With u238 (0.10.1) the same worlds test again host 60 OK, client 56 OK, 0 errors; the teleport rune went into the spell book after loading on both PCs. |
| **Gothic 1** (1.08k, Steam/GOG) | Last tested with exactly the release DLL of 0.6.0 (u197): guilds, coop inventory, doors, items of NPCs, saving and loading host 84 OK, client 82 OK, 1 error each in the test tool itself (a diagnosis step looked for the apple under its Gothic 2 name, fixed in u198) – 31 and 33 steps skipped, the test save has no chest and no suitable lever or trigger; separate worlds and world process host 55 OK, client 50 OK, 0 errors (4 and 9 skipped). The general changes of u198 to u217 (joining only with the same game, scripts of the client, companions, spells of the partner, trade, sleeping on the client, the dead after a world change, loading, safeguards against dialogue locks) have not been tested in Gothic 1 yet. Levers and triggers have not been tested in play in Gothic 1 yet. Last played by hand with u44; the additions since then (coop inventory, mission items, mechanisms, triggers) have not been tested by hand or over a real network. |
| **Othello** 4.x (Gothic 1) | Last tested with exactly the release DLL of 0.6.0 (u197), started with the Gothic 1 scripts (the script switch changed to Othello at start): guilds, coop inventory, chests, doors, mechanisms, triggers, items of NPCs, saving and loading host 99 OK, client 97 OK, 4 errors each – 3 at one gate (see below), 1 in the test tool itself (fixed in u198); separate worlds and world process host 55 OK, client 50 OK, 0 errors. The general changes of u198 to u217 have not been tested in Othello yet. Last played by hand with Othello Coop v106 (= Othello Coop 0.1.2); the general changes have been added since. With u242 (0.10.3): automated run of 40 minutes on 9 Nov 2026 without a crash – the client's fights work again (13 of 13 with hits; before, the client hit nothing in 9 of 10 fights), one swing every 0.26 s on average, as in Gothic 2. |
| **Legend of Ahssun** 1.2.0 (Gothic 2) | Own profile in the Gothic 2 DLL – the same coop as for Gothic 2, no separate package. Before u199 all 3,914 of the mod's own script functions were read and every serious finding was checked against the code; since u200 also all value proposals, all NPC states that address the hero and all world-init guards. Full automated test run with a save of the mod (development DLL u204): host 240 OK, client 236 OK, 0 errors – the dead meatbug after entering a world is fixed (u203); loading together, world change, world process and watch window passed. Plus automated runs in the mod since u170. Since 0.8.0 (u211–u217): seven one-hour automated runs on 8 Nov 2026 – no crash, one hang during a world change in the second run (a hang diagnosis was added in u213, no hang since); enlarged monsters no longer fly above their place on the client (u214/u215); final check rounds of the last five runs 0 errors (host 31 OK, client 25–31 OK). With u238 (0.10.1): a 40-minute automated run on 9 Nov 2026 without a crash, final check rounds host 31 OK, client 31 OK, 0 errors. |
| **The Chronicles of Myrtana: Archolos** (Gothic 2) | New in 0.8.0: own profile in the Gothic 2 DLL (detected by `Data\KM_Scripts.mod`), the same coop as for Gothic 2, no separate package. Fixed for it: a crash on the first save, the prologue on the ship not finishing in co-op, and per-PC values of the mod (screen fades, distance warnings, cutscene counters). Tested in long automated test runs on one PC (host and client, both heroes played automatically): about 7 hours in total without a crash; check rounds without errors except in one session (weapon mode and animation of nearby NPCs). Played so far in these runs: the prologue and chapter 1 up to Silbach (Rupert's escort). Since 0.8.0 (u211–u217) fixed for it: a crash when loading saves made during a screen fade, NPCs that no longer talked to the hero after a screen fade that never ended (u217), enlarged monsters flying high above their place on the client; plus a safeguard against a dialogue lock after interrupted conversations (derived from the mod scripts, not triggered in the runs). Seven one-hour automated runs on 8 Nov 2026: no crash, final check rounds 0 errors (host 25–31 OK, client 25–31 OK). Since 0.9.1 (u218–u237) fixed for it: scenes and cutscenes of the prologue on the ship (the captain no longer stays behind the grate), trialogs (speaker, order and positions of the NPCs for the partner), heads turning the same way, both PCs saving at the same moment – checked in automated runs with recordings on 9 Nov 2026. 0.10.0 and 0.10.1 crashed on every save in Archolos (fixed in u239), and the end of the prologue on the ship could hang when the client's part of the scene ran ahead (fixed in u240). Both checked in automated runs on 9 Nov 2026 with the client joining during the prologue: start save and joint saves, the end of the prologue on both PCs, the heroes reached the beach. With u243 (0.10.3) the whole prologue ran through again with the client joining during it – the captain's scene in the right order, then the final scene – without a crash. Not yet played by hand. |
| **Other mods** | Gothic 1/2 with other mods can be installed; the installer shows the mods in the folder. The coop's foundation runs; whatever adjustments a mod needs come from a profile file (see [Settings](#settings)). Not specifically tested. |

Known limits:
- The **world process** needs an NTFS drive (links instead of copies) and an existing save.
- Watch window and 3D preview need the **coop renderer** (GD3D11 v17.9.7 + coop patch) and a CPU with AVX2.
- "Join via IP" needs an open **UDP port 9955** on the host (router forwarding/firewall) – or Steam.
- **Othello, gates with levers:** if a gate is moved again while it is still moving (lever flipped a second time, both players at the lever at once, an arrow at its trigger), it ends up in different positions on the two PCs in the test run (`PSI_DOOR_TEMPLE_NYRAS_SECRECT_2`, 3 steps per side).
- **Gothic 2, old saves:** saves from versions before u140 can contain duplicate NPCs. Those stay; no new ones appear.
- **Script events on the client** (teleport, script death, removing an NPC) only apply at the host if it is in the same world.
- Please report every crash and every deviation, with the log `system\GothicCoop.log` from both PCs (with Othello
  `system\GothicCoopOthello.log`).

## Installation

**Requirements:** Windows 10/11; Gothic 1 version 1.08k (Steam/GOG), Gothic 2 Night of the Raven 2.6 (e.g. Gold Edition with
patch 2.6) or Gothic 1 with Othello 4.x. Installer and ZIPs bring Union 1.0m for Gothic 1/2; Othello includes Union.

### With the installer (recommended)

1. Download `Gothic-I-II-Universal-Coop-0.10.3-Setup.exe` from [Releases](../../releases) and start it.
2. Choose the **game folder** (the folder containing `system`; Steam and GOG folders are searched). There is no game to choose:
   the installer detects Gothic 1, Othello, Gothic 2 and the mods in the folder (e.g. Legend of Ahssun) by content and shows which
   are supported. It installs the coop for the engine with the scripts of all variants – if the mod changes later (GothicStarter,
   Othello added or removed), the coop switches by itself at start. It installs nothing into a wrong folder.
3. "Install". It writes the coop, brings GD3D11 v17.9.7 (if missing) and puts the coop renderer on top (can be deselected);
   if Union is missing for Gothic 1/2, it asks and installs Union 1.0m as well.
4. Everything it replaces is backed up first in `CoopBackup\<time>\`. **"Undo last installation"** in the same installer
   restores the old state (including Union, if it installed it). Log: `CoopLogs\Installer\`.

Without a window: `Setup.exe /still /ordner:"<game folder>" [/spiel:g1|g2|othello|loa as a check] [/union] [/ohnerenderer] [/rueckgaengig]`.

### With the ZIP

1. Download `Gothic-I-II-Universal-Coop-0.10.3.zip` – one ZIP for all games.
2. Copy the contents of **one** of its folders into the game folder (where `system` is) and replace existing files:
   `Gothic 1 + Othello` for Gothic 1 (also with Othello), `Gothic 2 + Mods` for Gothic 2 (also with Legend of Ahssun and other mods).
3. `Union 1.0m (nur wenn Union fehlt)` ("only if Union is missing"): only if the game folder has no Union yet (`system\Shw32.dll`
   missing) – never for Othello. Then remove `system\dinput.dll` (the SystemPack loader – Union loads SystemPack itself).
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

Keys (defaults – all can be changed under Settings › COOP › Coop keys): **H** (or F1) help · **Shift+K** coop window ·
**I** coop inventory · **Shift+T** teleport rune to hand (added if missing; press again to put it away) · **Shift+F5** watch window at the partner ·
**Shift+F4** revive the partner · **F6** resync the world. Older saves: Esc › "Choose save".

## Settings

- In the game under **Settings › COOP** (sync, world change together/separate, camera, difficulty, language); stored in
  `Gothic.ini`, section `[GOTHICCOOP]`.
- Basic settings in `system\GothicCoopConfig.json` (name, address, port, keys). **Without this file the coop does not start.**
  Some switches there: `coopSeparateWorlds` (separate worlds), `coopWorldProcessAuto` (start the world process in advance),
  `coopEnterTakeState` (take over the partner's state when entering a world).
- **Mod profile** (from u194): adjustments for a mod without a new DLL in `system\GothicCoopMod-<Ini>.json` (`<Ini>` = the mod's
  game ini without ".ini", e.g. `GothicCoopMod-LoA.json` when started with `-game:LoA.ini`; without `-game`:
  `system\GothicCoopMod.json`). Contents: `perPcVars` (script integers that stay per PC, `NAME_*` for a prefix), `heroVars` (from
  u197: values that travel with your own hero – when connecting and loading you keep your own, e.g. a mod's talents or recipes),
  `deltaVars` (from u199: counters that only player scripts raise – synced as a change, so two steps at the same time both count; never
  for counters of loops running on both PCs), `heroStates` (from u199: NPC AI states, `ZS_…`, whose scripts mean "the player here" by
  `HERO` – during them `HERO` is the player the NPC fights, otherwise the nearest player), `worldVars` (from u200: guards that only a
  world's init sets, `{"NAME": "WORLD"}` – they belong to the world file, so whoever enters with their own file runs the block), `vobSync`
  (from u201: names of world objects whose visibility, collision, transparency and visual mod scripts change directly – synced between
  players in the same world), `configDefaults`
  (defaults for the `coop…` switches as long as `GothicCoopConfig.json` does not set them), `notes` (hints in the log), `name`,
  `engine`. Comments are allowed. A commented template comes with every package (`system\GothicCoopMod.vorlage.json`). Both PCs
  need the same file; the log shows at start what was read.

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
  The same source is built per engine; the Gothic 1 DLL detects Othello itself and switches on its Othello adjustments, and the
  Gothic 2 DLL does the same for Legend of Ahssun and Archolos. Other mods get their adjustments from a profile file – there is no separate
  DLL per mod. The scripts of each variant are in `system\GothicCoop\Skripte\<g1|othello|g2>`; at start the DLL puts the right set
  into `system\Autorun` before zParserExtender loads it ("Skript-Weiche", script switch).
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
- Add and test more mods with profile files (built in so far: Othello, Legend of Ahssun, Archolos).
- Archolos: play beyond chapter 1 (longer automated runs with a walkthrough).
- Ship Union for Gothic 1 completely (Union's setup additionally writes `binkw32.dll`/`Mss32.dll` there).
- More sync details from the long test runs.

## Building it yourself

Visual Studio 2019 Build Tools (C++, Win32), Python 3:

```
powershell -File Werkzeuge\bau.ps1 -Config "G2A MD Release" -Copy Bau\Gothic2Coop.dll
powershell -File Werkzeuge\bau.ps1 -Config "G1 MD Release"  -Copy Bau\Gothic1Coop.dll
```

For ZIPs and the installer (`Werkzeuge\makezip_g1.py`, `makezip_g2.py`, from these the universal ZIP with `makezip_universal.py`;
`installer_bau.py`, test `installer_test.py`) you also need
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
