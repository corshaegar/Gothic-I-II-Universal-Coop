# Changes

> *Deutsche Fassung: [CHANGELOG.de.md](CHANGELOG.de.md).*

Each section names the project version and, for each game and mod, the coop version (`uNN`); the newest version is at the top.
At the very bottom you will find the history before the first release: the Othello Coop 0.1.0–0.1.2
(github.com/corshaegar/Othello-Coop) and the unreleased versions u1–u61 that became 0.2.0. This way, every change since the very
first version is recorded here.

## 0.11.1 – Host stuttering every 5 seconds, Archolos started from Steam (Gothic I u254, Gothic II u254, Othello u254, Legend of Ahssun u254, Archolos u254, Xeres' Rückkehr u254)

Found in the logs of two players who played Archolos together on 12 Nov 2026.

**Fixes**

- **The host's game froze every 5 seconds** (u253): An old diagnostic from the early days of the project wrote a snapshot of all characters nearby to `system\GothicCoopSnapshot.log` every 5 seconds while connected – and opened and closed the file again for every single line. On a PC whose virus scanner checks every file when it is closed, this took longer the bigger the file grew: in the player's log the host froze for 35 ms at first and for almost 0.9 seconds after 13 minutes. The partner noticed it as well, because no movements came from the host during those moments. The snapshot is now switched off, together with two more diagnostics nobody needs anymore: one on the client that looked for long-fixed bugs, and a log of NPC names that wrote more than 1,000 lines at once when loading. The time-lapse used by the automated test runs is no longer part of the release build either. Everything that helps with bug reports stays: frame times and a network summary every 30 seconds, the log that is sent to the partner, the self-check at startup, and the reports on hangs and crashes.
- **Archolos ran without its profile when started from Steam** (u254): Steam starts Archolos with `-game:TheChroniclesOfMyrtana.ini`. The coop did not recognize the mod by that ini name and treated Archolos like an unknown mod – without the adjustments made for it, such as the values Archolos keeps per PC (screen fades, distance warnings, the cutscene counters of the prologue). And if only one of the two players started Archolos this way, the coop refused the connection as if they were playing different games. It now also recognizes Archolos by `Data\KM_Scripts.mod` when the game is started with its own ini, so both ways of starting lead to the same profile.

## 0.11.0 – Xeres' Rückkehr, installer for Archolos, blood splatter crash, trapped mouse (Gothic I u252, Gothic II u252, Othello u252, Legend of Ahssun u252, Archolos u252, Xeres' Rückkehr u252)

**New**

- **Xeres' Rückkehr** (u246–u252): own profile in the Gothic 2 DLL, started with `-game:Xeres.ini`. Before that, all 6,583 script functions of the mod were read and every serious finding was checked against the code. Each hero keeps their own poisons, diseases, training, god bonus, transformations and perks; the mod's scene counters, the mine cart ride and the mod's settings apply per PC. Tested in automated test runs with a new game (host and client on one PC). The story has not been played through yet. For players without the coop, the fixes are also available as a separate package: [Xeres-Fixes](https://github.com/corshaegar/Xeres-Fixes).
- **The installer recognizes Archolos**: Up to 0.10.3 it refused the Archolos folder because Archolos ships the addon world only as an empty file. It now recognizes Archolos by its `Data\KM_Scripts.mod`, like the coop itself, and installs normally.

**Fixes**

- **Archolos no longer started after installing the ZIP by hand**: The ZIP and the installer replaced `system\steam_api.dll` and `steam_appid.txt`. Archolos brings its own Steam integration, which needs a function that the coop's newer `steam_api.dll` no longer has – afterwards Archolos did not start. The installer now keeps a game's own Steam files; in the ZIP they are in their own folders, to be copied only if the game has none. In Archolos, coop over Steam therefore stays off – connect over IP. If your Archolos no longer starts: let Steam verify the game files, then install with the new installer.
- **Crash when arrows hit you with blood splatter on** (u249–u252): With the Union Toolkit (`Data\Toolkit.vdf`, brought along by several Union plugins), LeGo 2.9 replaces a mod's older LeGo. The blood splatter and the damage function of mods like Xeres' Rückkehr then kept calling each other until the game crashed. The coop now gives the mod back its own blood splatter function: no loop, and the splatter shows the damage actually taken. Tested with a staged hit from a bow; without the fix the game hung at exactly that point.
- **Objects above 2 GB treated as missing** (u246): Gothic 2 may use up to 4 GB of memory, and some mods check object addresses with `> 0` – an object in the upper half then counts as missing. In Xeres' Rückkehr this affected 20 places (world changes, the pass into the Valley of Mines, the doors of the spider burrow, the mine carts, chests). The coop corrects such checks at game start, in every mod.
- **Mouse trapped in the top left corner** (u251, u252): The world process – a second Gothic that the coop runs in the background on its own Windows desktop – sometimes confined the real mouse to its invisible window. The coop now prevents this from the very start. Measured in two test runs with up to five Gothic instances: not trapped once (before: about 40 seconds per run).
- **Archolos: NPC stopped in conversation, duplicate NPCs** (u244, u245): An NPC could stop in the middle of a conversation because its command list no longer ran; the dialogue watchdog now restarts it. An NPC that a script had just inserted was sometimes created a second time at the partner's (e.g. Ana and Lea); the coop now also takes over its own NPC at a distance if it is the only one of its kind.
- **Guide for installing by hand**: Union is missing if `system\Union.patch` is missing – the old guide named `system\Shw32.dll`, which every Gothic has.

