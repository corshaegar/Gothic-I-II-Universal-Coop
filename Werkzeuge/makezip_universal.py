"""makezip_universal.py uNN [--projekt 0.6.0] [--force] - eine ZIP fuer alle Spiele (u197).

Nutzer (05.11.2026): "Da es jetzt anders laeuft braucht man auch nur noch eine Universal ZIP fuer die Mod". Seit u195 gibt es je
Engine ein Paket fuer alle Varianten (Skript-Weiche P204: Gothic 1 und Othello, Gothic 2 und jede Mod darin). Diese ZIP fasst beide
Engine-Pakete derselben Fassung zusammen (gebaut von makezip_g1.py und makezip_g2.py):
  LIESMICH.txt, README.txt                 was wohin (deutsch / englisch)
  Gothic 1 + Othello/                      Inhalt in den Ordner von Gothic 1 kopieren (auch mit Othello)
  Gothic 2 + Mods/                         Inhalt in den Ordner von Gothic 2 Die Nacht des Raben (auch Legend of Ahssun ...)
  Union 1.0m (nur wenn Union fehlt)/       die sieben Dateien von Union 1.0m (installer_bau.UNION_FILES) - nicht bei Othello
  Steam fuer Gothic 1|2 (...)/             u252: system\\steam_api.dll und steam_appid.txt der Engine - nur fuer "Coop ueber Steam" und
                                           nur, wenn das Spiel keine eigene steam_api.dll hat. Nutzer 11.11.2026: Absturz nach der
                                           ZIP (Archolos) - Archolos' KmSteam.dll braucht SteamAPI_Init aus seiner eigenen
                                           steam_api.dll, in der des Coops (neueres SDK) fehlt es; bis u251 lagen beide Dateien im
                                           Engine-Ordner und wurden beim Kopieren ersetzt
Getrennt bleiben die Engines: die DLL einer Engine gehoert nur in ihr Spiel (ihre festen Haken zeigen in der anderen Exe ins Leere).
u252: Union fehlt, wenn system\\Union.patch fehlt - system\\Shw32.dll hat jedes Gothic (Speicherverwaltung, ~50 KB), bis u251 stand
hier diese Datei als Merkmal (dann haette niemand Union kopiert).
Am einfachsten bleibt der Installer (erkennt das Spiel selbst). Ausgabe: <BAU>\\zip\\Gothic-I-II-Universal-Coop <projekt>.zip

Aufruf (aus dem Repo):  python tools\\makezip_universal.py u197 [--projekt 0.6.0] [--force]
"""
import argparse, hashlib, json, os, sys, time, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pfade

G1, G2, UNION = 'Gothic 1 + Othello', 'Gothic 2 + Mods', 'Union 1.0m (nur wenn Union fehlt)'
STEAM1, STEAM2 = 'Steam fuer Gothic 1 (nur Coop ueber Steam)', 'Steam fuer Gothic 2 (nur Coop ueber Steam, nicht bei Archolos)'
STEAMDATEIEN = ('system/steam_api.dll', 'system/steam_appid.txt')

LIESMICH = """Gothic-I-II-Universal-Coop {p} (Coop {f}) - ein Paket fuer alle Spiele

Am einfachsten: der Installer (Gothic-I-II-Universal-Coop-{p}-Setup.exe). Er erkennt das Spiel im Ordner selbst, zeigt die
unterstuetzten Mods, bringt GD3D11 und Union mit, wenn sie fehlen, und kann alles rueckgaengig machen.

Von Hand: den Inhalt EINES Ordners dieser ZIP in den Spielordner kopieren (den Ordner, in dem "system" liegt) und vorhandene
Dateien ersetzen:
  "{g1}"   Gothic 1 (Fassung 1.08k) - auch mit Othello 4.x: der Coop legt beim Start selbst die
                         Othello-Skripte an (system\\GothicCoop\\Skripte, Log "Skript-Weiche")
  "{g2}"      Gothic 2 Die Nacht des Raben (2.6) - auch mit Legend of Ahssun, Archolos und anderen
                         Mods; wer im GothicStarter die Mod wechselt, muss nichts neu installieren
  "{u}"   nur, wenn im Spielordner noch kein Union ist (system\\Union.patch fehlt) - nie bei
                         Othello (bringt Union mit). Danach system\\dinput.dll entfernen (Union laedt SystemPack
                         selbst).
  "{s1}" bzw.
  "{s2}"
                         nur fuer "Coop ueber Steam" und nur, wenn im Spielordner noch keine system\\steam_api.dll
                         liegt. Bringt das Spiel eine eigene mit (Archolos), diese NICHT ersetzen - sonst startet es
                         nicht mehr; der Coop laeuft dann ueber IP.
Der Coop-Renderer (system\\GD3D11\\Bin) wirkt nur mit installiertem GD3D11 v17.9.7 (der Installer bringt es mit).

Beide PCs brauchen dieselbe Coop-Fassung ({f}). Mehr in GothicCoop-LIESMICH.txt im jeweiligen Ordner, zu Othello und Legend
of Ahssun in GothicCoop-LIESMICH-Othello.txt bzw. GothicCoop-LIESMICH-LoA.txt.
Quelltext: https://github.com/corshaegar/Gothic-I-II-Universal-Coop
"""

