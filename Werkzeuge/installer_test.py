"""installer_test.py <setup.exe> <GD3D11-v17.9.7.zip> <repo> - Installer gegen nachgebaute Spielordner pruefen (ab u53).

Legt in %TEMP%\\GothicCoopInstallerTest Spielordner an (leere Exe-/Union-Dateien, ggf. GD3D11 aus der ZIP) und ruft den Installer
ohne Oberflaeche auf: Neuinstallation, zweiter Lauf ohne Aenderung, rueckgaengig, Renderer ueber v17.9.7, andere GD3D11-Fassung,
Othello, fremdes ddraw.dll, ohne Union, falsches Spiel, laufendes Gothic (PING.EXE als Gothic2.exe). Echte Spielordner fasst es nicht an.
T7 (ab u62): Union fehlt - mit /union schreibt der Installer die sieben Union-Dateien (Pruefsummen wie installer_bau.UNION_FILES),
legt system\\dinput.dll nach CoopBackup, "rueckgaengig" stellt den Ordner wieder her; ohne /union, bei Othello und mit vorhandenem
Union nichts davon (bis u61: Union-Setup mit Platzhalter tools\\installer_unionattrappe.c).
T8 (ab u54): falsches Spiel - Othello auf Gothic 1 und umgekehrt, Teile von Othello, Gothic 2 ohne Addon oder in anderer Fassung,
Gothic 1 in anderer Fassung oder ohne Welt, Gothic 2 <-> Othello; leere Dateien mit den richtigen Namen zaehlen nicht, Othello
unter anderem VDF-Namen schon (Erkennung am Katalog). T9: Gothic 1/2 mit Mods. Die Spiel-Exes sind kleine Platzhalter mit
Versionsressource (2.6.0.0, 1.0.8.11 bzw. falsche Fassungen), hier gebaut; die VDFs haben Kopf und Katalog wie Gothics.
"""
import hashlib, os, shutil, struct, subprocess, sys, time, zipfile

exe, gzip, repo = sys.argv[1:4]
base = os.path.join(os.environ.get('TEMP', '.'), 'GothicCoopInstallerTest')
fassung = os.path.basename(exe).split()[-1].replace('.exe', '')
ok_all = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from installer_bau import VCVARS, UNION_FILES
import pfade   # u62: Pfade im Repo


def rp(p):
    return os.path.join(repo, p)


def sha(p):
    return hashlib.sha256(open(p, 'rb').read()).hexdigest()


def check(name, cond, info=''):
    global ok_all
    print('  %-4s %s %s' % ('OK' if cond else 'FEHL', name, info))
    ok_all &= bool(cond)


def run(*args):
    r = subprocess.run([exe] + list(args)).returncode
    return r


def run_exe(e, *args):
    return subprocess.run([e] + list(args)).returncode


def last_log(d):
    ld = os.path.join(d, 'CoopLogs', 'Installer')
    return open(os.path.join(ld, sorted(os.listdir(ld))[-1]), encoding='utf-8-sig').read()


def touch(p, data=b'x'):
    os.makedirs(os.path.dirname(p), exist_ok=True)
    open(p, 'wb').write(data)


def tree(d):
    out = {}
    for root, _, files in os.walk(d):
        for f in files:
            p = os.path.join(root, f)
            rel = os.path.relpath(p, d)
            if rel.split(os.sep)[0] in ('CoopLogs', 'CoopBackup'):
                continue
            out[rel.lower()] = sha(p)
    return out


def make_vdf(p, files):
    """VDF mit Kopf und Katalog wie Gothics (Dateien leer) - der Installer liest nur den Katalog."""
    tree = {}
    for f in files:
        node = tree
        parts = f.upper().split('\\')
        for x in parts[:-1]:
            node = node.setdefault(x, {})
        node[parts[-1]] = None
    entries = []

    def level(node):
        start = len(entries)
        keys = list(node)
        entries.extend([k, 0, 0] for k in keys)
        for j, k in enumerate(keys):
            e = entries[start + j]
            if node[k] is not None:
                e[2] |= 0x80000000
                e[1] = level(node[k])
            if j == len(keys) - 1:
                e[2] |= 0x40000000
        return start

    level(tree)
    data = 296 + 80 * len(entries)
    cat = b''.join(k.encode('latin-1').ljust(64, b' ') + struct.pack('<4I', o if t & 0x80000000 else data, 0, t, 0x20) for k, o, t in entries)
    touch(p, b'Testdaten'.ljust(256, b'\x1a') + b'PSVDSC_V2.00\n\r\n\r' + struct.pack('<6I', len(entries), len(files), 0, 0, 296, 80) + cat)


