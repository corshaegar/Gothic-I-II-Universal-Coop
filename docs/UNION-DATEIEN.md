# Union-Dateien (für Pakete ohne Union-Installer)

Nutzer (26.10.2026): „ohne Installer sondern die Dateien von Union werden direkt reingepackt. Das macht jede Mod so Othello zb auch.“
Ermittelt an `Union Vorlage\Gothic II` (Gothic II Gold mit Union 1.0m, vom Nutzer frisch installiert; nicht im Repo): alles, was
der Union-Installer am 26.10.2026 16:42 geschrieben bzw. nach `.backup_union\` verschoben hat.

## Gothic II (Union 1.0m)

| Datei | Größe | SHA-256 (16) | Art |
|---|---|---|---|
| `Data\Union.vdf` | 2.037.609 | `B5E29DC8B7B7…` | neu |
| `system\Union.patch` | 251.001 | `77BAAABC57FC…` | neu |
| `system\Shw32.dll` | 4.396.032 | `78D9312F07EB…` | ersetzt (Original 50.176 B) |
| `system\Vdfs32g.dll` | 349.696 | `6E5D544BA513…` | ersetzt (Original 2.549.760 B) |
| `system\stdhost.exe` | 72.704 | `C5632644B2D3…` | neu |
| `system\msvcp100.dll` | 421.200 | `A3BA64219912…` | neu (VC++ 2010) |
| `system\msvcr100.dll` | 770.384 | `1221A0948496…` | neu (VC++ 2010) |
| `system\dinput.dll` | – | – | **entfernt** (SystemPack-Lader; Union lädt SystemPack selbst) |
| `Union - Uninstall.exe` | 13.678.862 | – | nur Installer, nicht ins Paket |

Unverändert bleiben `system\SystemPack.ini`, `Data\SystemPack.vdf`, `system\Gothic.ini`. Gesichert legt der Installer die
ersetzten bzw. entfernten Dateien in `.backup_union\` ab (dinput.dll, Gothic.ini, Shw32.dll, SystemPack.ini, SystemPack.vdf,
Vdfs32g.dll). Der Coop-Spielordner `Gothic II` hat dieselben Union-Dateien (Prüfsummen gleich).

Für ein Paket heißt das: die sieben Dateien mitliefern und `system\dinput.dll` entfernen (Installer) bzw. in der Anleitung nennen
(ZIP); vorher sichern wie `.backup_union`, damit Rückgängig geht. Othello bringt Union selbst mit. Gothic I: noch nicht ermittelt
(eigene Vorlage nötig).

## Hinweis Prüfsummen

Kurzform wie in `builds\INHALT.md` (erste Zeichen von SHA-256). Volle Werte bei Bedarf mit `Get-FileHash` neu ziehen.