README = """Gothic-I-II-Universal-Coop {p} (coop {f}) - one package for all games

Easiest: the installer (Gothic-I-II-Universal-Coop-{p}-Setup.exe). It detects the game in the folder, shows the supported
mods, brings GD3D11 and Union if they are missing and can undo everything.

By hand: copy the contents of ONE folder of this ZIP into the game folder (the folder containing "system") and replace
existing files:
  "{g1}"   Gothic 1 (version 1.08k) - also with Othello 4.x: the coop sets up the Othello scripts
                         itself at start (system\\GothicCoop\\Skripte, log "Skript-Weiche")
  "{g2}"      Gothic 2 Night of the Raven (2.6) - also with Legend of Ahssun, Archolos and other mods;
                         switching the mod in the GothicStarter needs no new installation
  "{u}"   only if the game folder has no Union yet (system\\Union.patch missing) - never for
                         Othello (it brings Union). Then remove system\\dinput.dll (Union loads SystemPack itself).
  "{s1}" or
  "{s2}"
                         only for "coop over Steam", and only if the game folder has no system\\steam_api.dll yet.
                         If the game brings its own (Archolos), do NOT replace it - the game would no longer start;
                         the coop then runs over IP.
The coop renderer (system\\GD3D11\\Bin) only works with GD3D11 v17.9.7 installed (the installer brings it).

Both PCs need the same coop version ({f}). More in GothicCoop-LIESMICH.txt in each folder (German).
Source: https://github.com/corshaegar/Gothic-I-II-Universal-Coop
"""


def sha16(b):
    return hashlib.sha256(b).hexdigest()[:16].upper()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('fassung')
    ap.add_argument('--projekt', help='Projektversion (Vorgabe aus versionen.json)')
    ap.add_argument('--force', action='store_true')
    a = ap.parse_args()
    projekt = a.projekt or json.load(open(os.path.join(ROOT, 'versionen.json'), encoding='utf-8'))['projekt']
    src1 = os.path.join(ROOT, pfade.zip_ziel('Gothic-I-II-Universal-Coop Gothic I %s.zip' % a.fassung))
    src2 = os.path.join(ROOT, pfade.zip_ziel('Gothic-I-II-Universal-Coop Gothic II %s.zip' % a.fassung))
    for s in (src1, src2):
        if not os.path.isfile(s):
            sys.exit('Paket der Engine fehlt: %s (erst makezip_g1.py bzw. makezip_g2.py %s)' % (s, a.fassung))
    out = os.path.join(ROOT, pfade.zip_ziel('Gothic-I-II-Universal-Coop %s.zip' % projekt))
    if os.path.exists(out) and not a.force:
        sys.exit('gibt es schon: %s (--force zum Ueberschreiben - nie bei einer verteilten Fassung)' % out)
    import installer_bau
    union = set(rel.lower() for rel, _, _ in installer_bau.UNION_FILES)
    entries, union_seen = [], {}
    steam_seen = 0
    for src, folder, steam in ((src1, G1, STEAM1), (src2, G2, STEAM2)):
        with zipfile.ZipFile(src) as z:
            for info in z.infolist():
                if info.is_dir():
                    continue
                data = z.read(info.filename)
                if info.filename.lower() in STEAMDATEIEN:   # u252: eigener Ordner - nie ueber eine steam_api.dll des Spiels
                    entries.append((steam + '/' + info.filename, data, info.date_time))
                    steam_seen += 1
                    continue
                if info.filename.lower() in union:
                    if info.filename.lower() in union_seen:
                        if union_seen[info.filename.lower()] != data:
                            sys.exit('Union-Datei %s in beiden Paketen verschieden' % info.filename)
                        continue
                    union_seen[info.filename.lower()] = data
                    entries.append((UNION + '/' + info.filename, data, info.date_time))
                else:
                    entries.append((folder + '/' + info.filename, data, info.date_time))
    if len(union_seen) != len(union):
        sys.exit('Union-Dateien unvollstaendig: %d von %d' % (len(union_seen), len(union)))
    if steam_seen != 2 * len(STEAMDATEIEN):
        sys.exit('Steam-Dateien unvollstaendig: %d von %d' % (steam_seen, 2 * len(STEAMDATEIEN)))
    fmt = dict(p=projekt, f=a.fassung, g1=G1, g2=G2, u=UNION, s1=STEAM1, s2=STEAM2)
    texts = [('LIESMICH.txt', LIESMICH.format(**fmt).replace('\n', '\r\n').encode('cp1252')),
             ('README.txt', README.format(**fmt).replace('\n', '\r\n').encode('cp1252'))]
    tmp = out + '.part'
    with zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for name, data in texts:
            info = zipfile.ZipInfo(name, time.localtime()[:6])
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, data)
        for name, data, dt in entries:
            info = zipfile.ZipInfo(name, dt)
            info.compress_type = zipfile.ZIP_DEFLATED
            z.writestr(info, data)
    with zipfile.ZipFile(tmp) as z:   # zuruecklesen und vergleichen
        bad = z.testzip()
        if bad:
            sys.exit('ZIP fehlerhaft bei %s' % bad)
        want = dict((n, d) for n, d in texts)
        want.update((n, d) for n, d, _ in entries)
        for n, d in want.items():
            if z.read(n) != d:
                sys.exit('Inhalt weicht ab: %s' % n)
        names = z.namelist()
    os.replace(tmp, out)
    per = {}
    for n in names:
        per[n.split('/')[0] if '/' in n else '(oben)'] = per.get(n.split('/')[0] if '/' in n else '(oben)', 0) + 1
    print('; '.join('%s: %d Dateien' % kv for kv in sorted(per.items())))
    print('%s  %d Bytes  SHA-256 %s' % (out, os.path.getsize(out), sha16(open(out, 'rb').read())))


if __name__ == '__main__':
    main()