## 0.10.3 – Scenes in the Archolos prologue, fights in Othello (Gothic I u243, Gothic II u243, Othello u243, Legend of Ahssun u243, Archolos u243)

Found in automated test runs on 9 Nov 2026 in Archolos and Othello (host and client on one PC).

**Fixes**

- **No more saving in the middle of a scene** (u243): When the game saved during a scene – automatically, together with the partner or when the partner joined – it froze for a moment. Afterwards the mod's scripts caught up on all the missed steps at once and skipped parts of the scene. In Archolos this cut the captain's scene on the ship for the host while it played on the client. Like Gothic in single player, the coop now holds off saving while a camera sequence or a scene of the mod is running. In the test, the prologue ran through completely.
- **Invincible after the partner's conversation** (u242): When the partner talked to an NPC, the NPC's dialogue script also made your own hero invincible – and it stayed that way. In Othello, monsters did not attack such a hero but ran away; the client hit nothing in 9 of 10 fights. In some mods, NPCs no longer talked to that hero. The coop now removes the invincibility 10 seconds after the conversation ends. In the Othello test, the client landed hits in all 13 fights.
- **Camera sequences playing twice** (u241): If the client was behind the host in a scene, a camera sequence could play a second time. The host now also recognizes steps it already carried out itself up to two minutes earlier and does not repeat them.

## 0.10.2 – Saving in mods with LeGo without crashing, Archolos prologue to the end (Gothic I u240, Gothic II u240, Othello u240, Legend of Ahssun u240, Archolos u240)

Found in automated test runs on 9 Nov 2026 in Archolos (host and client on one PC; the client joined in the middle of the prologue).

**Fixes**

- **Crash when saving in Archolos and other mods with LeGo** (u239): With 0.10.0 and 0.10.1 the game crashed on every save if a mod's scripts hook into saving via LeGo (e.g. Archolos). Since 0.10.0 the coop places a hook there as well, so that saving only happens together. At game start, LeGo copies the jump into the coop to another location, where it points to nowhere. The coop has been correcting such copies since 0.6.0 (back then for Legend of Ahssun), but it did not know the new locations. Now it detects every location at game start where it or another plugin has placed a hook. Checked in Archolos: the save at the start of the game and joint saves.
- **Archolos: the end of the prologue hung** (u240): Some scenes are played by each PC on its own. They got stuck when the client was further along than the host: the client's scripts killed or moved NPCs that the host's scene was still waiting for – on the ship, for example, Rupert first has to jump into the water. The client's camera stayed stuck in the last shot, and both heroes stood on deck. The host now waits up to 30 seconds to see whether its own scene performs the same step; only if it does not, it carries out the client's step. In the test, the client was ahead again, the end of the prologue ran through on both PCs, and both heroes reached the beach.

## 0.10.1 – Teleport rune to hand by key, always exactly one; tidier message line (Gothic I u238, Gothic II u238, Othello u238, Legend of Ahssun u238, Archolos u238)

**New**

- **Teleport rune by key** (u238): The rune key (Shift+T by default) puts the teleport rune in your hand. If it is missing, it is first added to the inventory and the spell book; pressing the key again puts it away. It is cast like any other rune. Previously the key teleported immediately.
- **Always exactly one coop rune** (u238): Every hero always carries their teleport rune – in Gothic 1 and Gothic 2, with and without mods – and after every load also in the spell book if there is room. If it is missing, it comes back; if you pick up a second one, you still have only one. A rune lying loose in the world disappears after a few seconds.

**Fixes**

- **Message line** (u238): The hint about the watch window and its key now appears only once per session instead of at every automatic switch. The client shows difficulty and sync settings only when the host changes them during the game; the values at connection time only go to the log.

## 0.10.0 – Saving only together, coop keys and help, Archolos prologue with trialogs (Gothic I u237, Gothic II u237, Othello u237, Legend of Ahssun u237, Archolos u237)