OTH_SCRIPTS = ['SYSTEM\\AUTORUN\\OTHELLO_ABI.DLL', '_WORK\\DATA\\SCRIPTS\\_COMPILED\\GOTHIC.DAT', '_WORK\\DATA\\WORLDS\\GOBLINCAVE.ZEN']
OTH_WORLDS = ['_WORK\\DATA\\WORLDS\\WORLD.ZEN', '_WORK\\DATA\\WORLDS\\GOBLINCAVE.ZEN', '_WORK\\DATA\\WORLDS\\OLDTEMPLE.ZEN']


def game(name, g2=True, union=True, othello=False, ver=None, addon=True, world=True, exe_name=None):
    d = os.path.join(base, name)
    os.makedirs(os.path.join(d, 'system'))
    shutil.copy(dummy[ver or ('g2_26' if g2 else 'g1_108k')], os.path.join(d, 'system', exe_name or ('Gothic2.exe' if g2 else 'GothicMod.exe')))
    if g2:
        make_vdf(os.path.join(d, 'Data', 'Worlds.vdf'), ['_WORK\\DATA\\WORLDS\\NEWWORLD\\NEWWORLD.ZEN'])
        if addon:
            make_vdf(os.path.join(d, 'Data', 'Worlds_Addon.vdf'), ['_WORK\\DATA\\WORLDS\\ADDON\\ADDONWORLD.ZEN', '_WORK\\DATA\\WORLDS\\NEWWORLD\\NEWWORLD.ZEN'])
    elif world:
        make_vdf(os.path.join(d, 'Data', 'worlds.VDF'), ['_WORK\\DATA\\WORLDS\\WORLD.ZEN', '_WORK\\DATA\\WORLDS\\OLDMINE.ZEN'])
    if union:
        touch(os.path.join(d, 'system', 'Shw32.dll'))
        touch(os.path.join(d, 'system', 'Union.patch'))
    if othello:
        make_vdf(os.path.join(d, 'Data', 'ScriptsOthello4.0_de.vdf'), OTH_SCRIPTS)
        make_vdf(os.path.join(d, 'Data', 'WorldsOthello4.0.vdf'), OTH_WORLDS)
        touch(os.path.join(d, 'Othello-4.4.2.1-uninstall.exe'))
    return d


def extract_gd3d11(d):
    z = zipfile.ZipFile(gzip)
    for i in z.infolist():
        if not i.is_dir():
            touch(os.path.join(d, 'system', *i.filename.split('/')), z.read(i))


if os.path.exists(base):
    shutil.rmtree(base)
os.makedirs(base)

# Spiel-Exes als Platzhalter (nur Versionsressource)
att = os.path.join(base, 'attrappe')
os.makedirs(att)
open(os.path.join(att, 'spiel.c'), 'w', encoding='ascii').write('int main(void) { return 0; }\n')
open(os.path.join(att, 'spiel.rc'), 'w', encoding='ascii').write(
    '#include <winver.h>\nVS_VERSION_INFO VERSIONINFO\n FILEVERSION VA,VB,VC,VD\n PRODUCTVERSION VA,VB,VC,VD\nBEGIN\n'
    ' BLOCK "VarFileInfo"\n BEGIN\n  VALUE "Translation", 0x409, 1200\n END\nEND\n')
dummy = {'g2_26': (2, 6, 0, 0), 'g2_130': (1, 30, 0, 0), 'g1_108k': (1, 0, 8, 11), 'g1_108f': (1, 0, 8, 6)}
lines = ['@echo off', 'call "%s" x86 >nul' % VCVARS, 'cd /d "%s"' % att]
for k, v in dummy.items():
    lines.append('rc /nologo /d VA=%d /d VB=%d /d VC=%d /d VD=%d /fo %s.res spiel.rc >nul || exit /b 1' % (v + (k,)))
    lines.append('cl /nologo /O1 /MT spiel.c %s.res /Fe%s.exe >nul || exit /b 1' % (k, k))
    dummy[k] = os.path.join(att, k + '.exe')
