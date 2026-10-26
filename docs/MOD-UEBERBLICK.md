# Othello-Coop — technischer Überblick (Einstieg für ein neues Fenster)

Diese Datei erklärt **wie die Mod funktioniert**, damit man ohne Vorgeschichte mitreden kann. Kein Arbeitsstand – der steht in
`..\NEUES-FENSTER.md` (Regeln, Chronik). Fix-Katalog mit Engine-Adressen: `COOP-FIXES.md`. Je Fassung ein Abschnitt: `README.md`.

## 1. Was es ist

Ein **Union-Plugin** für **Gothic 1** mit der Mod **Othello 4.4.2.1**, entstanden als Eigenbau aus dem quelloffenen
**GothicCoop / Gothic Together** („GT“, <https://gitlab.com/gothiccoop/gothiccoop>). Zwei Spieler spielen dieselbe Welt.
Die meisten Änderungen sind **allgemeine GT-Fehler oder fehlende Funktionen** und gelten auch für Vanilla-G1 und (mit anderen
Adressen) G2; Othello-spezifisches ist in der Doku als „nur Othello“ gekennzeichnet.

Alle Quellen liegen in `gothiccoop-src\` und werden über **`Sources.h` in eine einzige Übersetzungseinheit** eingebunden –
die Reihenfolge dort zählt (wer wen sehen kann). Engine-Funktionen kommen aus dem Union-SDK
(`ZenGin\Gothic_I_Classic\API\*.h`, `zCall(Adresse)`), eigene Haken über `CInvoke<…>(Adresse, &Funktion)`.

## 2. Dateien im Spiel

| Datei | Rolle |
|---|---|
| `system\Autorun\Gothic1Coop.dll` | das Plugin selbst (Union lädt jede DLL in `Autorun`) |
| `system\Autorun\COOP_MENU.D` | Menü-Skript (zParserExtender, `Parser = Menu`): COOP-Menüs und alle Einträge, Windows‑1252 |
| `system\Autorun\PC_MULTIPLAYER_HERO.D` | Skript-Instanz für die Figur des Mitspielers |
| `system\Autorun\COOP_ITEMS.D` | ab v102: Coop-Gegenstände (Game-Parser), bisher die Teleport-Rune `ITRU_COOP_TELEPORT` |
| `system\GD3D11\Bin\g1_avx2.dll` | Renderer, eigener GD3D11-Build „coop6“: Zuschau-Fenster + 3D-Figur-Vorschau |
| `system\GothicCoopConfig.json` | Grundeinstellungen; **fehlt sie, bricht das Plugin mit „No config file found“ ab** |
| `Gothic.ini [GOTHICCOOP]` | alles, was im Menü einstellbar ist (Gothic schreibt selbst) |
| `gamemodes\GothicCoop\` | Kopie von DLL und `.D` (GTs Launcher kopiert von dort nach `Autorun`) |

In `Autorun` darf **genau eine** GT-DLL liegen. DLL, `COOP_MENU.D` und (ab v102) `COOP_ITEMS.D` gehören zusammen; **beide PCs brauchen dieselbe Fassung**
(der Versionsvergleich beim Verbinden lehnt sonst ab).

## 3. Grundprinzip: der Host führt

- Netz: **ENet**, Inhalte als JSON → BSON. Client → Host; der Host verarbeitet (`ProcessServerPacket` → `ProcessCoopPacket`)
  und verteilt Client-Pakete automatisch weiter (`ReadyToBeDistributedPackets`), eigene Host-Pakete per `ReadyToSendJsons`.
- **Konvention (ab v13):** Ein Client schickt seine Anfrage **ohne** Feld `by`. Der Host handelt und verteilt das Ergebnis
  **mit** `by` (= Verursacher). Clients reagieren **nur** auf Pakete mit `by`. So wird die automatisch weitergeleitete
  Client-Fassung ignoriert und niemand wendet etwas doppelt an; `by == eigene Kennung` heißt „habe ich selbst ausgelöst“.
- Jedes Weltpaket trägt `w` (Weltdatei) – Pakete aus einer anderen Welt gelten nicht.
- NPCs werden über **eindeutige Namen** adressiert (`BuildGlobalNpcList`, `NpcToUniqueNameList`), nicht über Zeiger oder Indizes.
- **Ausnahme von „Host führt“:** Kampf gegen einen Client. NPCs, die gegen ihn kämpfen, rechnet sein PC
  (`CoopCombatLead.cpp`, Paket 54), und Treffer auf seinen Helden entscheidet er selbst (`CoopHitAuthority.cpp`, Paket 52).
  Heldenwerte (Ausrüstung, Boni, EP, Gilde) bleiben immer beim eigenen Helden.
- Pakettypen ab 23 sind unsere; die Tabelle mit allen Feldern steht in `COOP-FIXES.md` Abschnitt 4.

## 4. Takt

Union ruft aus der Engine:

- `Game_Loop` (im Spiel, je Bild) → Pakete, Schwierigkeit, neues Spiel, Kamera, Sprache …
- `Game_MenuLoop` (**jedes Menübild**, auch im Hauptmenü ohne geladenes Spiel) → Lobby, Steam, Menü-Maus, Vorschau,
  Spielstand-Liste, Sprach-Umschaltung. Union ersetzt dafür `zCMenu::Render` in allen Menü-vtables.
- `Game_SaveBegin/End`, `Game_LoadBegin/End` → Spielstände, Rücksetzen skalierter Werte.

Im Hauptmenü gibt es **keine Welt und keinen Helden** (`player == NULL`, `ogame` existiert schon). Die Wartephase
(„Lobby“) verarbeitet darum nur ihre eigenen Pakete (`SYNC_LOBBY`), den Speicher-Abgleich und den Log-Versand.

## 5. Teile und Zuständigkeiten

| Datei | Aufgabe |
|---|---|
| `Plugin.cpp`, `UnionHooks.cpp`, `DllMain.cpp` | Einstieg, Takt, Tasten (Shift+F1…F4), Anzeige oben links |
| `Client.cpp`, `Server.cpp`, `PacketProcessor.cpp`, `SafeQueue.cpp` | ENet-Fäden, Paketverteilung |
| `LocalNpc.cpp`, `RemoteNpc.cpp`, `PlayerAnimation.cpp` | NPC-Zustände senden/anwenden, Figuren, Bewegung glätten |
| `CoopWorldSync.cpp` | Truhen, Gegenstände am Boden, Leichen – der Host führt den Bestand |
| `CoopSaveSync.cpp` (größte Datei) | gemeinsames Speichern/Laden, Archiv, Stand übernehmen, F9-Liste |
| `CoopDialogSync.cpp` | Dialoge (einer je NPC, beobachtbar), Quest-Fortschritt, Tagebuch, Kapitel, Spawns/Trigger über den Host |
| `CoopCombatLead.cpp`, `CoopHitAuthority.cpp`, `CoopDamage.cpp`, `Hooks.cpp` | Kampf: Übergabe, Treffer-Hoheit, Schaden skalieren |
| `CoopFarSync.cpp` | Abgleich außerhalb der Reichweite (Verfolgungspunkt, Tode, EP-Reichweite) |
| `CoopDifficulty.cpp` | Schwierigkeitsgrad und Abgleich-Regeln; **der Host gibt sie vor** (Paket 53) |
| `CoopMenuLobby.cpp` | COOP-Menü: Verbinden, Status, Spielstand-Liste, Haken auf `zCMenu::HandleSelAction` |
| `CoopMenuInject.cpp` | hängt die COOP-Einträge **zur Laufzeit** in Othellos `MENU_MAIN`/`MENU_OPTIONS` ein |
| `CoopMenuMouse.cpp` | Maus in allen Gothic-Menüs (Zeiger, Überfahren, Klick, Rad) |
| `CoopLook.cpp`, `CoopPreview.cpp` | eigenes Aussehen (+ NPC-Suche), 3D-Vorschau über den Coop-Renderer |
| `CoopSavePreview.cpp` | Vorschaubilder und Angaben beider PCs in der Spielstand-Liste |
| `CoopSteam.cpp` | Coop über Steam: Tunnel unter ENet über Steam-P2P, Rich Presence, Freundesliste |
| `CoopNewGame.cpp` | gemeinsames neues Spiel (Intro für beide, Spawn nebeneinander, Diego spricht zuerst beim Host) |
| `CoopCamera.cpp`, `CoopTitle.cpp` | Zuschau-Fenster (Shift+F5), „Hauptmenü“ im Esc-Menü |
| `CoopTeleport.cpp`, `CoopRuneAssets.h` | Teleport-Rune zum Mitspieler (Ziel über `AI_TELEPORT`, Mana 0, Rune nach jedem Laden); ab v106 eigenes Aussehen (Modell + Textur aus `tools\runetex.py`, zur Laufzeit angelegt, Konstante `COOP_RUNE_VISUAL` in `COOP_ITEMS.D`) |
| `CoopPartnerMarker.cpp`, `CoopArrowTex.h` | Mitspieler-Markierung: Name über dem Kopf (hinter Wänden blass), Pfeil am Bildrand; auf Othellos Karte rote Pfeile beider Spieler (Textur aus `tools\arrowtex.py`, unsichtbare Othello-Marken für Lage und Namen, `OTHELLO_ABI`-Bytes geprüft) |
| `CoopCore.cpp` (u1) | Fassungsname `COOP_FASSUNG`, Profil (Othello/Vanilla, `CoopProfile()`), Hauptprotokoll, Log-Archiv, Sprache (`CoopTr`), `COOP_ENGINE_SWITCH` – alle Engines |
| `OthelloCompat.cpp` | Ersatz für von Othello umgewidmete Engine-Funktionen, Othellos Absturzbericht (nur Othello) |
| `OthelloHookOrder.cpp`, `OthelloUnlock.cpp`, `Utils.cpp` | Haken-Reihenfolge gegen Mod-Haken, Othello-Besonderheiten, Robustheit |
| `CoopAudit.cpp`, `CoopDiag*.cpp`, `CoopTrace.cpp`, `CoopSelfTest.cpp`, `CoopLogShip.cpp` | Selbstprüfung, Diagnose, Testlauf (Shift+F11), Logs zum Partner |

## 6. Menüs

- `COOP_MENU.D` legt eigene Menüs an (`MENU_COOP`, `MENU_COOP_LOOK`, `MENU_COOP_IP`, `MENU_COOP_LOAD`, `MENU_COOP_STEAM`,
  `MENU_COOP_NG`, `MENU_OPT_COOP…`). Die Einträge lösen `SEL_ACTION_EXECCOMMANDS` mit `COOP_*` aus; GT fängt
  `zCMenu::HandleSelAction` ab und behandelt sie. Othellos eigene Menüs werden **nicht** ersetzt, sondern zur Laufzeit ergänzt.
- Texte, die GT selbst setzt (Status, Listenzeilen, Knopftexte), gehen über `zCMenuItemText::SetText`. **Achtung:** `SetText`
  ändert `m_parText[0]` nicht – nie damit vergleichen, sonst bleiben alte Texte stehen.
- **Sprache:** `Gothic.ini [GOTHICCOOP] coopLang` (0 Deutsch, 1 English). Jeder eigene Menüeintrag trägt sein Englisch selbst
  in `userString[2]` (Text bzw. Optionen) und `userString[3]` (Infozeile); GT tauscht zur Laufzeit. Jeder Text aus dem Code
  läuft über `CoopTr(deutsch, english)`. **Logs bleiben immer deutsch.**
- **Textbreiten:** Gothic setzt Menüs in der Auflösung aus `Gothic.ini` (hier 640×480). Breiten mit der Schrift messen
  (`zCFont::GetFontX`) gegen die Breite aus dem Skript – **nicht** mit `zCView::FontSize`, das über die aktuelle
  Bildpunkt-Breite des Eintrags rechnet und beim Öffnen noch falsch ist. Ein Plugin skaliert die Schriften (Faktor bei
  0x7079C0F4).
- Menü-Skript vor dem Einspielen prüfen: `tools\menucheck.py` (Bezeichner gegen `MENU.DAT`) und
  `tools\menudiff.py <vorher> <neu>` (nichts versehentlich verloren).
- **Eigene Bilder (Texturen, Gegenstands-Aussehen):** Ablauf, Werkzeuge und Gothics Formate in `gt-fix\BILDER-EINBINDEN.md`
  (Bytes in der DLL, GT schreibt die Dateien selbst in den Spielordner – nichts zusätzlich zu verteilen).

## 7. Spielstände

- Jeder gemeinsame Stand hat eine **Kennung `sid`** (Zeit_Rolle) in `COOPSYNC.TXT`, dazu die **PC-Kennung** des Besitzers,
  ein Manifest mit CRC32 je Datei und seit v93 einen **Durchlauf** `JJJJ-MM-TT_hh-mm-ss_Host_Mitspieler`.
- Eigenes Archiv `<Spielstände>\coop_archiv\<Durchlauf>\<sid>\`; Partnerkopien `…__von_<PC>`. Othellos 15 Slots dienen nur
  zum Laden: die Plätze **6–9** sind für Coop reserviert, ein Stand wird vor dem Ersetzen gesichert.
- Laden läuft immer über den Host: er wählt, beide laden denselben Stand, **jeder mit seinem eigenen Helden**. Fehlt dem
  Partner die Welt, holt er sie in Stücken (`FILE_REQ/PART/END`, gepackt, mit Prüfsummen).
- Regel: Alles, was nur zur Laufzeit gilt (Kampf-Übergabe, Treffer, laufende Zustände), beim **Ladebeginn verwerfen** –
  danach gilt der Spielstand.

## 8. Engine-Fallen (dieser Mod-Stack)

1. **Othello widmet Engine-Funktionen um**, die niemand direkt aufruft (`MakeSpellBook`, `UseStandAI`, `DeleteNpc`) – dort
   steht fremder Code. `OthelloCompat.cpp` bildet sie nach. `gtcalls.py` meldet darum dauerhaft „2 ERSETZT“.
2. **vtable-Plätze** können woanders hinzeigen als die Funktion selbst (z. B. `ResetPos` mit falscher Argumentzahl →
   Stapel kaputt). Jeden virtuellen Aufruf gegen das Speicherabbild prüfen; im Zweifel den nicht-virtuellen Weg nehmen.
3. **Felder sind verschoben** (Othello vergrößert Klassen, z. B. `oCWorld`-Objektlisten) – Offsets zur Laufzeit lesen.
4. **Menülisten ungeprüft lesen stürzt ab** (`GetActiveItem` ohne Bereichsprüfung, Othellos Steuerungsmenü hat keine Einträge).
5. **`string::Combine` nimmt für `%s` nur Union-`string`-Objekte**, und als Format nur `char*` (daher gibt `CoopTr` `char*`).
6. Vor `zCView::InsertItem(…, stayOnTop)` das echte Ende der `screen`-Liste setzen, sonst wird alles danach abgeschnitten.
7. Prüfregeln vor jeder neuen Engine-Stelle: `COOP-FIXES.md` Abschnitt 2 (Eingang, Rumpf, vtable, Aufrufer, Felder) und
   nach jedem Build `gtcalls.py` gegen die vorige Fassung vergleichen.
8. **GT sperrt eigene Animationen von Puppen** (`zCModel_StartAni`: beim Host vom Client geführte NPCs, beim Client vom Host
   geführte). Engine- oder Othello-Code, der eine Animation startet und sofort abfragt, greift dann ins Leere – so stürzte
   Othellos Seitenschritt in `zCModel::GetProgressPercent` ab (v101 A17: Ergebnis 0 statt Absturz). Neue Sperren immer darauf
   prüfen, wer nach `StartAni` die laufende Animation voraussetzt.
9. Startet GT einen Kampfzustand neu (Übergabe, Umlenkung), läuft dessen Startfunktion erneut – in `ZS_Attack` z. B.
   `B_WhirlAround` (Überraschungs-Sprung). v100 C21 macht daraus bei solchen Neustarts eine normale Drehung.

## 9. Logs und Diagnose

- `system\GothicCoopOthello.log` ist das Hauptprotokoll unter Othello, sonst (ab u1) `system\GothicCoop.log` (deutsch, mit
  Sekunde und Rolle; zweite Zeile nennt das Profil). Der Partner bekommt es gespiegelt: `…Client.log` beim Host, `…Host.log`
  beim Client (z. B. `GothicCoopOthelloClient.log`).
- Beim Start archiviert GT die Logs der vorigen Sitzung nach `CoopLogs\<Fassung>\<Start>\`. **Logs nie löschen.**
- `GothicCoopAudit.log` ist die Selbstprüfung: alle Haken vorn? fremde Patches? Fingerabdruck der Engine-Abschnitte.
- `Shift+F11` startet einen Testlauf mit Rückmeldung des Partners je Schritt.
- **Abstürze:** Im Vollbild liegen die Fehlerfenster unsichtbar hinter dem Spiel – ein Absturz sieht dann aus wie ein Hänger.
  Seit v101 schreibt GT zuerst einen Block „ABSTURZ: …“ ins Hauptprotokoll (Stelle als Modul+Versatz, Register,
  Rücksprünge vom Stapel) und minimiert das Spielfenster. **Othello hat einen eigenen Absturz-Filter** (schreibt
  `system\Othello_Crash_Reports.txt`, Fenster „== application crashed“) und ruft Gothics Filter nicht auf – Haken auf Gothics
  `zCExceptionHandler` laufen unter Othello nie. GT legt sich darum jedes Bild per `SetUnhandledExceptionFilter` davor und
  reicht weiter. Ohne Log-Eintrag hilft das Windows-Ereignisprotokoll
  (Anwendung, ID 1000 „GothicMod.exe“). Ein `c0000409` in `KERNELBASE.dll+0x16bae4` etwa 10 s nach dem Start steht seit
  22.10. bei jedem Start dort; das Spiel läuft danach weiter (gleiche Prozessnummer) – Ursache ungeklärt, bisher ohne Folgen
  (Code 21 = „ungültige Ausnahme-Kette“). Der Absturz vom 22.10. 22:11 (v105 A19) lief an allen Filtern vorbei; seit v105 schreibt
  darum ein Vectored Exception Handler die ersten drei Zugriffsfehler je Sitzung schon beim Auftreten ins Log („Ausnahme (erste
  Stufe …)“, A20).

## 10. Bauen und prüfen

```
cd "C:\Gothic Othello Kopie\_othello_backup"
python tools\hooktable.py gt-fix\gothiccoop-src gt-fix\gothiccoop-src\ZenGin\Gothic_I_Classic\API builds\<letzte>.dll gt-fix\gothiccoop-src\CoopHookTable.h
cd gt-fix\gothiccoop-src
msbuild GothicCoop.vcxproj /p:Configuration="G1 MD Release" /p:Platform=Win32 /p:PreBuildEventUseInBuild=False /p:PostBuildEventUseInBuild=False
```

`CoopHookTable.h` **vor** dem Bauen erzeugen (sonst fehlen neue Haken in der Selbstprüfung); ändern sich die Aufrufziele,
danach noch einmal erzeugen und neu bauen. Erwartet (Stand v105): 46 Haken, 317 Aufrufziele, „2 ERSETZT“, Haken-Tabelle gleich.
ZIP je Fassung: `python tools\makezip.py vNN --dll … --menu …`.

## 11. Wo was steht

| Datei | Inhalt |
|---|---|
| `..\NEUES-FENSTER.md` | Arbeitsregeln, Ordner, Fassungschronik, offene Punkte |
| `COOP-FIXES.md` | Fix-Katalog A–L mit Engine-Adressen, Pakettabelle, Konfiguration, G1→G2-Tabelle, Grenzen |
| `README.md` | je Fassung: Fehlerbild, Ursache, Lösung, Testplan |
| `..\WEITERMACHEN.md` | Chronik, neueste oben |
| `PLAN-*.md` | Entwürfe größerer Umbauten (Steam, Spielstände, Verbindungsmenü, COOP-Menü, allgemeines Coop) |
| `UPSTREAM-ABGLEICH.md` | Abgleich mit dem neueren GothicCoop |
| `SPIELSTAND-BERICHT.md`, `VORSCHAU-3D.md`, `GD3D11-UMBAU.md` | Spielstand-Sicherheit, 3D-Vorschau, Renderer-Bau |
| `..\speicherabbild\` | Abbild des laufenden Spiels (`patchmap.tsv`, `.text.mem`/`.text.file`, …) – Grundlage jeder Adressprüfung |