Based on automated test runs with recordings on 8 and 9 Nov 2026 in Archolos and Legend of Ahssun (host and client on one PC, both heroes controlled automatically) and a worlds test in Gothic 2 with u237.

**New**

- **Saving only together** (u227, u236, u237): Both PCs save at the same moment – within a tenth of a second according to the file times of the saves, auto-saves included. Previously the partner only saved once the other player was done, and if busy, even later – with a different game state. Now both PCs keep telling each other whether they can save right now. If the partner cannot (dialogue, open inventory, loading), the game refuses to save and shows the reason (e.g. "Diego: dialogue"); the auto-save waits until saving is possible.
- **Coop keys** (u226): All coop keys can be assigned freely under Settings › COOP › Coop keys, including combinations with Shift, Ctrl or Alt; if Gothic already uses a key, it says so. Newly assigned: help (H, or F1 if Gothic already uses H), coop window (Shift+K) and teleport to the partner (Shift+T, with the rune in the inventory).
- **Help, coop window, introduction** (u226): A help menu shows all keys and briefly explains the essentials. The coop window opens by key or under Settings › COOP (hosting, joining and disconnecting also during the game). A short introduction appears before a new game (it can be turned off), and the game gives hints – when hosting, about port and firewall, and when the partner lies unconscious, about the key to revive them.
- **Partner display** (u226): Name, distance, display behind walls and the arrow at the edge of the screen are adjustable (Settings › COOP › Partner display). With the partner in focus, their name no longer appears twice.
- **Appearance "like an NPC"** (u222): The selection now knows all 654 characters of Gothic 2, also in mods; the characters of the running game come first.
- **Head turning** (u235): NPCs turn their heads the same way on both PCs; previously, on the client they mostly looked at the client's own hero.
- **Watch window** (u225): At a low frame rate it redraws only every second to fourth frame – less stutter.

**Fixes**

- **Archolos, prologue on the ship** (u218, u221, u224, u228, u229): Scenes and cutscenes run to the end on both PCs. A conversation of the partner no longer deletes a scene's commands; if a scene starts on the partner's side, your own conversation ends or the scene waits until it is over; counters that drive a scene start correctly on the partner's side; death scripts and scenes running on both PCs no longer trigger anything twice. As a result, the captain no longer stays behind the grate.
- **Trialogs** (LeGo – Archolos and many other mods; u220, u230–u235): In conversations with several NPCs (on the ship: the captain, Jorn and Timo) the partner sees the right speaker in the right order. NPCs no longer jump to other spots, the lines of the other speakers appear, and skipping hits the right line.
- **Conversations with the same NPC** (u220): If both players talk to the same NPC at the same time, the conversations no longer interfere with each other.
- **Escort** (u223): When an NPC leads the group, it measures the distance to the nearest player instead of only to the hero of its own PC.
- **Settings › COOP** (u227): The entry now also appears in mods whose menu has no graphics entry from Gothic (e.g. Archolos).

**Removed** (u237)

- The list of coop saves in the game (F9, F8) – the saves are in the menu under Esc › "Choose save".
- The key for the world process (Shift+F12) – it still starts by itself.
- The coop test run with its keys (Shift+F11, F7), the network statistics (O) and the editing mode of the template (`editmode`). Test runs are only part of the development build now.

## 0.9.1 – Archolos: NPCs talk again after a stuck screen fade (Gothic I u217, Gothic II u217, Othello u217, Legend of Ahssun u217, Archolos u217)

From the seventh and eighth one-hour automated test runs on 8 Nov 2026 (Archolos and Legend of Ahssun at the same time, host and client on one PC): no crash, no hang, and the final check rounds found no errors.

**Fixes**

- **Screen fade lock** (u217): Archolos allows no conversation while the screen is faded out (script variables `SCREENFADED` and `FADESCREENSTATE`) – not even the important dialogues through which NPCs move the story forward. If the game was saved in the middle of a fade and that save was loaded with a version before u211, the screen never faded back in: from then on no NPC talked to the hero, in every later save too. If the screen now stays faded out for 60 seconds without a dialogue being open, the coop fades it back in with the mod's own function; this also lifts the hero's movement lock. In the test run this kicked in after loading such saves on host and client; afterwards the NPCs talked again, and Rupert led the heroes on. This applies to all mods with these fade functions.

## 0.9.0 – Stability: crash when loading, flying monsters, hang diagnosis (Gothic I u216, Gothic II u216, Othello u216, Legend of Ahssun u216, Archolos u216)

Based on a long overnight run (three crashes when loading in Archolos, which led to u211) and six one-hour automated test runs on 8 Nov 2026 in Archolos and Legend of Ahssun (both at the same time, host and client on one PC, every run evaluated). The six runs had no crash and one hang (in the second run, Legend of Ahssun, during a world change; the hang diagnosis was added afterwards, and no hang has occurred since). The check rounds at the end of the last four runs found no errors.

