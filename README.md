# Gothic-I-II-Universal-Coop

Koop-Mehrspieler für **Gothic 1**, **Gothic 2 Die Nacht des Raben** und **Gothic 1 mit Othello** – zwei (oder mehr) Spieler
spielen dieselbe Geschichte gemeinsam: Welt, NPCs, Kämpfe, Dialoge, Tagebuch und Spielstände werden abgeglichen.
Ein Union-Plugin auf Grundlage von [GothicCoop (Gothic Together)](https://gitlab.com/gothiccoop/gothiccoop), AGPL-3.0.

*English: co-op multiplayer for Gothic 1, Gothic 2 Night of the Raven and Gothic 1 with Othello (Union plugin, AGPL-3.0).
Download the installer or the ZIP for your game from the releases; documentation is in German.*

## Versionen

Projektversion **0.2.0** – je Spiel/Mod die Coop-Fassung (zählt nur hoch, wenn sich daran etwas ändert):

| Spiel / Mod | Coop-Fassung |
|---|---|
| Gothic I | u62 |
| Gothic II | u62 |
| Othello | u62 |

Beide PCs brauchen dieselbe Coop-Fassung. Die Fassung steht im Hauptmenü („GothicCoop uNN“).

## Herunterladen und installieren

Unter **Releases** liegen:

- **Installer** (`Gothic-I-II-Universal-Coop-0.2.0-Setup.exe`): Spiel wählen (Gothic 1, Gothic 2, Othello), Spielordner
  wählen, Installieren. Er prüft das Spiel am Inhalt, bringt GD3D11 v17.9.7 mit (falls es fehlt) und legt den Coop-Renderer
  darüber; fehlt Union bei Gothic 1/2, installiert er Union 1.0m mit. Alles Ersetzte kommt nach `CoopBackup`,
  „Letzte Installation rückgängig“ stellt es wieder her.
- **ZIP je Spiel** (`Gothic-I-II-Universal-Coop-0.2.0-Gothic-I-uNN.zip`, `…-Gothic-II-uNN.zip`, `…-Othello-uNN.zip`): in den
  Spielordner entpacken (den Ordner, in dem `system` liegt). Die ZIPs für Gothic 1/2 enthalten Union 1.0m – war Union vorher
  nicht installiert, danach `system\dinput.dll` entfernen. Hinweise in `GothicCoop-LIESMICH.txt` in der ZIP.

Voraussetzungen: Gothic 1 in Fassung 1.08k (Steam/GOG), Gothic 2 Die Nacht des Raben 2.6, Othello 4.x (bringt Union mit).

Spielen: Hauptmenü › COOP › Figur wählen › „Weiter: Verbinden“ › „Spiel hosten“ bzw. „Über IP beitreten“ (Port 9955) oder
über Steam.

## Ordner

| Ordner | Inhalt |
|---|---|
| `Kern/` | gemeinsamer Quelltext aller Spiele: das Union-Plugin (`GothicCoop.vcxproj`, Unity-Build über `Sources.h`), Union-SDK in `ZenGin/` |
| `Gothic I/` | Spielskripte des Coops für Gothic 1 (`Skripte/`), Steam-Kennung, Bilder, Anleitung |
| `Gothic II/` | dasselbe für Gothic 2 Die Nacht des Raben |
| `Othello/` | Spielskripte für Othello (die Coop-DLL ist die von Gothic 1; sie erkennt Othello selbst) |
| `Installer/` | Quelltext und Texte des Installers |
| `Renderer/` | Coop-Patches für GD3D11 v17.9.7 (Zuschau-Fenster, 3D-Vorschau, Weltprozess) |
| `Werkzeuge/` | Bauen der DLL, der ZIPs und des Installers, Installer-Test |
| `Lizenzen/` | Lizenztexte und Herkunftshinweise |
| `docs/` | wie die Mod funktioniert, Fix-Katalog, Union-Dateien |

## Bauen

Visual Studio 2019 Build Tools (C++, Win32):

```
powershell -File Werkzeuge\bau.ps1 -Config "G2A MD Release" -Copy Bau\Gothic2Coop.dll
powershell -File Werkzeuge\bau.ps1 -Config "G1 MD Release"  -Copy Bau\Gothic1Coop.dll
```

Für ZIPs und Installer (`Werkzeuge\makezip_g1.py`, `makezip_g2.py`, `installer_bau.py`) liegen zusätzlich in `Bau\eingaben\`:
die Coop-Renderer (aus GD3D11 v17.9.7 mit `Renderer\coop8.patch` gebaut), `steam_api.dll` (Steamworks) und unter
`Bau\eingaben\Union\` die Dateien von Union 1.0m (`docs\UNION-DATEIEN.md`, geprüft gegen ihre Prüfsummen); für den Installer
außerdem das Release-ZIP von GD3D11 v17.9.7. Die DLLs je Fassung erwartet es unter `Bau\uNN\`.

## Lizenz und Herkunft

GNU Affero General Public License 3.0 (`LICENSE`) wie das Original GothicCoop / Gothic Together. Fremdbibliotheken: ENet
(MIT), nlohmann/json (MIT). Der Coop-Renderer ist eine geänderte Fassung von GD3D11 (GPL-3.0), siehe `Lizenzen/`.
Gothic-, Othello- und Union-Dateien gehören ihren Rechteinhabern; Union 1.0m (Gratt, WOG RU Community) liegt den Paketen
bei, wie es Mods üblicherweise tun. Es gibt keinerlei Gewährleistung.
