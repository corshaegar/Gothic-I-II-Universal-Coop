"""ZIP einer Fassung fuer Gothic 1 ohne Mod bauen: builds\\zip\\Gothic-I-II-Universal-Coop Gothic I uNN.zip, mit --othello ... Othello uNN.zip (bis u61 "Gothic Union Coop G1 uNN.zip").

Inhalt (Ordner wie im Spielordner, zum Entpacken in den Gothic-Ordner):
  system/Autorun/Gothic1Coop.dll        GT-DLL der Fassung (builds\\uNN\\Gothic1Coop-uNN.dll)
  system/Autorun/COOP_MENU.D            Menue-Skript      (scripts\\g1, tools\\skripte_g1.py)
  system/Autorun/COOP_ITEMS.D           Coop-Gegenstaende (scripts\\g1)
  system/Autorun/PC_MULTIPLAYER_HERO.D  Coop-Figur        (scripts\\g1)
  system/GothicCoopConfig.json          Vorgaben - ohne sie beendet GT das Spiel beim Start (Plugin.cpp)
  system/steam_api.dll, steam_appid.txt Steam-Verbindung (assets\\steam: Steam API mit SteamAPI_InitFlat, App 65540)
  system/GD3D11/Bin/g1_avx2.dll         Coop-Renderer (ab u42 coop8, wirkt nur mit installiertem GD3D11)
  GothicCoop-LIESMICH.txt               kurze Anleitung
  Data/Union.vdf, system/Union.patch ... die sieben Dateien von Union 1.0m (ab u62, wie makezip_g2.union_entries; fuer G1
                                        und G2 dieselben - binkw32.dll/Mss32.dll, die Unions Setup bei G1 auch schreibt,
                                        noch nicht: eine G1-Vorlage fehlt)
Bis u61 installierte der Nutzer Union (mit zParserExtender) selbst.

Aufruf (aus dem Repo):  python tools\\makezip_g1.py u5 [--force]
Vorbild: tools\\makezip.py (Othello-ZIPs, Pfade des Othello-Repos).
"""
import argparse, hashlib, os, sys, time, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pfade   # u62: Pfade im Repo (oeffentlicher Export: eigener Aufbau)

LIESMICH = """GothicCoop {f} - Gothic 1 ohne Mod (Union-Coop)

Voraussetzung: Gothic 1 (Steam/GOG, Fassung 1.08k). Union 1.0m (mit zParserExtender) liegt bei.
Beide PCs brauchen dieselbe GothicCoop-Fassung ({f}).

Installation: den Inhalt dieser ZIP in den Gothic-Ordner entpacken (den Ordner, in dem "system" liegt) und
vorhandene Dateien ersetzen. War Union noch nicht installiert: danach system\\dinput.dll entfernen (der Lader von
SystemPack - Union laedt SystemPack selbst). Bequemer und rueckgaengig machbar: der Installer.
  Data\\Union.vdf, system\\Union.patch, Shw32.dll, Vdfs32g.dll, stdhost.exe, msvcp100.dll, msvcr100.dll   Union 1.0m
  system\\Autorun\\              GothicCoop-DLL und drei Skripte
  system\\GothicCoopConfig.json  Vorgaben (Name, Adresse, Aussehen - im Spiel unter COOP aenderbar)
  system\\steam_api.dll und steam_appid.txt   fuer "Coop ueber Steam" (Steam muss laufen)
  system\\GD3D11\\Bin\\g1_avx2.dll  Coop-Renderer - nur wirksam, wenn GD3D11 installiert ist (3D-Vorschau der
                                 Figur, Zuschau-Fenster beim Mitspieler, Bild des Weltprozesses)

Spielen: Hauptmenue > COOP > Figur waehlen > "Weiter: Verbinden" > "Spiel hosten" bzw. "Ueber IP beitreten"
(Port 9955). Protokoll: system\\GothicCoop.log.

Ab u42 (Versuch): Weltprozess auch in Gothic 1 - ist ein Mitspieler in einer anderen Welt (z. B. in der Alten Mine),
startet ein zweites, unsichtbares Gothic auf dem eigenen PC, laedt den neuesten Spielstand und jene Welt; das
Zuschau-Fenster (Shift+F5, blauer Rahmen) zeigt den Mitspieler dort. Sein Ordner liegt im Gothic-Ordner
("Gothic\\Gothic_CoopWelt", legt GT selbst an; braucht ein NTFS-Laufwerk). Shift+F12 beendet ihn bzw. startet ihn wieder.
Beide PCs brauchen den Renderer coop8 aus dieser ZIP.
Ab u62 (Projekt Gothic-I-II-Universal-Coop 0.2.0, Quelltext: https://github.com/corshaegar/Gothic-I-II-Universal-Coop):
Union 1.0m liegt als Dateien bei.
"""