**Fixes**

- **Crash when loading** (u211): In mods that store Ikarus function pointers in the save, the game crashed after loading if the script code was at a different address in the new process – in Archolos with saves made during a screen fade (in co-op the partner can trigger a joint save at any time). The coop now stores the address of the script code in the save and converts such pointers when loading. This applies to all mods.
- **Flying monsters** (u212–u215): On the client, enlarged monsters (Archolos: goblin warriors, lurkers; Legend of Ahssun: old molerats, meatbugs) floated hundreds of meters to kilometers above their spot. The cause is an engine bug: for enlarged models far from the camera, the ground level of the animation grows from frame to frame, and the ground correction lifts the character by exactly that amount. The coop now keeps the value fixed (Gothic 2; in Gothic 1 it recalculates it for the host's NPCs once it has grown too large).
- **Spells of the partner** (u211): Replaying a spell of the partner while the spell book was empty crashed the game, and an interrupted replay then got stuck. Both are fixed.
- **Items in NPCs' hands** (u211): If an item that an NPC is currently using gets deleted, the coop immediately removes the reference to it.
- **Hang diagnosis** (u213): If the game stands still for 90 seconds, the coop writes the call stack of the main thread, the script trace and the game state to `system\GothicCoopStapel.log`. This makes it possible to investigate hangs in which the game does not crash.
- **Dialogue lock safeguard** (u216): At the start of a conversation, Archolos sets the flag "invincible in dialogue" on the hero and only clears it when the conversation ends normally; as long as it is set, no NPC starts a conversation with the hero. If a conversation ends any other way (loading in the middle of a dialogue, an interruption), the hero would stay locked, even in the save. The coop clears the flag 10 seconds after the conversation ends and after loading, unless the mod needs it at that moment (open dialogue, use of an interactive object, intended invisibility). Found by reading the mod's scripts; not triggered in the test runs so far.

## 0.8.0 – The Chronicles of Myrtana: Archolos, sleeping on the client, the dead after a world change, trade (Gothic I u210, Gothic II u210, Othello u210, Legend of Ahssun u210, Archolos u210)

**New**

- **The Chronicles of Myrtana: Archolos** (u204–u210): The coop detects Archolos (by `Data\KM_Scripts.mod`) and brings its own mod profile: values kept separate per PC or per hero, separate counters for cutscenes, screen fades and distance warnings of the mod for each PC, and a COOP entry in the mod's main menu. Fixed: a crash on the first save (u205) and the prologue on the ship not finishing in co-op (u207). Checked in long automated test runs (about 7 hours in total without a crash); played so far: the prologue and chapter 1 up to Fabio's cave.
- **Sleeping on the client** (u204): If the client sleeps or a script on the client sets the clock, the new time applies to both.
- **Trade** (u201): Buying and selling at a merchant now applies to both – the merchant's goods and gold match on both PCs. Previously the partner could buy the same goods again. Applies to Gothic 1 and 2 and all mods; not yet confirmed in normal play.
- **Mod profiles: object states** (u201): New entry `vobSync` – if mod scripts change the visibility, collision, transparency or visual of named objects in the world, this applies to both players in the same world. Legend of Ahssun: the barricade when clearing out the nest (chapter 5) is now open for the partner too; orc graves, felled trees, a blood trail and torture racks look the same for both.

**Fixes**

- **The dead after a world change** (u203): After entering a world, some corpses were missing on the partner's side (Legend of Ahssun: the dead meatbug), and dead NPCs that the host had created after loading had a different name on the partner's side.
- **Talking to NPCs who first have to get up** (u206, u210): The client could not talk to NPCs who were leaning, sitting or standing guard. Now they get up; on the client they only play the getting-up itself and otherwise keep following the host's state.
- **Script teleports via Ikarus** (u204): If a script on the client moves one of the host's NPCs via Ikarus, this now also happens on the host.
- **Script values changed at the same time** (u204): If both PCs change the same value almost simultaneously (Archolos rolls more than 100 events at the change of day), the host keeps its value.
- **Legend of Ahssun** (u202): 15 more entries in the mod profile, found while reviewing the analysis.
- **Crash log** (u205, u210): Stack overflows now go to `system\GothicCoopStapel.log`; for deleted items in NPCs' hands and for vanished corpses, the log names more callers.

## 0.7.1 – Legend of Ahssun: more values per PC and per hero, more NPC states that refer to the player on the spot (Gothic I u200, Gothic II u200, Othello u200, Legend of Ahssun u200)

From the further review of the Legend of Ahssun analysis (value proposals and all NPC states that concern the hero). Checked against the code and in the automated test run, not yet confirmed in normal play.

**Fixes**

- **Legend of Ahssun** (u200): The dream locked the inventory on the partner's side too; an answer at the throne guardian locked the conversations of both players; the cold and thirst bars jumped back and forth between the values of both heroes; cooking talent, water knowledge, an infection, one-time XP for reading and menu states (praying, mixing paints, tobacco) were carried over to the partner; open displays, gambling stakes, pickpocketing and effects of the Magus fight got mixed up between the PCs. All of this is fixed.
- **Legend of Ahssun, NPCs** (u200): Ambushes, guards, waiting NPCs, guides, companion animals, sleepers and a ghost react to the player who is there, no longer always to the host – the ghost used to hit both heroes. Companions and summons still follow their own player.
- **Legend of Ahssun, worlds** (u200): Whoever entered the gold mine in chapter 4 second and alone found all its inhabitants still alive: the world's start script skipped the relevant part because the partner had already set the flag. Such flags now belong to the world file (new profile entry `worldVars`), and so does the inventory of the Undead Dragon.
- **Legend of Ahssun, counters** (u200): Good deeds, mined ore and 24 more counters from conversations (respect of the craftsmen and recruits, votes of the fighters, fights won …) are no longer lost when both players hold different conversations at the same time: both steps count.

## 0.7.0 – Legend of Ahssun fully analyzed, companions and summons of the client, script events on the client take effect immediately (Gothic I u199, Gothic II u199, Othello u199, Legend of Ahssun u199)

The changes from the Legend of Ahssun analysis were checked against the mod's code, and those marked accordingly also in the automated test run; nobody has experienced them in normal play yet.

**New**

- **Legend of Ahssun read completely** (u198, u199): All 3,914 of the mod's own functions were read before the next test (with a pattern search in the scripts and a local language model), and every serious finding was checked individually against the code. The changes below come from this; every new mod will be checked this way from now on.
- **Companions and summons of the client** (u199): They now follow the client. Previously they always ran to the host because the NPC AI runs on the host (Gothic 2: summons; Legend of Ahssun: recruited companions). The log says "Begleiter … gehoert zu FRIEND_n".
- **Spawns from the client's scripts** (u199): What the client's scripts spawn with `Wld_SpawnNpcRange` (summons, graves, traps, boss attacks) now appears for both; previously these NPCs only appeared on the client.
- **Spells of the partner** (u199): Replaying a spell of the partner no longer has any script effect. Previously a summon by the partner created the hostile variant on your own PC (Gothic 2), and runes with a lasting effect also acted on the partner.
- **Mod profiles: counters** (u199): New entry `deltaVars` – counters that only the players' scripts raise are synced as a change rather than as a fixed value; two simultaneous steps both count.
- **Mod profiles: "the player on the spot"** (u199): New entry `heroStates` – in the listed AI states, `HERO` means the player the NPC is fighting, otherwise the nearest player. Legend of Ahssun: the summoning in the swamp, the fist-fighting arena, Kastor/Parcival and the Magus's illusions now refer to the player who is there, previously always to the host (the arena even took the host's weapon away when the client fought). The mechanism has been checked in the automated test run.
- **Joining only with the same game** (u198): Engine and mod are sent along when connecting; a client with a different game disconnects before it loads a save (previously, for example, a Gothic 2 client crashed when loading at a Gothic 1 host).

