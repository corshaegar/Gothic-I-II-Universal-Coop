# Plan: allgemeines Coop für Gothic 1 und 2

Stand: **23.10.2026**, Ausgangsstand **Othello-Coop v106** (Projekt `C:\Gothic Union Coop`, Tag `othello-v106`). Fassung des
Plans vom 20.10.2026 (v50) ist im Othello-Repo und hier in der Geschichte (`git show othello-v106:docs/PLAN-ALLGEMEINES-COOP.md`).
Grundlage: `COOP-FIXES.md` (Bauanleitung, Fix-Katalog, G1→G2-Adresstabelle in Abschnitt 6), `MOD-UEBERBLICK.md` (wie die Mod
funktioniert), `GD3D11-UMBAU.md` (Renderer), `othello\FASSUNGEN-v1-v106.md` (Belege je Fassung).

**Auftrag (Nutzer, 23.10.2026):** Plan starten, alle Änderungen bis **v106** übernehmen, als Basis die GothicCoop-Fassung von
Gothic-Workshop in `GothicCoop\`, lokales Repo, das mit jedem Update nachgeführt wird. Ziele: **Vanilla Gothic 1** und
**Gothic 2 Die Nacht des Raben**.

## 1. Was seit dem Plan v50 dazukam (v51–v106)

Kurz, Einzelheiten im Fix-Katalog: Datentransfer und Bewegung (L1–L7), Treffer-Hoheit des Clients (C11), Kampf-Übergabe an den
Client (C18), K.o./Bestrafung/Zielwechsel (C13–C21), Schwierigkeitsgrad (H16), COOP-Menü mit Aussehen, Maus, 3D-Vorschau,
Verbindungsmenü in zwei Stufen, Deutsch/English (H8–H33), Beitritt aus dem Hauptmenü, gepackte Stände (H17–H20), Spielstände
„wasserdicht“ mit PC-Kennung, Manifest, Durchläufen und Vorschau (H22, H23, H29, H31, S13–S15), „Hauptmenü“ im Esc-Menü (H26),
Coop über Steam (H28), gemeinsames neues Spiel (H30), Absturz-Erfassung vor allen Filtern (A17–A20), Upstream-Fixes (A11–A14),
Teleport-Rune (N1), Mitspieler-Markierung mit Kartenpfeilen (O1–O4).

## 2. Ausgangslage (gemessen 23.10.2026 an `othello-v106`)

| Was | Umfang |
|---|---|
| Quellen `GothicCoop\` (ohne SDK) | 65 Dateien, 34.616 Zeilen; davon 3.784 Zeilen erzeugte Bilddaten (`CoopRuneAssets.h`, `CoopArrowTex.h`) und 1.412 Zeilen `CoopHookTable.h` |
| Ganz in `#if ENGINE == Engine_G1` | 37 Dateien, ≈ 25.900 Zeilen; für G2 nur leere Platzhalter im `#else` |
| Feste Engine-Adressen im Code | ≈ 144 verschiedene (je Datei gezählt, ohne Kommentare und ohne `CoopHookTable.h`); am meisten `Hooks.cpp` 35, `OthelloCompat.cpp` 23, `CoopMenuMouse.cpp` 13, `CoopWorldSync.cpp` 12, `CoopAudit.cpp` 12 |
| GT-Haken / Aufrufziele | 46 Haken; 320 zCall-Ziele, 51 push-Ziele (`tools\dllziele.py`, Build v106) |
| Othello-Bezüge im Code | 272 Erwähnungen in 40 Dateien; Othello-eigene Dateien `OthelloCompat.cpp`, `OthelloHookOrder.cpp`, `OthelloUnlock.cpp`; Skripte `scripts\othello\` |
| **G2A-Build** | baut **nicht**: 15 Fehler – das Protokoll (`OthelloCompatLog`, `DiagSeconds`, `DiagRole`) liegt nur im G1-Teil, `oCNpc::Interrupt` hat in G2A eine andere Signatur |
| Fix-Katalog | 163 Einträge: 68 als „allgemein“, 22 als „nur Othello“ gekennzeichnet, 73 noch ohne Zuordnung (meist vor v50) |
| Installationen | `Gothic\` G1 1.08k_mod, `Gothic II\` G2 NotR 2.6 (fix), je mit SystemPack, **ohne Union**; G1-Modkit `mdk\g1`, G2-Modkit `mdk\g2` (seit 24.10.2026; vorher fälschlich „in `Gothic II\_work`“) |
| Renderer | GD3D11 v17.9.7 + coop6 (`gd3d11-src`, Zweig `coop`); gebaut nur `Release_G1_AVX2` |

## 3. Zielbild: drei Schichten

1. **Kern** (spiel- und mod-neutral): Netz, Pakettypen, Protokoll/Log-Archiv/Log-Versand, Welt-/Truhen-Abgleich (E),
   gemeinsames Speichern (F, S), Dialoge und Story (D), Fern-Abgleich (I), Kamera (K), Diagnose/Selbsttest (G), Kampf-Regeln (C),
   Menü-Logik (H), Steam (H28), Markierung (O1/O2).
2. **Engine-Schicht** je Spiel (G1, G2A, ggf. G2 Classic): Haken-Adressen, Aufrufstellen, vtable-Plätze, globale Daten, feste
   Feld-Offsets und Bits (`+0x79C`, `state` an `+0x470`, Listen-Offsets in `oCWorld`), Engine-Unterschiede (G2: eine
   Inventarliste, `aiscriptvars[100]`, andere Signaturen). Quelle: `CoopAddr.h` (neu) mit Spalten G1/G1A/G2/G2A, je Adresse gegen
   die Exe bzw. ein Speicherabbild geprüft.
3. **Mod-Profil** je Spiel/Mod (Othello, Vanilla-G1, Vanilla-G2A, …): Erkennung, Skriptnamen mit Rückfallwerten, Erwartungsliste
   der Engine-Eingriffe für die Selbstprüfung, Menü-Skript, Figur-Skript, Gegenstände (`COOP_ITEMS.D`), Aussehen-Tabellen,
   `coopStoryIgnore`, Sonderfälle (Othello: B1–B8, verlegtes Gildenfeld `+0xA34`, `ZS_SAW`, Karte, Tagebuch, Bildfilter).

## 4. Grundsatz: Othello-Befunde gelten nur für Othello

Othello verändert die Engine (über `OTHELLO_ABI.DLL`) und seine Skripte an vielen Stellen. Diese Änderungen gibt es in Vanilla
und in anderen Mods **so nicht** – dort können andere stehen. Jeder Befund ist als **„nur Othello“** oder **„allgemein“**
gekennzeichnet; nichts wird ungeprüft übertragen. Bekannte Othello-Eingriffe, die GT berühren (Stand v106):

| Othello-Eingriff | Wirkung | GT-Antwort (Datei) | In Vanilla |
|---|---|---|---|
| `MakeSpellBook`, `UseStandAI`, `oCSpawnManager::DeleteNpc` umgewidmet (B1) | fremder Code an der Original-Adresse | Nachbildung, sonst Original (`OthelloCompat.cpp`) | Original benutzen |
| `oCWorld` vergrößert, Objektlisten verlegt, je Start anders (B2) | feste Offsets falsch | Offset aus dem Engine-Befehl | SDK-Offset |
| `CanUse`-Haken ohne Weiterreichen (B3), Haken-Reihenfolge | GT-Haken übergangen | `OthelloHookOrder.cpp` | entfällt |
| Overlay-Nummern ≥ 100 nur lokal (B4) | falsche Overlays | per Name senden | prüfen |
| `CanParade` = Platzhalter „immer 1“ (B5) | Parieren | Host entscheidet | Original |
| vtable `DoTakeVob` = `return 0` (B6), vtable `ResetPos` → 2 Argumente (B7) | Aufheben unsichtbar, Stapel kaputt | Welt → Inventar beobachten; Original direkt | vtable prüfen |
| Plugin-Sperre „Remove the following unsupported plugins“ + 5 verdorbene Engine-Bytes (B8) | Start mit GT unmöglich | `OthelloUnlock.cpp` (im Speicher abschalten) | entfällt |
| Gildenfeld verlegt `+0x7A6` → `+0xA34` (`GetTrueGuild`, `IsHuman`, `IsMonster`, `GetPermAttitude`) | SDK-Feld falsch | nur über Funktionen lesen | SDK-Feld |
| HP-Abzug in `OnDamage_Hit` über `OTHELLO_ABI+0x7E1E0` (C8, C18b), `OnDamage_Script` mit Vorbau (C10) | Schadensmessung, doppelter Abzug | Haken auf `ChangeAttribute`, Wahrnehmung ergänzen | neu messen |
| `DoAIState` löscht Wahrnehmungen über `OTHELLO_ABI+0x79A40` (C10); `ZS_SAW` ohne Wahrnehmungen | Reaktion | Wahrnehmung ergänzen | Engine selbst; Skripte prüfen |
| `CollectFocusVob` umgeleitet (C5), `aivar[49]` = `AIV_BOSSBEHAVIOR` (C6) | Fokus, Immunitäten der Figur | Wiederbeleben ohne Fokus, Figur-Skript | `aivar[49]` frei |
| `oCGuilds::SetAttitude` ohne Grenzprüfung (G6) | – | Audit kennt es | Original |
| `CreateHit`-Aufruf in `HitCombo` über `OTHELLO_ABI+0xC4B70` (C11) | Treffer-Erzeugung | dieselbe Aufrufstelle | Original |
| Test-Modus: Shift+F3 = Noclip (H7) | traf „Figuren neu“ | im Test-Modus gesperrt | Engine belegt F2–F12 ebenfalls |
| `StartAIState`: `CloseCutscenes` → `ret`; `ClearEM` → `OTHELLO_ABI+0x7B610` (C13) | ohne Wirkung für GT | über die Eingänge | Original |
| Menüs `MENU_MAIN`/`MENU_OPTIONS` aus Othellos `MENU.DAT`; `zCMenu::SetByScript` → `OTHELLO_ABI+0x9CC60`; „Gothic beenden“ auf 5525 (H25, H26b) | COOP-Einträge zur Laufzeit | `CoopMenuInject.cpp` | Vanilla-Menünamen/-Lagen aus dem Modkit |
| `zCMenu::HandleEvent` → `OTHELLO_ABI+0x9CEE0`, `zCView::InsertBack` → `+0x831D0` (H9) | Maus | nur gelesen bzw. durchgereicht | Original |
| `MENU_OTHELLO_CONTROLS` ohne Einträge (H14) | Absturz in `GetActiveItem` | eigene Bereichsprüfung (allgemein nötig) | – |
| Bildfilter `zCView::Blit` → `OTHELLO_ABI+0x83240`, Merker `+0x137BDF` (H26d) | Startbild verschluckt | Merker umgehen | entfällt |
| Texturen/Köpfe: Gesichter 0–164, Körper 0–14, 14 Kopfformen, rote 4×4-Platzhalter (H8, H15) | Auswahl im COOP-Menü | Tabelle in `CoopLook.cpp`, Größe statt Name zählen | aus den VDFs neu bestimmen |
| `WORLD_INITIALISE`: Glatze bei `HERO_SHAVED_HEAD` (H8) | überschreibt Aussehen | nach jedem Laden neu anlegen | entfällt |
| 15 Spielstand-Slots, `oCSavegameInfo` auf 0xA0 vergrößert (H23, H31) | Liste, Vorschau, Slots 6–9 für Coop | Slots aus der Engine, Größe zur Laufzeit | Slotzahl/Klassengröße prüfen |
| Eigener Tagebuch-Bildschirm (H27b, H27c) | Klick, Esc | `CoopMenuMouse.cpp` | Vanilla-Tagebuch prüfen |
| Schwierigkeit beim neuen Spiel, Diego-Start, Spielstart-Bonus, Ruf (H30, D17, D18) | gemeinsames neues Spiel | `CoopNewGame.cpp`, `CoopDialogSync.cpp` | G1: Diego ohne Othello-Teile; G2: Xardas |
| Dialog-Überspringen nach dem Laden (D14) | falsches Überspringen | Merker beim Laden | prüfen |
| Karte mit eigenen Marken (Liste bei `OTHELLO_ABI+0x156324`, Zeichnen über `+0x11660`) (O3, O4) | Kartenpfeile | `CoopPartnerMarker.cpp`, Bytes geprüft | G1: keine Marken; G2: eigene Kartenposition prüfen |
| Eigener Absturz-Filter `OTHELLO_ABI+0x70810`, ruft Gothics Filter nicht (A18) | Absturz unsichtbar | eigener Filter jedes Bild davor, VEH (A20) | Gothics `zCExceptionHandler` |
| Schriften skaliert ein Plugin (Faktor bei `0x7079C0F4`) (H33b) | Textbreiten | mit `zCFont::GetFontX` messen (allgemein richtig) | – |
| Begleit-Plugins: zUtilities (C8), zParserExtender (Menü-Skripte, steam_api schon geladen – H28), SHW32 (Spielstände) | messen/zeichnen mit | berücksichtigt | zParserExtender wird gebraucht (`.D` in Autorun) |
| **zNoFriendlyFire** (aus `gothictogether.vdf` = GT-Paket, **kein** Othello) | filtert Schaden | abschaltbar, GT braucht es nicht | nur mit GT-Paket |

**Allgemein**, obwohl unter Othello gefunden: `other == hero` in `B_MM_ReactToCombatDamage` und `B_InterceptAssessDamage` (C9),
Löschen der Wahrnehmungen bei jedem Zustandswechsel (Engine), `ZS_ProclaimAndPunish` mit `hero` (C13), endlose Dreh-Animationen
(L7), `GetActiveItem` ohne Bereichsprüfung (H14), `stayOnTop` schneidet die `screen`-Liste ab (H13), Cutscene-Player ohne Manager
(A19), `GetProgressPercent` ohne laufende Animation (A17), Host-Paketleck (A11).

**Arten von Mods** (bestimmt den Aufwand je Profil): 1. nur Skripte – klein; 2. eigene DLL/Union-Plugins wie Othello – mittel bis
groß; 3. Laufzeit-Patches aus den Skripten (Ikarus/LeGo, z. B. **Returning New Balance**, noch nicht geprüft) – groß, Eingriffe
erst nach dem Laden eines Stands sichtbar, Zeiger-Variablen aus dem Story-Abgleich heraus. Für unbekannte Mods soll GT selbst
warnen: Selbstprüfung findet Eingriffe an Funktionen, die GT benutzt, und kein Profil kennt sie → Hinweis im Log und beim
Verbinden.

## 5. Entscheidungen (23.10.2026)

| Frage | Entscheidung |
|---|---|
| Ziel nach Othello | **Vanilla G1 und G2 NotR** (Nutzer). Reihenfolge: G1 zuerst (gleiche Engine, Phase 2), dann G2A (Phase 3) |
| Installationen | `C:\Gothic Union Coop\Gothic` und `…\Gothic II` auf diesem PC. **Offen:** dieselben auf dem anderen PC; Union richtet der Nutzer selbst ein |
| Eine DLL je Spiel oder gemeinsam | **je Spiel eine** (`Gothic1Coop.dll` / `Gothic2Coop.dll`, Konfigurationen „G1 MD Release“ / „G2A MD Release“) – wie GT; „MP x2“ erst, wenn beide laufen |
| Fassungsnamen | `uNN` (Union-Coop), Tag je Build; Othello-Fassungen `vNN` laufen im Othello-Repo weiter |
| Othello im neuen Code | bleibt als Profil; jeder Umbau in Phase 1 muss Othello unverändert lassen |
| v107 (Testlauf mit 100 Schritten) | u6: v107–v110 übernommen (Nutzer: „die 100 Tests auch hier“); Ruf-Schritte nur Othello, sonst Gothic 1 tauglich. Für G2A noch zu prüfen (Skript-Namen, Gilden, Testgegenstände) |

## 6. Phasen

### Phase 0 – Regeln für jeden neuen Code (gilt weiter)
- Jeden Befund in `COOP-FIXES.md` als „nur Othello“ / „allgemein“ kennzeichnen.
- Othello-Eigenes nur im Othello-Profil bzw. hinter `CoopProfileIsOthello()`, im Kommentar „Othello:“.
- Skriptnamen nie fest verdrahten: über den Parser mit Rückfallwert und Log (Muster `CoopAivIndex`).
- Engine-Zugriffe über SDK-Methoden; jede neue feste Adresse in `CoopAddr.h` mit G2A/G2-Spalte (bzw. „prüfen“).
- Jede neue Funktion bekommt einen Schritt im Selbsttest, der auch ohne Othello passt.
- Nach jedem Build `tools\dllziele.py` gegen die vorige Fassung; Commit + Tag `uNN`.

### Phase 1 – Trennung in der G1-Fassung, ohne Verhaltensänderung (Aufwand: mittel bis groß)
**Stand 23.10.2026:** Schritte 1–5 erledigt: u1 (Kern, Profil, G2A baut), u2 (`CoopAddr.h`), u3 (Freigabe je Modul statt
G1-Klammern, Prüfbau für G2A fehlerfrei, Feldabstände über SDK-Namen bzw. belegt; COOP-FIXES P1–P10). Offen in Phase 1: Schritt 6
(Selbstprüfung/Stempel aus dem Profil) und 7 (Katalog kennzeichnen) sowie der Spieltest unter Othello. Freigabe der Module für
G2A: `G2A-FREIGABE.md` (Prüfliste je Modul). Union ist in beiden Vanilla-Installationen eingerichtet – Phase 2 kann beginnen.
Reihenfolge so, dass jeder Schritt einzeln baut und geprüft wird:
1. **Kern-Protokoll engine-neutral** (u1): `OthelloCompatLog`, `DiagSeconds`, `DiagRole`, Log-Archiv aus den G1-Klammern lösen.
   Unter Othello bleibt der Dateiname `GothicCoopOthello.log`, sonst `GothicCoop.log`.
2. **Profil-Schnittstelle** `CoopProfile.cpp` (u1): Erkennung beim Start (Othello: `Data\ScriptsOthello4.0*.vdf` vorhanden bzw.
   `OTHELLO_ABI.DLL` geladen; sonst Vanilla-G1 / Vanilla-G2A nach `ENGINE`), Name, Stempel, Logzeile; danach die Othello-Teile
   schrittweise dahinter (Karte, Bildfilter, Plugin-Sperre, Menü-Einhängen, Tagebuch, Diego/Bonus/Ruf, Schadensabzug).
3. **Engine-Tabelle** `CoopAddr.h` (u2): alle festen Adressen mit Namen, Spalten G1/G1A/G2/G2A (`zSwitch`), Quelle je Zeile (SDK,
   Exe, Speicherabbild); G2-Werte aus `COOP-FIXES.md` Abschnitt 6 und `tools\g1g2map.py`, Aufrufstellen/vtables/Globals aus
   `Gothic2.exe` (statisch, `tools\gdis.py`). Code greift nur noch über Namen zu.
4. **Feste Feld-Offsets/Bits** sammeln und über SDK-Felder oder Laufzeit-Erkennung lesen (wie `oCWorld::voblist_npcs`).
5. **G1-Klammern auflösen**: jede Datei baut für G2A; was noch nicht portiert ist, meldet in G2 „nicht unterstützt“ im Log statt
   still leer zu laufen. Ziel u3: „G2A MD Release“ baut.
6. **Selbstprüfung und Stempel aus dem Profil**: Erwartungsliste (`CoopAudit.cpp`), Menü-/Stempelprüfung (`CoopMenuLobby.cpp`),
   Fassungsprüfung beim Verbinden um Spiel + Profil erweitern.
7. **Fix-Katalog durchgehen**: die 73 Einträge ohne Zuordnung kennzeichnen.
- **Prüfung:** `tools\dllziele.py` gegen die vorige DLL – gleiche Engine-Ziele bis auf neue Hilfsfunktionen; unter Othello
  Spieltest (Kampf nah/fern, Dialog, Speichern/Laden, Zuschau-Fenster, Menü) – nur auf Ansage des Nutzers einspielen.

### Phase 2 – Vanilla Gothic 1 (Aufwand: mittel)
**Stand 23.10.2026 (u5):** Skripte `scripts\g1\` (erzeugt von `tools\skripte_g1.py`, geprüft gegen `MENU.DAT`/`GOTHIC.DAT`),
Code je Profil (COOP-FIXES P11–P16: Laden mit `SAVEGAME_LOAD`, Hauptmenü, Menü-Zeiger, Aussehen, Dialog-Sonderwege) und u5
eingespielt: `Gothic\system\Autorun\` (`Gothic1Coop.dll`, drei `.D`) und `Gothic\system\GothicCoopConfig.json`. Offen: erster
Start (Log `system\GothicCoop.log`: Profil, Menü-Einhängen, Zeiger, Selbstprüfung), Speicherabbild, Skriptregeln (unten),
Spielrunde zu zweit. Nicht für Gothic 1 da: Zuschau-Fenster/3D-Vorschau (kein Coop-GD3D11), Esc-Menü-Einträge (Gothic 1 hat
kein `MENU_MAIN_INGAME`), GRawInput (Zeiger evtl. in der Fenstermitte gefangen).
- Braucht: Union in `Gothic\` (Nutzer), auf beiden PCs; zParserExtender für die `.D`-Skripte (prüfen, ob im Union-Paket).
- Speicherabbild (`tools\memdump.py <ordner> GothicMod.exe`), Selbstprüfung und `gtcalls.py` gegen dieses Abbild.
- Profil Vanilla-G1: `scripts\g1\COOP_MENU.D` (Vorlage `mdk\g1\…\Scripts\System\Menu`), `PC_MULTIPLAYER_HERO.D` ohne
  `aivar[49]`, `COOP_ITEMS.D` mit G1-Runenmodell, Aussehen-Tabelle aus Gothics VDFs (`tools\texreal.py`).
- Skriptregeln gegen die G1-Original-Skripte prüfen (Modkit): C3/C9 (`B_CombatReactToDamage`, `B_MM_ReactToCombatDamage`,
  `B_InterceptAssessDamage`), Kill-EP in `ZS_Dead`, D8 `B_LogEntry`, Zustände ohne Wahrnehmungen, Diego-Start (H30c).
- **Prüfung:** Selbsttest + Spielrunde.
- **Vermerk für Gothic 1 (Nutzer 23.10.2026, Test u21):** Die G1-DLL (gebaut, `Gothic\` noch mit u7) hat seit u21/u22 dieselben
  Menü-Änderungen wie G2 – beim nächsten Einspielen in Gothic 1 prüfen:
  - Mauszeiger = Spielerpfeil der G2-Karte (`O-C.TEX` aus Gothic 2, nicht das leicht andere aus Gothic 1), 30° geneigt, dünner
    dunkler Rand, Größe Bildhöhe / 36 (P45, P47).
  - Markierter Eintrag grün statt leuchtend, auch Tagebuch-Themen (P48, kräftiger ab u23: P49).
  - Auswahlfelder mit „<“ und „>“ (P50).
  - MENU_MAIN ist in Gothic 1 auch das Esc-Menü: Einträge nur im Spiel/nur beim Start ausgeblendet, Zeilen lückenlos, nach
    Neuaufbau des Menüs wieder eingehängt (P42, P46); im Spiel ohne Neues Spiel/Intro/Weiterspielen, „Hauptmenü“ vor „Beenden“
    (P53).
  - Tagebuch-Themen direkt mit der Maus (P43).
  - Rune: G1 behält das eigene Modell aus v106 (grünes F), das neue Bild ist nur für Gothic 2.

### Phase 3 – Gothic 2 Die Nacht des Raben (G2A, Vanilla) (Aufwand: groß – größter Block)
**Stand 23.10.2026 (u11):** Grundlauf, Menü, Steam (u8), Neues Spiel, Renderer mit Vorschau und Zuschau-Fenster (u10), Dialog
(u11). Vollständige Liste des Fehlenden mit neuer Reihenfolge: **`G2-FEHLT.md`** (ersetzt die Reihenfolge unten).
Ursprüngliche Reihenfolge der Freigaben (je Gruppe prüfen, bauen, Spieltest):
1. **u8 – Grundlauf + Menü + Steam** (erledigt, ungetestet): Trace, DiagClient, MenuInject, Lobby, Mouse, Look, Steam
   (COOP-FIXES P17–P21, `G2A-FREIGABE.md`). Skripte `scripts\g2\` (Menü, Figur), Steam-App 39510. Jeder lädt seinen Stand.
2. Speichern-Gruppe (Save, SavePreview) + NewGame: Slots/Ordner G2, Welt-Dateien (Addon), `COOPHERO.TXT` mit 100 aivar,
   Spielbeginn bei Xardas, Intro.
3. Dialog, DialogWatch, World, FarSync: Story-Abgleich mit G2-Skripten (`B_LogEntry`, Kapitel), Truhen/Schlösser G2.
4. Kampf-Gruppe (HooksExt, Damage, Hit, CombatLead, Difficulty): G2-Schadensablauf messen, `CreateHit`-Kopf G2A.
5. Teleport (G2-Rune, `COOP_ITEMS.D` G2), Marker, SelfTest (G2-taugliche Schritte), Audit (eigene G2-Haken-Tabelle).
6. Coop-Renderer für G2 (`g2a_avx2.dll` mit coop6) → Preview, Camera.
Für jede Stufe: Speicherabbild des laufenden G2A (`tools\memdump.py <ordner> Gothic2.exe`) und Selbstprüfung, sobald Audit
für G2 steht. G2-Inhaltsskripte als Quelle seit 24.10.2026 im Modkit `mdk\g2\_work\Data\Scripts\Content` (vorher nur
kompiliert, `GOTHIC.DAT`); Menü-Skripte auch in `Gothic II\_work\Data\Scripts\System\Menu`.
- Braucht: Union in `Gothic II\` (Nutzer), auf beiden PCs; G2-Modkit in `mdk\g2` (Skripte in `_work\Data\Scripts\Content`);
  Speicherabbild G2A.
- Engine-Tabelle G2A: jede Zeile nach der Prüfliste (`COOP-FIXES.md` Abschnitt 2) gegen `Gothic2.exe` und das Abbild prüfen,
  vtable-Plätze neu bestimmen (`DisableVob`, `IsHuman`, `ResetPos`, `InitMenuItem` …), Globals (`ogame`, `ztimer`, Tastenpuffer).
- Engine-Unterschiede: Inventar (eine Liste, `CoopForEachInvItem`), `aiscriptvars[100]`, `Interrupt`-Signatur,
  Spielstand-Slots, Felder; Upstream `dad4501` (`IsNpcDead`, `EndMovement` für G2A).
- Skripte G2 lesen: Schadensreaktion, Monster, Kill-EP, Tagebuch, Wahrnehmungen je Zustand, Info-Abfragen beim Angriff, Start bei
  Xardas statt Diego.
- Menü und Figur für G2 (Menü-Skript aus `_work\Data\Scripts\System\Menu`, Körper/Kopf der Figur), Teleport-Rune als G2-Rune.
- Renderer: coop6 auf den G2-Build von GD3D11 (`Release_AVX2`, Quelle `gd3d11-src`) übertragen, Zweitbild und Schatten prüfen.
- **Prüfung:** Selbsttest (G2-taugliche Schritte), Spielrunde.

### Phase 4 – Gothic 2 Classic (optional, niedrige Priorität)
Nur auf Wunsch: Spalte G2 der Tabelle, sonst wie Phase 3.

### Phase 5 – weitere Mods als Profile (Aufwand je nach Art der Mod)
Je Mod **und je Mod-Fassung**: Speicherabbild im laufenden Spiel (nach dem Laden eines Stands) + Erwartungsliste, Skript-DAT
lesen, Menü, Figur, Story-Ausnahmen. Nichts aus dem Othello-Profil übernehmen, ohne es in der Ziel-Mod nachzuweisen.
Ikarus/LeGo: Zeiger-Variablen (`MEM_*`, `_@`, Handles) aus dem Story-Abgleich, LeGo-Haken an GT-Eingängen erkennen.

### Phase 6 – Paket und Verteilung (Aufwand: klein bis mittel)
- Je Spiel ein Ordner: DLL, Skripte des Profils, `GothicCoopConfig.json` (ohne sie bricht GT ab), Coop-GD3D11,
  `installieren.bat` / `original-zurueck.bat` / `BITTE-LESEN.txt` (Muster `gd3d11\verteilen\`); `tools\makezip.py` je Spiel.
- Kurze Anleitung für Spieler (Installation, Tasten, Menü, Grenzen).

## 7. Offene Punkte (aus dem Othello-Coop, Stand v106)

| # | Punkt | Stand |
|---|---|---|
| O1 | Skript-Trigger, die auf beiden PCs feuern (doppelte Folgen beim Host) | offen, allgemein |
| O2 | `Wld_InsertItem`/`Mob_CreateItems`, Beschwörungen des Clients laufen lokal | offen, allgemein |
| O3 | Gleichnamige Spawns tauschen beim Client die Rollen (Zuordnung über die Lage) | offen, allgemein |
| O4 | `RemoteNpc`-Objekte werden nie gelöscht (Upstream N1) | klein, offen |
| O5 | Upstream-Funktionen F1–F5 (Zauberstufe, Fackel, Türen/Hebel, Uhrzeit 5 s, Fallenlassen) | nur nach Prüfung je Engine |
| O6 | `c0000409` in `KERNELBASE.dll` 10 s nach dem Start (Othello), ohne Folgen | Ursache offen – in Vanilla beobachten |
| O7 | Grenzen laut `COOP-FIXES.md` Abschnitt 7 (Partikel im Zuschau-Fenster, Treffer-Näherung, Story-Takt …) | unverändert |
| O8 | Synchronisation über Welten (nächstes großes Thema im Othello-Coop) | gleich allgemein planen |

## 8. Pflege

Dieser Plan wird bei jedem Schritt nachgeführt (Abschnitte 2, 4, 6, 7); Fix-Einträge stehen in `COOP-FIXES.md`, der Verlauf in
`CHRONIK.md`, Builds in `..\builds\INHALT.md`.