def sha16(data):
    return hashlib.sha256(data).hexdigest()[:16].upper()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('fassung')
    ap.add_argument('--dll', help='Vorgabe builds\\<fassung>\\Gothic1Coop-<fassung>.dll')
    ap.add_argument('--othello', action='store_true', help='u62: Paket fuer Gothic 1 mit Othello (Othello-Skripte und -LIESMICH, ohne Union)')
    ap.add_argument('--scripts', help='Vorgabe pfade.SKRIPTE g1 bzw. othello')
    ap.add_argument('--renderer', default=pfade.RENDERER['g1'], help='"-" = weglassen')   # u42: coop8 (Weltprozess)
    ap.add_argument('--steam', default='', help='"-" = weglassen')
    ap.add_argument('--union-dir', help='Spielordner mit Union 1.0m (Vorgabe wie installer_bau.py), "-" = weglassen')
    ap.add_argument('--name', help='Dateiname der ZIP (Vorgabe "Gothic-I-II-Universal-Coop Gothic I <fassung>.zip" bzw. "... Othello ...")')
    ap.add_argument('--force', action='store_true', help='vorhandene ZIP ersetzen')
    a = ap.parse_args()
    dll = a.dll or pfade.dll(a.fassung, 'g1')
    scripts = a.scripts or pfade.SKRIPTE['othello' if a.othello else 'g1']

    out = os.path.join(ROOT, pfade.zip_ziel(a.name or 'Gothic-I-II-Universal-Coop %s %s.zip' % ('Othello' if a.othello else 'Gothic I', a.fassung)))
    if os.path.exists(out) and not a.force:
        sys.exit('%s gibt es schon - mit --force ersetzen' % out)

    entries = [
        ('system/Autorun/Gothic1Coop.dll', dll),
        ('system/Autorun/COOP_MENU.D', os.path.join(scripts, 'COOP_MENU.D')),
        ('system/Autorun/COOP_ITEMS.D', os.path.join(scripts, 'COOP_ITEMS.D')),
        ('system/Autorun/PC_MULTIPLAYER_HERO.D', os.path.join(scripts, 'PC_MULTIPLAYER_HERO.D')),
        # Othello hat keine eigene Vorgabe - die von Gothic 1 (wie im Installer)
        ('system/GothicCoopConfig.json', os.path.join(pfade.SKRIPTE['g1'], 'GothicCoopConfig.json')),
    ]
    if a.steam != '-':
        entries += [('system/steam_api.dll', pfade.STEAM_API), ('system/steam_appid.txt', pfade.STEAM_APPID['g1'])]
    if a.renderer != '-':
        entries.append(('system/GD3D11/Bin/g1_avx2.dll', a.renderer))
    if a.union_dir != '-' and not a.othello:   # u62: Union 1.0m dazu (geprueft); Othello bringt Union selbst mit
        import makezip_g2
        entries += makezip_g2.union_entries(a.union_dir)
    text = LIESMICH
    if a.othello:
        text = open(os.path.join(ROOT, pfade.INSTALLER, 'texte', 'LIESMICH-Othello.txt'), encoding='utf-8').read()
    readme = text.format(f=a.fassung).replace('\r\n', '\n').replace('\n', '\r\n').encode('cp1252')

    os.makedirs(os.path.dirname(out), exist_ok=True)
    tmp = out + '.part'
    with zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for arc, src in entries:
            path = os.path.join(ROOT, src)
            info = zipfile.ZipInfo(arc, time.localtime(os.path.getmtime(path))[:6])
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, open(path, 'rb').read())
        info = zipfile.ZipInfo('GothicCoop-LIESMICH.txt', time.localtime()[:6])
        info.compress_type = zipfile.ZIP_DEFLATED
        z.writestr(info, readme)

    # zuruecklesen und mit den Quellen vergleichen
    with zipfile.ZipFile(tmp) as z:
        bad = z.testzip()
        if bad:
            sys.exit('ZIP fehlerhaft bei %s' % bad)
        for arc, src in entries + [('GothicCoop-LIESMICH.txt', None)]:
            got = z.read(arc)
            want = readme if src is None else open(os.path.join(ROOT, src), 'rb').read()
            print('%-42s %9d  %s  %s' % (arc, len(got), sha16(got), 'gleich' if got == want else 'ABWEICHUNG'))
            if got != want:
                sys.exit('Inhalt weicht ab: %s' % arc)
    os.replace(tmp, out)
    print('%s  %d Bytes  SHA-256 %s' % (out, os.path.getsize(out), sha16(open(out, 'rb').read())))


if __name__ == '__main__':
    main()