**Fixes**

- **Crashes on the client** (u199): Scripts that keep using an NPC or item they just inserted reached into nothing on the client (Legend of Ahssun: chapter 5 in the conversation with Argos, the gate in the Temple of Awakening, arena round 9). The client now creates the object itself right away, and the host's later message is matched to this object (checked in the automated test run in Gothic 2 and Legend of Ahssun).
- **Legend of Ahssun** (u199): Giving and selling items, the grindstone and taking runes apart also ran on the partner's side (their selected item was gone, the movement lock stayed); the blessing at the shrine took values from the partner when it expired that they had never received; curse points, recipes, lectern bonuses, character values and other values of your own hero are no longer passed on to the partner; if both players put out fires at the same time, both count (quest "Löscharbeit").
- **Gothic 2** (u199): Practice from mining gold stays with your own hero.
- **Duplicate events** (u199): If both PCs insert the same thing within 3 seconds, the host now merges them even when the scripts roll a different waypoint on each PC.
- **Test run** (u198, u199): Two runs started shortly after each other no longer use the same port; the engine comparison no longer depends on how module names are spelled; the diagnosis step with the apple works in Gothic 1 and Othello.

## 0.6.0 – One coop for all games and mods, Legend of Ahssun, mod profiles (Gothic I u197, Gothic II u197, Othello u197, Legend of Ahssun u197)

**New**

