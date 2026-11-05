"""u187: ZIP einer Mod-Variante bauen (Fundament + Mod-Profil, docs\\MODULE.md).

Eine Variante ist das Paket ihrer Engine (gleiche DLL, gleiche Skripte) mit eigener Anleitung. Die DLL erkennt die Mod beim Start
selbst (CoopProfile, Mods\\CoopModList.h) und schaltet ihr Profil ein.

  builds\\zip\\Gothic-I-II-Universal-Coop <Variante> uNN.zip
    = builds\\zip\\Gothic-I-II-Universal-Coop <Engine-Paket> uNN.zip ohne dessen GothicCoop-LIESMICH.txt
    + GothicCoop-LIESMICH.txt der Mod

Aufruf (aus dem Repo):  python tools\\makezip_mod.py loa u187 [--force]
"""
import argparse, hashlib, os, sys, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

MODS = {
    'loa': {
        'name': 'Legend of Ahssun',
        'kurz': 'LoA',          # u195: Ini-Name wie in GothicCoop\Mods\CoopModList.h, Anleitung GothicCoop-LIESMICH-LoA.txt im Engine-Paket
        'geprueft': '1.2.0',    # u195: im Installer unter "Unterstuetzt" angezeigt
        'basis': 'Gothic II',
        'liesmich': """GothicCoop {f} - Legend of Ahssun (Mod fuer Gothic 2 Die Nacht des Raben), Union-Coop

Voraussetzung: Legend of Ahssun (geprueft mit 1.2.0) auf Gothic 2 Die Nacht des Raben 2.6. Union 1.0m liegt bei.
Beide PCs brauchen dieselbe GothicCoop-Fassung ({f}) und dieselbe LoA-Fassung.

Installation: den Inhalt dieser ZIP in den Ordner von Legend of Ahssun entpacken (der Ordner mit "system" und
"Data\\ModVDF\\LoA.mod") und vorhandene Dateien ersetzen. Gestartet wird wie gewohnt: im GothicStarter "Legend of Ahssun"
waehlen oder system\\Gothic2.exe -game:LoA.ini.

Was der Coop fuer LoA tut (im Log system\\GothicCoop.log steht "Profil: Legend of Ahssun"):
- LeGo-Haken vor GT: GT versetzt den kopierten Sprung selbst (P177). An der Reihenfolge der Plugins ist nichts zu tun.
- Hauptmenue mit den LoA-Eintraegen: COOP wird ins Raster eingehaengt (P178).
- Fernkampf ohne Munition und Fehler mitten im Laden sind abgefangen (P180, P181).
- Hinweis: Im Fenstermodus koennen Videos mit dem Renderer GD3D11 haengen. Randloses Vollbild ist empfohlen.
- Eigene Heldenwerte von LoA (z. B. AMBVAR_HERO_TALKING) bleiben je PC - das LoA-Profil ist eingebaut (P190, ab u194 im
  Format der Profil-Datei). Ergaenzungen ohne neue DLL: system\\GothicCoopMod-LoA.json (Vorlage mit Erklaerung:
  system\\GothicCoopMod.vorlage.json, ab u194). Beide PCs brauchen dieselbe Datei.
- Ab u196: kein Absturz mehr beim gemeinsamen Laden (LeGos Objektzaehler, P206; beschaedigte Spielstaende repariert der Coop
  beim Laden). Ab u197: Uebung, Talente, Rezepte und Erfolge bleiben auch beim Verbinden die eigenen (P207).
- Ab u195: dieselben Dateien wie im Paket "Gothic II" - ein Coop fuer Gothic 2 und jede Mod darin. Wer im GothicStarter
  zwischen Gothic 2 und Legend of Ahssun wechselt, muss nichts neu installieren; der Coop erkennt beim Start, was laeuft.

Alles andere ist das Fundament des Coops wie im Paket "Gothic II" derselben Fassung ({f}) - dessen Anleitung liegt als
GothicCoop-LIESMICH-Gothic2.txt bei.
""",
    },
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('mod', choices=sorted(MODS))
    ap.add_argument('fassung')
    ap.add_argument('--force', action='store_true')
    a = ap.parse_args()
    m = MODS[a.mod]
    zdir = os.path.join(ROOT, 'builds', 'zip')
    basis = os.path.join(zdir, 'Gothic-I-II-Universal-Coop %s %s.zip' % (m['basis'], a.fassung))
    ziel = os.path.join(zdir, 'Gothic-I-II-Universal-Coop %s %s.zip' % (m['name'], a.fassung))
    if not os.path.isfile(basis):
        sys.exit('Paket der Engine fehlt: %s (erst makezip_g2.py bzw. makezip_g1.py)' % basis)
    if os.path.exists(ziel) and not a.force:
        sys.exit('gibt es schon: %s (--force zum Ueberschreiben - nie bei einer getaggten oder verteilten Fassung)' % ziel)
    n = 0
    with zipfile.ZipFile(basis) as zin, zipfile.ZipFile(ziel, 'w', zipfile.ZIP_DEFLATED) as zout:
        for info in zin.infolist():
            name = info.filename
            if name == 'GothicCoop-LIESMICH-%s.txt' % m['kurz']:
                continue   # u195: kommt hier als GothicCoop-LIESMICH.txt
            data = zin.read(name)
            if name == 'GothicCoop-LIESMICH.txt':
                info.filename = 'GothicCoop-LIESMICH-Gothic2.txt'   # u195: Anleitung des Fundaments bleibt dabei
            zout.writestr(info, data)
            n += 1
        text = m['liesmich'].format(f=a.fassung).replace('\n', '\r\n')
        zout.writestr('GothicCoop-LIESMICH.txt', text.encode('cp1252', errors='replace'))
    h = hashlib.sha256(open(ziel, 'rb').read()).hexdigest().upper()
    print('%s  %d Bytes  SHA-256 %s  (%d Dateien aus %s + Anleitung)' % (ziel, os.path.getsize(ziel), h[:16], n,
                                                                        os.path.basename(basis)))


if __name__ == '__main__':
    main()