open(os.path.join(att, 'bau.bat'), 'w', encoding='ascii').write('\r\n'.join(lines) + '\r\n')
r = subprocess.run(['cmd', '/c', os.path.join(att, 'bau.bat')], capture_output=True, text=True, errors='replace')
if r.returncode or not all(os.path.exists(p) for p in dummy.values()):
    sys.exit('Platzhalter nicht gebaut: ' + (r.stdout + r.stderr)[-400:])
dll2 = rp(pfade.dll(fassung, 'g2'))
dll1 = rp(pfade.dll(fassung, 'g1'))
r2 = rp(pfade.RENDERER['g2'])
r1 = rp(pfade.RENDERER['g1'])
o2 = rp(pfade.RENDERER['g2_original'])

print('T1 Gothic 2 ohne GD3D11')
d = game('T1')
before = tree(d)
rc = run('/still', '/spiel:g2', '/ordner:' + d)
check('Rueckgabe 0', rc == 0, rc)
check('Coop-DLL', sha(os.path.join(d, 'system', 'Autorun', 'Gothic2Coop.dll')) == sha(dll2))
check('Skripte G2', sha(os.path.join(d, 'system', 'Autorun', 'COOP_MENU.D')) == sha(os.path.join(rp(pfade.SKRIPTE['g2']), 'COOP_MENU.D')))
check('GD3D11 ddraw.dll', os.path.exists(os.path.join(d, 'system', 'ddraw.dll')))
check('Coop-Renderer', sha(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g2a_avx2.dll')) == sha(r2))
check('G1-Renderer original', sha(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g1_avx2.dll')) != sha(r1))
check('Lizenzen', os.path.exists(os.path.join(d, 'GothicCoop-Lizenzen', 'GD3D11-coop9.patch')))
logs = os.listdir(os.path.join(d, 'CoopLogs', 'Installer'))
check('Protokoll', len(logs) == 1, logs)
runs = os.listdir(os.path.join(d, 'CoopBackup'))
check('Lauf in CoopBackup', len(runs) == 1 and os.path.exists(os.path.join(d, 'CoopBackup', runs[0], 'COOPINSTALL.TXT')), runs)
time.sleep(1.1)
rc = run('/still', '/spiel:g2', '/ordner:' + d)
check('zweimal: Rueckgabe 0', rc == 0, rc)
check('zweimal: kein neuer Lauf (alles gleich)', len(os.listdir(os.path.join(d, 'CoopBackup'))) == 1)
rc = run('/still', '/rueckgaengig', '/ordner:' + d)
check('rueckgaengig: Rueckgabe 0', rc == 0, rc)
check('rueckgaengig: Ordner wie vorher', tree(d) == before, sorted(set(tree(d)) ^ set(before))[:5])
check('rueckgaengig: GD3D11-Ordner weg', not os.path.exists(os.path.join(d, 'system', 'GD3D11')))
check('Protokolle bleiben', len(os.listdir(os.path.join(d, 'CoopLogs', 'Installer'))) == 3)

print('T2 Gothic 2 mit GD3D11 v17.9.7, alte Coop-Fassung, eigene Konfig')
d = game('T2')
extract_gd3d11(d)
touch(os.path.join(d, 'system', 'Autorun', 'Gothic2Coop.dll'), b'alte Fassung')
touch(os.path.join(d, 'system', 'GothicCoopConfig.json'), b'{"nickname": "Meins"}')
before = tree(d)
rc = run('/still', '/spiel:g2', '/ordner:' + d)
check('Rueckgabe 0', rc == 0, rc)
check('Coop-Renderer ueber v17.9.7', sha(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g2a_avx2.dll')) == sha(r2))
check('Konfig behalten', open(os.path.join(d, 'system', 'GothicCoopConfig.json'), 'rb').read() == b'{"nickname": "Meins"}')
run_dir = os.path.join(d, 'CoopBackup', os.listdir(os.path.join(d, 'CoopBackup'))[0])
man = open(os.path.join(run_dir, 'COOPINSTALL.TXT'), encoding='utf-8').read()
check('nur Renderer + Coop ersetzt (kein GD3D11-Grundpaket)', man.count('datei=') < 20, man.count('datei='))
check('Sicherung Renderer', open(os.path.join(run_dir, 'system', 'GD3D11', 'Bin', 'g2a_avx2.dll.sicherung'), 'rb').read() == open(o2, 'rb').read())
check('Sicherung alte Coop-DLL', open(os.path.join(run_dir, 'system', 'Autorun', 'Gothic2Coop.dll.sicherung'), 'rb').read() == b'alte Fassung')
check('keine .dll in CoopBackup', not any(f.lower().endswith('.dll') for _, _, fs in os.walk(os.path.join(d, 'CoopBackup')) for f in fs))
rc = run('/still', '/rueckgaengig', '/ordner:' + d)
check('rueckgaengig: Ordner wie vorher', rc == 0 and tree(d) == before, sorted(set(tree(d)) ^ set(before))[:5])

print('T3 Gothic 2 mit anderer GD3D11-Fassung')
d = game('T3')
extract_gd3d11(d)
touch(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g1.dll'), b'andere Fassung')
rc = run('/still', '/spiel:g2', '/ordner:' + d)
check('Rueckgabe 0', rc == 0, rc)
z = zipfile.ZipFile(gzip)
check('GD3D11 auf v17.9.7 gebracht', sha(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g1.dll')) == hashlib.sha256(z.read('GD3D11/Bin/g1.dll')).hexdigest())
check('Coop-Renderer', sha(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g2a_avx2.dll')) == sha(r2))

print('T4 Gothic 1 mit Othello')
d = game('T4', g2=False, othello=True)
rc = run('/still', '/spiel:othello', '/ordner:' + d)
check('Rueckgabe 0', rc == 0, rc)
check('Coop-DLL G1', sha(os.path.join(d, 'system', 'Autorun', 'Gothic1Coop.dll')) == sha(dll1))
check('Othello-Skripte', sha(os.path.join(d, 'system', 'Autorun', 'COOP_MENU.D')) == sha(os.path.join(rp(pfade.SKRIPTE['othello']), 'COOP_MENU.D')))
check('Coop-Renderer G1', sha(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g1_avx2.dll')) == sha(r1))
check('LIESMICH Othello', 'Othello' in open(os.path.join(d, 'GothicCoop-LIESMICH.txt'), encoding='cp1252').read())

print('T5 Gothic 1 ohne Mod, fremdes ddraw.dll, ohne Union')
d = game('T5', g2=False, union=False)
touch(os.path.join(d, 'system', 'ddraw.dll'), b'fremder Aufsatz')
rc = run('/still', '/spiel:g1', '/ordner:' + d)
check('Rueckgabe 0', rc == 0, rc)
check('fremdes ddraw.dll unangetastet', open(os.path.join(d, 'system', 'ddraw.dll'), 'rb').read() == b'fremder Aufsatz')
check('kein GD3D11', not os.path.exists(os.path.join(d, 'system', 'GD3D11')))
check('Skripte G1', sha(os.path.join(d, 'system', 'Autorun', 'COOP_MENU.D')) == sha(os.path.join(rp(pfade.SKRIPTE['g1']), 'COOP_MENU.D')))
log = open(os.path.join(d, 'CoopLogs', 'Installer', os.listdir(os.path.join(d, 'CoopLogs', 'Installer'))[0]), encoding='utf-8-sig').read()
check('Hinweis Union fehlt', 'Union fehlt' in log)
check('ohne /union kein Union mitinstalliert', 'mitinstalliert' not in log.split('HINWEIS')[0] and not os.path.exists(os.path.join(d, 'system', 'Shw32.dll')))
rc = run('/still', '/spiel:g1', '/ddraw', '/ordner:' + d)
check('mit /ddraw: GD3D11 installiert', rc == 0 and os.path.exists(os.path.join(d, 'system', 'GD3D11', 'Bin', 'g1_avx2.dll')), rc)
bak = [os.path.join(r, f) for r, _, fs in os.walk(os.path.join(d, 'CoopBackup')) for f in fs if f.lower() == 'ddraw.dll.sicherung']
check('fremdes ddraw.dll gesichert', len(bak) == 1 and open(bak[0], 'rb').read() == b'fremder Aufsatz')

print('T6 falsches Spiel / Gothic laeuft')
d = game('T6')
rc = run('/still', '/spiel:g1', '/ordner:' + d)
check('G1 auf G2-Ordner: Rueckgabe 1', rc == 1, rc)
check('nichts geschrieben', not os.path.exists(os.path.join(d, 'system', 'Autorun')))
fake = os.path.join(base, 'fake', 'Gothic2.exe')
os.makedirs(os.path.dirname(fake))
shutil.copy(os.path.join(os.environ['SystemRoot'], 'System32', 'PING.EXE'), fake)
p = subprocess.Popen([fake, '-n', '20', '127.0.0.1'], stdout=subprocess.DEVNULL)
time.sleep(1)
rc = run('/still', '/spiel:g2', '/ordner:' + d)
p.kill()
check('Gothic laeuft: Rueckgabe 1', rc == 1, rc)
check('nichts geschrieben', not os.path.exists(os.path.join(d, 'system', 'Autorun')))

print('T8 falsches Spiel: nichts installieren, Grund im Protokoll (Erkennung am Inhalt, nicht am Namen)')
MOD = ('Data/Mod_Test.vdf', ['_WORK\\DATA\\SCRIPTS\\_COMPILED\\GOTHIC.DAT', '_WORK\\DATA\\WORLDS\\MODWELT.ZEN'])


def extras(d, extra):
    for x in extra:   # 'Pfad' = leere Datei, ('Pfad', [Eintraege]) = VDF mit diesem Katalog
        if isinstance(x, tuple):
            make_vdf(os.path.join(d, *x[0].split('/')), x[1])
        else:
            touch(os.path.join(d, *x.split('/')))


for name, kw, extra, key, want in (
        ('T8a', dict(g2=False), [], 'othello', 'KEIN OTHELLO'),
        ('T8b', dict(g2=False, othello=True), [], 'g1', 'OTHELLO GEFUNDEN'),
        ('T8c', dict(g2=False), ['Data/TexturesOthello4.0.vdf'], 'g1', 'TEILE VON OTHELLO'),
        ('T8d', dict(g2=False), ['Data/TexturesOthello4.0.vdf'], 'othello', 'OTHELLO UNVOLLST'),
        ('T8e', dict(addon=False), [], 'g2', 'KEIN GOTHIC 2 DIE NACHT DES RABEN'),
        ('T8f', dict(ver='g2_130'), [], 'g2', 'KEIN GOTHIC 2 DIE NACHT DES RABEN'),
        ('T8g', dict(g2=False, ver='g1_108f'), [], 'g1', 'GOTHIC 1 IN ANDERER FASSUNG'),
        ('T8h', dict(), [], 'othello', 'FALSCHES SPIEL'),
        ('T8i', dict(g2=False, othello=True), [], 'g2', 'FALSCHES SPIEL'),
        ('T8j', dict(g2=False), [], 'g2', 'FALSCHES SPIEL'),
        # nur die Namen ohne Inhalt: kein Othello / kein Addon
        ('T8l', dict(g2=False), ['Data/ScriptsOthello4.0_de.vdf', 'Data/WorldsOthello4.0.vdf'], 'othello', 'OTHELLO UNVOLLST'),
        ('T8o', dict(addon=False), ['Data/Worlds_Addon.vdf'], 'g2', 'KEIN GOTHIC 2 DIE NACHT DES RABEN'),
        ('T8p', dict(g2=False, world=False), [], 'g1', 'KEIN VOLLSTÄNDIGES GOTHIC 1'),
        # Othello-Inhalt unter anderem Namen wird trotzdem erkannt
        ('T8m', dict(g2=False), [('Data/Eigene.vdf', OTH_SCRIPTS), ('Data/Welten.vdf', OTH_WORLDS)], 'g1', 'OTHELLO GEFUNDEN')):
    d = game(name, **kw)
    extras(d, extra)
    rc = run('/still', '/spiel:' + key, '/ordner:' + d)
    log = last_log(d) if os.path.isdir(os.path.join(d, 'CoopLogs', 'Installer')) else ''
    check('%s %-8s Rueckgabe 1, nichts geschrieben, "%s"' % (name, key, want),
          rc == 1 and not os.path.exists(os.path.join(d, 'system', 'Autorun')) and not os.path.exists(os.path.join(d, 'CoopBackup'))
          and want in log, rc)
d = game('T8k', g2=False, exe_name='GOTHIC.EXE')
rc = run('/still', '/spiel:g1', '/ordner:' + d)
check('T8k Gothic 1 nur mit GOTHIC.EXE: Rueckgabe 0', rc == 0 and 'GOTHIC.EXE 1.0.8.11' in last_log(d), rc)
d = game('T8n', g2=False)
extras(d, [('Data/Eigene.vdf', OTH_SCRIPTS), ('Data/Welten.vdf', OTH_WORLDS)])
rc = run('/still', '/spiel:othello', '/ordner:' + d)
check('T8n Othello unter anderem VDF-Namen: Rueckgabe 0, Othello-Skripte', rc == 0 and sha(os.path.join(d, 'system', 'Autorun', 'COOP_MENU.D')) ==
      sha(os.path.join(rp(pfade.SKRIPTE['othello']), 'COOP_MENU.D')), rc)

print('T9 Gothic 1 / 2 mit Mods (eigene VDF in Data, .mod in Data\\modvdf)')
for name, g2, key, scripts in (('T9a', False, 'g1', 'g1'), ('T9b', True, 'g2', 'g2')):
    d = game(name, g2=g2)
    extras(d, [MOD, 'Data/modvdf/TestMod.mod'])
    rc = run('/still', '/spiel:' + key, '/ordner:' + d)
    check('%s %s mit Mod: Rueckgabe 0, Coop-Skripte %s' % (name, key, scripts),
          rc == 0 and sha(os.path.join(d, 'system', 'Autorun', 'COOP_MENU.D')) == sha(os.path.join(rp(pfade.SKRIPTE[scripts]), 'COOP_MENU.D')), rc)

print('T7 Union fehlt: Union 1.0m mitinstallieren (u62: Dateien statt Setup)')


def union_ok(d):
    return all(os.path.exists(os.path.join(d, *rel.split('/'))) and sha(os.path.join(d, *rel.split('/'))) == want.lower()
               for rel, _, want in UNION_FILES)


for name, g2, key, dll in (('T7a', True, 'g2', dll2), ('T7e', False, 'g1', dll1)):
    d = game(name, g2=g2, union=False)
    touch(os.path.join(d, 'system', 'dinput.dll'), b'SystemPack-Lader')
    before = tree(d)
    rc = run('/still', '/union', '/spiel:' + key, '/ordner:' + d)
    log = last_log(d)
    check('%s %s: Rueckgabe 0' % (name, key), rc == 0, rc)
    check('Union-Dateien da (Pruefsummen)', union_ok(d))
    check('dinput.dll entfernt', not os.path.exists(os.path.join(d, 'system', 'dinput.dll')))
    bak = [os.path.join(r, f) for r, _, fs in os.walk(os.path.join(d, 'CoopBackup')) for f in fs if f.lower() == 'dinput.dll.sicherung']
    check('dinput.dll gesichert', len(bak) == 1 and open(bak[0], 'rb').read() == b'SystemPack-Lader')
    run_dir = os.path.dirname(os.path.dirname(bak[0])) if bak else ''
    man = open(os.path.join(run_dir, 'COOPINSTALL.TXT'), encoding='utf-8').read() if run_dir else ''
    check('Protokoll: Union mitinstalliert, kein Hinweis', 'mitinstalliert' in log and 'HINWEIS: Union fehlt' not in log)
    check('Liste: entfernt + Union neu', 'datei=entfernt|' in man and 'datei=neu|' in man and 'Union.patch' in man)
    check('Coop-DLL', sha(os.path.join(d, 'system', 'Autorun', os.path.basename(dll).replace('-' + fassung, ''))) == sha(dll))
    rc = run('/still', '/rueckgaengig', '/ordner:' + d)
    check('rueckgaengig: Ordner wie vorher (Union weg, dinput.dll zurueck)', rc == 0 and tree(d) == before,
          sorted(set(tree(d)) ^ set(before))[:5])

d = game('T7b', union=False)
rc = run('/still', '/spiel:g2', '/ordner:' + d)
check('T7b ohne /union: Rueckgabe 0, kein Union, Hinweis', rc == 0 and not os.path.exists(os.path.join(d, 'system', 'Union.patch')) and
      'HINWEIS: Union fehlt' in last_log(d), rc)

d = game('T7c', g2=False, union=False, othello=True)
rc = run('/still', '/union', '/spiel:othello', '/ordner:' + d)
check('T7c Othello ohne Union: Rueckgabe 0, kein Union, Hinweis', rc == 0 and not os.path.exists(os.path.join(d, 'system', 'Union.patch')) and
      'bei Othello' in last_log(d), rc)

d = game('T7d')
touch(os.path.join(d, 'system', 'dinput.dll'), b'bleibt')
rc = run('/still', '/union', '/spiel:g2', '/ordner:' + d)
check('T7d Union vorhanden: unberuehrt', rc == 0 and open(os.path.join(d, 'system', 'Shw32.dll'), 'rb').read() == b'x' and
      not os.path.exists(os.path.join(d, 'Data', 'Union.vdf')) and open(os.path.join(d, 'system', 'dinput.dll'), 'rb').read() == b'bleibt', rc)

print('ALLES OK' if ok_all else 'FEHLER')