- **One coop for all games and mods** (u195, u197): The installer no longer asks for the game. It detects Gothic 1, Othello, Gothic 2 and the mods installed there by the folder contents and shows before installing which ones are supported. It installs the coop for the respective engine together with the scripts of all variants. If the mod changes later (via the GothicStarter, or Othello is added or removed), the coop switches by itself at start (log: "Skript-Weiche"). Instead of one ZIP per game there is one universal ZIP with a folder per engine.
- **Legend of Ahssun** (Gothic 2, u170–u197): The mod is supported as the second one after Othello, with its own profile in the Gothic 2 DLL. The full automated test run passes with a save of the mod. Needed for this, among other things: the COOP entry in the main menu of mods that use Gothic's menu grid (u170); coop hooks that act behind those of LeGo, also in the world process (u170, u190); separate values per PC for script libraries such as Ikarus, LeGo and Gothic Free Aim (u189, u195); a fix for a crash when loading, including the repair of damaged saves (u196); each hero's own practice, talents, recipes and achievements, also when connecting (u197).
- **Mod profiles** (u187, u194, u197): The coop now generally detects mods started with `-game:<ini>`. What a mod needs is in a profile file `system\GothicCoopMod-<Ini>.json` instead of the code: values per PC (`perPcVars`), values that travel with your own hero (`heroVars`) and defaults for coop switches (`configDefaults`). A template with explanations comes with every package.
- **Events on the host's NPCs** (u195): Teleporting, killing by script and removing an NPC, triggered by a dialogue or a script loop on the client, now also apply on the host (as long as both are in the same world). Checked in the automated test run.
- **NPC inventories from conversations** (u163): Whatever a conversation puts into or takes out of an NPC's inventory (such as the merchant goods at a chapter change in Gothic 1), the partner gets as well. In saves in which the client has already changed chapters, the goods are not added afterwards.
- **Movers that react to being touched** (u167): The target stones and a gate in the Sleeper temple (Gothic 1) and Vino's cellar door (Gothic 2) now also move for the partner. Checked in the automated test run, not yet confirmed in normal play.

**Fixes**

- **NPCs on the client** (u179, u188, u193): NPCs run by the host react again on the client after one of their reactions was suppressed there – previously they then refused conversations, and gate guards overlooked the hero. They only become hostile on the client when your own hero attacks or is attacked, not merely when they observe something else.
- **Knocked out by drinking** (u161): The hero wakes up again; previously the hero stayed on the ground (seen in Gothic 1).
- **Hits** (u165): Two hits in the same frame (e.g. from a fall) are no longer replayed by the partner with too much damage.
- **Loading and taking over** (u168, u178): The client's hero is no longer placed on an edge from which the hero falls. An error in the middle of loading ends Gothic instead of leaving it with a half-loaded world.
- **Attitudes per hero** (u162): NPCs' attitudes towards the host's hero from guild quests no longer reach the client with the full sync after loading.
- **NPC with an invalid position** (u164): An NPC with an invalid position (NaN, e.g. after a fight was handed over to the client) is put back in its place, also in existing saves.
- **Chapter screen** (u182): The partner's chapter screen appears immediately, even in the middle of your own conversation.
- **Gothic 2** (u195): The upgrade level of the Claw of Beliar and the menu state at crafting spots stay with your own hero.
- **Clock** (u186): The coop's deadlines and waiting times no longer depend on the PC's clock; when the clock jumped back, they all used to shift by an hour.
- **Crashes:** firing an arrow without a ranged weapon in hand (u173, u177); picking up an item that had already been deleted (u186, Gothic 1); death of the client's character at the host (u186, Gothic 1, safeguarded); the protection for items in NPC slots missed memory that had already been reused (u166).

## 0.5.0 – Triggers and NPCs' items shared, own hero when loading, guild progress per hero (Gothic I u160, Gothic II u160, Othello u160)

**New**

- **Triggers** (u146, u158): Triggers hit by an arrow or spell, or entered by a hero or NPC, now also fire for the partner – for example the bridge on Irdorath, the main gate of the Old Camp, traps and doors in the crypt and in the Adanos temple, and in Gothic 1 the floor plates in the Sleeper temple. Previously the chain only ran on the PC on which it was triggered. Checked in the automated test run, not yet confirmed in normal play.
- **NPCs' items** (u156–u158): Whatever NPCs drop or throw (weapons when dying or knocked out, tools) lies at the same spot for both. Previously it fell on each PC separately, at different distances or only on one side. Arrows and bolts stay separate per PC: they stick in the target or disappear after a few seconds.
- **Guild progress per hero, also when loading** (u150, u152, u154, u160): Values, conversations, diary topics and NPC attitudes from guild quests and joining stay with your own hero, also after loading together and even if the partner was never there. Previously the partner got the host's guild progress when loading and then possibly could not get into their own guild any more. Saves from before u150 do not contain this information.
- **End of the game** (u147, Gothic 2 only): After the credits both players end up in the main menu, not just the one who held the last conversation.

**Fixes**

- **Own hero when loading** (u159): Anyone who loads a save on their own that holds the partner's world (e.g. after a takeover or a restart) now gets their own hero instead of a copy of the partner's hero.
- **Levers** (u145, u147): If both pull the same lever at once, it fires only once; previously sequence puzzles reset, and gates opened and immediately closed again. A lock on a mover (the last door on Irdorath) is now also found on the partner's side.
- **NPC names and the dead** (u150–u152): NPCs with the same name, such as several goblins of a group, were swapped between the two PCs after loading; a death then hit the wrong animal on the partner's side. Wrong dead from older saves and corpses that are still alive on the host get back up.
- **Transformation** (u149): During a transformation or mind control there is no joint save any more (previously the animal became the hero of the save), and the sync of mission items no longer takes anything away from the partner.
- **Partner's character** (u144): It no longer falls endlessly when there is no ground below it.
- **Mission items** (u160, mainly Gothic 1): An item whose info lines are all taken is no longer re-marked twice per second.
- **Crashes:** loading in the middle of a conversation (u155); an NPC switching spells (u146); the world process in the first frame after a world change, now caught generally (u148); Gothic 1 client when loading with its own guild topics (u153).

## 0.4.1 – Doors shared (Gothic I u143, Gothic II u143, Othello u143)

- **Doors** (u143): Opening, closing and unlocking by a hero now apply to both. 0.4.0 already listed doors, but in fact the sync never took effect there: for doors, the engine never calls the function it was attached to. NPCs still use doors on each PC separately, so whether a door is open or closed can differ when an NPC walks through.

## 0.4.0 – Mechanisms shared, world change without duplicate NPCs, dragons on both PCs (Gothic I u142, Gothic II u142, Othello u142; tag `v0.4.0` only, no separate release)

**New**

- **Mechanisms shared** (u137): Levers, stone plates and portals now work for both (doors only from 0.4.1), including the scripts and camera sequences behind them. Previously, for example, a portal stayed closed on the partner's side; after loading it was closed again for both, yet the item used on it was gone anyway. Fireplaces stay separate per PC (u138).

**Fixes**

- **World change** (u140, u141, Gothic 2): Whoever entered a world second got the partner's world file – and still the world's start script ran once more. NPCs then stood in the Valley of Mines twice, and the duplicates were not synced. The flags now come with the world file, and whatever the start script sets, the partner gets too. Saves that already contain duplicate NPCs stay that way.
- **Dragons** (u139, u140): The conversation makes a dragon mortal, but that only applied on the PC of the player who talked. Protection, guild and type of NPCs are now synced. Saves in which a dragon stayed immortal because of this are repaired when loading.
- **Crashes:** saving with the watch window open without `AtmosphericScattering` (u135, Gothic 2, renderer coop9); castle in the Valley of Mines, room with guild -1 (u136, Gothic 2); loading a save that was made during a camera sequence (u138); an item still needed by a running message had already been deleted, mostly on the client in Gothic 1 and Othello (u142, caught); camera of the world process in the first frame after a world change (u142).

## 0.3.1 – Gothic 1 and Othello tested automatically, fixes to the world process (Gothic I u134, Gothic II u134, Othello u134)

For the first time, the automated test run (host and client on one PC) also runs in Gothic 1 and Othello; the results are in the [README](README.md#test-status). Found and fixed along the way:

- **World process** (general, u134): If the partner stood still in the other world, the host's world process got no job and the watch window stayed empty. Even a short pause in network traffic reset the waiting time.
- **Othello** (u134): The world process and the test instance store their saves in the same folder as the main game (`SAVES_OTHELLOZ`, previously always `Saves`). The test run now also checks separate worlds under Othello.
- **Gothic 1, Othello** (u134): The flag "mine already entered" belongs to the world state of each PC and is no longer synced.

## 0.3.0 – Coop inventory, shared mission items, guilds per hero (Gothic I u106, Gothic II u106, Othello u106; tag `v0.3.0` only, no separate release)

**New**

- **Coop inventory** (u97–u103): a shared chest. The key (I by default, Settings › COOP) opens it directly without the hero having to move; both players can have it open at the same time. The contents are kept on the host and in the save, and items from it count for quests.
- **Shared mission items** (u96, u104, u106): If one player gets one, the partner has it too; if one hands it in, the partner loses it as well. The name says "(shared)", and the description explains it.
- **Guilds per hero** (u85): Guild dialogues are only shared between heroes of the same guild (including the reward); for the partner, the admission dialogue never counts as already held.
- **Auto-save** (u105): now also every 3 minutes.
- **World change under Othello** (u64, Othello only): The changes of the Othello Coop v107–v114 are included – the coop loads the new world itself if Union does not report the change, and three menu hooks are switched off under Othello.

**Fixes** (general)

- **Crashes and hangs:** crash when quitting via the Esc menu (u79); deleted items in NPC slots and while putting items down (u88–u90); a deleted sword in the hand after taking over a save (u91, u93); a hanging dialogue no longer blocks loading (u100); after dying at the world border of Gothic 2, the coop loads via the main menu instead of waiting forever (u91).
- **Game clock:** The host sends the time every 10 instead of every 60 seconds (u69); the clocks of the client and of the world process run in step with the host's to the second, and daily routines readjust after large time jumps (u83).
- **NPCs on the client:** Small deviations are evened out smoothly instead of with a jump, and large distances are set cleanly, so nobody gets stuck (u72, u74).
- **Items and chests:** Late deletions are caught up on, so no ghost items lie around until the next load (u70); chests in the world hand out items cleanly (u98).
- **Story videos:** Skipping for both also works when the video's internal number has ten digits (u87).

## 0.2.0 – First release as Gothic-I-II-Universal-Coop (Gothic I u62, Gothic II u62, Othello u62)

- Coop for Gothic 1 (1.08k), Gothic 2 Night of the Raven (2.6) and Gothic 1 with Othello 4.x from one shared source:
  players and NPCs; fights in which your own PC decides hits on your own hero and the client's PC computes the NPCs fighting it;
  dialogues, story and diary (quest XP for both); chests, items and looting; joint saves with preview and backup at the
  partner; separate worlds with world process and watch window; shared new game; story videos that can be skipped for both;
  COOP menu with character and 3D preview; coop via Steam; COOP difficulty; teleport rune to the partner; coop test run
  (Shift+F11).
- u62: Union 1.0m is included as files (in the installer and in the ZIPs for Gothic 1 and 2). The installer installs Union too
  if it is missing, moves `system\dinput.dll` from SystemPack to `CoopBackup` and restores both on undo (up to u61 it started
  the Union setup). New: a ZIP for Othello.
- u56–u61: The coop itself is unchanged compared to u55.

## History before the first release

### Gothic Union Coop u1–u61 (unreleased)

The Othello Coop grew into a Union plugin that covers Gothic 1, Gothic 2 Night of the Raven and Othello with one shared
source. The main steps:

- **u1–u7 – one core for all engines:** The coop's core is independent of the engine. All addresses in the engine are kept in a
  table, and each part of the coop is enabled for each engine separately. The coop can now also be built for Gothic 2, and in
  Gothic 1 it also runs without Othello (with its own main menu and loading). The new features of the Othello Coop up to v110
  are included, plus an automated coop test run.
- **u8–u20 – Gothic 2 Night of the Raven:** COOP menu with mouse control, shared new game with intro for both, coop renderer
  with 3D preview of the character and watch window, conversations (one player talks, the other listens – progress, diary and
  experience count for both), sync of the world, joint saves with preview, the combat system for two PCs (as described under
  0.2.0), COOP difficulty, teleport rune, partner marker and a self-check that all addresses match the engine.
- **u21–u28 – controls:** Esc menu with "Main menu" and "Choose save", diary with mouse control, a mouse cursor, the chapter
  image of your own guild, story videos for the partner too, and the partner on Gothic's map.
- **u29–u40 – separate worlds:** Each player can be in their own world (Khorinis, Valley of Mines, Jharkendar, Irdorath); a
  joint world change can be set up. The world process – a second Gothic in the background – shows the partner's world in the
  watch window. Plus teleporting between worlds and exchanging the world states when saving in different worlds.
- **u41–u55 – stable and installable:** crash fixes from test runs, an automated test run with host and client on one PC, the
  installer (detects the game by the folder contents, backs everything up, "Undo last installation"), the backup at the
  partner (everyone also gets the other player's save) and story videos that can be skipped for both.

### Othello Coop 0.1.0–0.1.2 (Gothic 1 with Othello 4.4.2.1 only, alpha)

- **0.1.2:** Teleport rune to the partner (costs no mana, its own look). Partner marker: name above the head (faded behind
  walls, with distance), an arrow at the edge of the screen, both players as arrows on Othello's world map. Fixed: jumps when
  NPCs switch their target between the players; errors in fights of NPCs hostile to the host; crashes on a side step in combat
  and during a cutscene shortly after loading. When loading, the state of the old world is discarded.
- **0.1.1:** Fixed a memory leak on the host (memory use grew with play time and every sync); the crash report no longer
  crashes itself; a more stable queue between network and game; no more crash when the partner switches spells.
- **0.1.0:** first public version – coop for Gothic 1 with Othello: COOP menu with partner character, renderer with watch
  window and character preview, connecting via IP or Steam.
