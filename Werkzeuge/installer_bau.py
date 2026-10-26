"""installer_bau.py uNN [--gd3d11-zip <GD3D11-v17.9.7.zip>] [--union-dir <Ordner>] [--ausgabe <exe>] [--force] - Installer bauen (ab u53).

Packt alles, was der Installer schreibt, in eine Nutzlast (installer\\obj\\payload.bin, Format in GothicCoopSetup.cpp
LoadPayload; je Datei LZMS ueber die Windows-Compression-API und SHA-256), uebersetzt installer\\GothicCoopSetup.cpp mit den
VS2019 Build Tools (x86, statisch gelinkt) und prueft das Ergebnis (Aufruf mit /pruefen). Ausgabe:
builds\\zip\\Gothic Union Coop Setup uNN.exe (--ausgabe: anderer Pfad). Projektversion fuer die Versionsangaben aus versionen.json.

Teile (wie in GothicCoopSetup.cpp):
  1 Coop Gothic 1      Gothic1Coop.dll, scripts\\g1, Config (nur wenn neu), Steam (App 65540), LIESMICH (makezip_g1)
  2 Coop Gothic 2      Gothic2Coop.dll, scripts\\g2, Config (nur wenn neu), Steam (App 39510), LIESMICH (makezip_g2)
  3 Coop Othello       Gothic1Coop.dll, scripts\\othello, Config von G1 (nur wenn neu), Steam (App 65540), LIESMICH-Othello
  4 Renderer G1        gd3d11\\g1_avx2-v17.9.7-coop8.dll -> system\\GD3D11\\Bin\\g1_avx2.dll
  5 Renderer G2        gd3d11\\g2a_avx2-v17.9.7-coop8.dll -> system\\GD3D11\\Bin\\g2a_avx2.dll
  6 GD3D11 v17.9.7     das offizielle Release (ZIP, nur gelesen - nichts daraus wird ausgefuehrt), alles nach system\\
  7 Lizenzen           GothicCoop-Lizenzen\\ (AGPL, GPL, ENet, json, Hinweise, GD3D11-Patches coop6-coop8)
  8 Union 1.0m         u62: die sieben Dateien von Union (UNION_FILES, docs\\UNION-DATEIEN.md) aus einem Spielordner mit Union
                       (Vorgabe: Union Vorlage\\Gothic II, sonst Gothic II) - der Installer schreibt sie, wenn bei Gothic 1/2
                       Union fehlt (Othello bringt Union selbst mit). Bis u61: Union_1.0m_26.06.2022.exe, gestartet vom Installer
"""
import argparse, ctypes, hashlib, io, json, os, struct, subprocess, sys, zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TOOLS = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, TOOLS)
import pfade   # u62: Pfade im Repo (oeffentlicher Export: eigener Aufbau)
INST = os.path.join(ROOT, pfade.INSTALLER)
OBJ = os.path.join(INST, 'obj')
VCVARS = r'C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat'

F_KEEP, F_STORED = 1, 2
# u62: Union 1.0m (Gratt / WOG RU Community) - die Dateien, die sein Installer schreibt (fuer G1 und G2 gleich), Groesse, SHA-256
UNION_FILES = [
    ('Data/Union.vdf', 2037609, 'B5E29DC8B7B7E38EC446DACE3A4DFD15B203DFA7D134508D6286B03472CFF19A'),
    ('system/Union.patch', 251001, '77BAAABC57FC57CBE161BBA609AE80311B58B5AC7E126DDEF1DCF40EA934B45B'),
    ('system/Shw32.dll', 4396032, '78D9312F07EBDF978D301FDE63841DDCBE265B59CB7C2753E01D2DBDC5742888'),
    ('system/Vdfs32g.dll', 349696, '6E5D544BA5139725A42729CBE6C243A41B5460310806C371B03AAC79657DC607'),
    ('system/stdhost.exe', 72704, 'C5632644B2D365F6FAA9F21C564E1141B4EE7A1F11DE2666B976BF3E5EFB8DA3'),
    ('system/msvcp100.dll', 421200, 'A3BA6421991241BEA9C8334B62C3088F8F131AB906C3CC52113945D05016A35F'),
    ('system/msvcr100.dll', 770384, '1221A09484964A6F38AF5E34EE292B9AFEFCCB3DC6E55435FD3AAF7C235D9067'),
]


def union_files(folder):
    """die sieben Union-Dateien aus einem Spielordner, geprueft gegen UNION_FILES - [(Pfad, Daten)]"""
    out = []
    for rel, size, want in UNION_FILES:
        p = os.path.join(folder, *rel.split('/'))
        data = open(p, 'rb').read()
        if len(data) != size or hashlib.sha256(data).hexdigest().upper() != want:
            sys.exit('%s ist nicht die Datei von Union 1.0m (SHA-256 %s)' % (p, sha16(data)))
        out.append((rel, data))
    return out


def versionen():
    """versionen.json: Projektversion und Fassung je Spiel/Mod"""
    return json.load(open(os.path.join(ROOT, 'versionen.json'), encoding='utf-8'))


def sha16(b):
    return hashlib.sha256(b).hexdigest()[:16].upper()


def readme_text(text):
    return text.replace('\r\n', '\n').replace('\n', '\r\n').encode('cp1252')


# --- LZMS ueber cabinet.dll (wie Decompress im Installer) ---
_cab = ctypes.WinDLL('cabinet')
_cab.CreateCompressor.argtypes = [ctypes.c_uint32, ctypes.c_void_p, ctypes.POINTER(ctypes.c_void_p)]
_cab.CreateCompressor.restype = ctypes.c_int
_cab.Compress.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t)]
_cab.Compress.restype = ctypes.c_int
_cab.CloseCompressor.argtypes = [ctypes.c_void_p]
COMPRESS_ALGORITHM_LZMS = 5


def lzms(data):
    if not data:
        return None
    h = ctypes.c_void_p()
    if not _cab.CreateCompressor(COMPRESS_ALGORITHM_LZMS, None, ctypes.byref(h)):
        return None
    try:
        src = ctypes.create_string_buffer(data, len(data))
        need = ctypes.c_size_t(0)
        _cab.Compress(h, src, len(data), None, 0, ctypes.byref(need))
        if not need.value:
            return None
        out = ctypes.create_string_buffer(need.value)
        got = ctypes.c_size_t(0)
        if not _cab.Compress(h, src, len(data), out, need.value, ctypes.byref(got)):
            return None
        return out.raw[:got.value]
    finally:
        _cab.CloseCompressor(h)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('fassung')
    ap.add_argument('--gd3d11-zip', default=os.path.join(os.environ.get('USERPROFILE', ''), 'Downloads', 'GD3D11-v17.9.7.zip'))
    ap.add_argument('--gd3d11-ver', default='v17.9.7')
    ap.add_argument('--union-dir', default=pfade.union_vorlage(ROOT),
                    help='Spielordner mit Union 1.0m (die Dateien werden gegen UNION_FILES geprueft)')
    ap.add_argument('--ausgabe', help='anderer Zielpfad')
    ap.add_argument('--force', action='store_true', help='vorhandenen Installer ersetzen')
    a = ap.parse_args()
    f = a.fassung
    projekt = versionen()['projekt']
    out = a.ausgabe or os.path.join(ROOT, pfade.zip_ziel('Gothic-I-II-Universal-Coop Setup %s.exe' % f))
    if os.path.exists(out) and not a.force:
        sys.exit('%s gibt es schon - mit --force ersetzen' % out)
    union = union_files(a.union_dir)

    import makezip_g1, makezip_g2   # nur die LIESMICH-Texte (main laeuft beim Import nicht)

    def src(p):
        return open(os.path.join(ROOT, p), 'rb').read()

    entries = []   # (Teil, Merker, Pfad im Spielordner, Daten)

    def coop(comp, dll, scripts, appid, liesmich):
        entries.append((comp, 0, 'system/Autorun/%s' % os.path.basename(dll).replace('-%s' % f, ''), src(dll)))
        for s in ('COOP_MENU.D', 'COOP_ITEMS.D', 'PC_MULTIPLAYER_HERO.D'):
            entries.append((comp, 0, 'system/Autorun/' + s, src(os.path.join(scripts, s))))
        cfg = os.path.join(scripts, 'GothicCoopConfig.json')
        if not os.path.exists(os.path.join(ROOT, cfg)):
            cfg = os.path.join(pfade.SKRIPTE['g1'], 'GothicCoopConfig.json')
        entries.append((comp, F_KEEP, 'system/GothicCoopConfig.json', src(cfg)))
        entries.append((comp, 0, 'system/steam_api.dll', src(pfade.STEAM_API)))
        entries.append((comp, 0, 'system/steam_appid.txt', src(appid)))
        entries.append((comp, 0, 'GothicCoop-LIESMICH.txt', liesmich))

    install_note = ('\r\nInstalliert mit dem Installer von "Gothic-I-II-Universal-Coop" {p} (Coop {f}). Rueckgaengig: den Installer\r\n'
                    'noch einmal starten, Spielordner waehlen, "Letzte Installation rueckgaengig" - ersetzte Dateien liegen in\r\n'
                    'CoopBackup\\<Zeit>\\, das Protokoll in CoopLogs\\Installer\\. Fehlte Union, hat der Installer Union 1.0m mitinstalliert\r\n'
                    '(system\\dinput.dll von SystemPack liegt dann in CoopBackup) - das Rueckgaengig nimmt auch das wieder heraus.\r\n'
                    ).format(f=f, p=projekt).encode('cp1252')
    coop(1, pfade.dll(f, 'g1'), pfade.SKRIPTE['g1'], pfade.STEAM_APPID['g1'], readme_text(makezip_g1.LIESMICH.format(f=f)) + install_note)
    coop(2, pfade.dll(f, 'g2'), pfade.SKRIPTE['g2'], pfade.STEAM_APPID['g2'], readme_text(makezip_g2.LIESMICH.format(f=f)) + install_note)
    coop(3, pfade.dll(f, 'g1'), pfade.SKRIPTE['othello'], pfade.STEAM_APPID['g1'],
         readme_text(open(os.path.join(INST, 'texte', 'LIESMICH-Othello.txt'), encoding='utf-8').read().format(f=f)))
    entries.append((4, 0, 'system/GD3D11/Bin/g1_avx2.dll', src(pfade.RENDERER['g1'])))
    entries.append((5, 0, 'system/GD3D11/Bin/g2a_avx2.dll', src(pfade.RENDERER['g2'])))

    z = zipfile.ZipFile(a.gd3d11_zip)
    if z.testzip():
        sys.exit('GD3D11-ZIP fehlerhaft')
    n6 = 0
    for i in z.infolist():
        if i.is_dir():
            continue
        name = i.filename.replace('\\', '/')
        if name.startswith('/') or '..' in name:
            sys.exit('GD3D11-ZIP: verdaechtiger Pfad %s' % name)
        entries.append((6, F_KEEP if name.lower().endswith('.ini') else 0, 'system/' + name, z.read(i)))
        n6 += 1
    lz, tx = pfade.LIZENZEN, os.path.join(pfade.INSTALLER, 'texte')
    for p, n in ([(os.path.join(lz, 'GothicCoop-AGPL-3.0.txt'), None), (os.path.join(lz, 'GD3D11-GPL-3.0.txt'), None),
                  (os.path.join(lz, 'ENet-LICENSE.txt'), None), (os.path.join(lz, 'nlohmann-json-LICENSE.MIT.txt'), None),
                  (os.path.join(tx, 'GothicCoop-HINWEIS.txt'), None), (os.path.join(tx, 'GD3D11-HERKUNFT.txt'), None)] +
                 [(p, 'GD3D11-' + os.path.basename(p)) for p in pfade.RENDERER_PATCHES]):
        entries.append((7, 0, 'GothicCoop-Lizenzen/' + (n or os.path.basename(p)), src(p)))
    for rel, data in union:
        entries.append((8, 0, rel, data))

    seen = set()
    for comp, _, path, _ in entries:
        if (comp, path.lower()) in seen:
            sys.exit('doppelt: Teil %d %s' % (comp, path))
        seen.add((comp, path.lower()))

    # Nutzlast
    head = io.BytesIO()
    blob = io.BytesIO()
    head.write(b'GCPK' + struct.pack('<I', 1))
    for s in (f, a.gd3d11_ver):
        b = s.encode('utf-8')
        head.write(struct.pack('<H', len(b)) + b)
    head.write(struct.pack('<I', len(entries)))
    raw_total = packed_total = 0
    per = {}
    for comp, flags, path, data in entries:
        c = lzms(data)
        if c is None or len(c) >= len(data):
            c, flags = data, flags | F_STORED
        b = path.encode('utf-8')
        head.write(struct.pack('<BBH', comp, flags, len(b)) + b + struct.pack('<QQQ', len(data), len(c), blob.tell()) +
                   hashlib.sha256(data).digest())
        blob.write(c)
        raw_total += len(data)
        packed_total += len(c)
        k = per.setdefault(comp, [0, 0, 0])
        k[0] += 1
        k[1] += len(data)
        k[2] += len(c)
    os.makedirs(OBJ, exist_ok=True)
    payload = os.path.join(OBJ, 'payload.bin')
    open(payload, 'wb').write(head.getvalue() + blob.getvalue())
    names = {1: 'Coop Gothic 1', 2: 'Coop Gothic 2', 3: 'Coop Othello', 4: 'Renderer G1 coop8', 5: 'Renderer G2 coop8',
             6: 'GD3D11 ' + a.gd3d11_ver, 7: 'Lizenzen', 8: 'Union 1.0m'}
    for comp in sorted(per):
        n, r, p = per[comp]
        print('Teil %d %-18s %5d Dateien %10d -> %9d Bytes' % (comp, names[comp], n, r, p))
    print('Nutzlast: %d Dateien, %d -> %d Bytes' % (len(entries), raw_total, os.path.getsize(payload)))

    # Ressourcen (Nutzlast, Fassung) und Bau
    esc = payload.replace('\\', '\\\\')
    num = ''.join(ch for ch in f if ch.isdigit()) or '0'
    open(os.path.join(OBJ, 'setup_gen.rc2'), 'w', encoding='ascii').write(
        'IDR_PAYLOAD RCDATA "%s"\n\n'
        'VS_VERSION_INFO VERSIONINFO\n FILEVERSION 1,0,%s,0\n PRODUCTVERSION 1,0,%s,0\n FILEOS 0x40004\n FILETYPE 0x1\n'
        'BEGIN\n BLOCK "StringFileInfo"\n BEGIN\n  BLOCK "040704b0"\n  BEGIN\n'
        '   VALUE "CompanyName", "Gothic-I-II-Universal-Coop"\n   VALUE "FileDescription", "Gothic-I-II-Universal-Coop %s Setup (Coop %s)"\n'
        '   VALUE "FileVersion", "%s"\n   VALUE "ProductName", "Gothic-I-II-Universal-Coop"\n   VALUE "ProductVersion", "%s"\n'
        '   VALUE "LegalCopyright", "AGPL-3.0 (GothicCoop), GPL-3.0 (GD3D11)"\n  END\n END\n'
        ' BLOCK "VarFileInfo"\n BEGIN\n  VALUE "Translation", 0x407, 1200\n END\nEND\n' % (esc, num, num, projekt, f, f, projekt))
    exe = os.path.join(OBJ, 'GothicCoopSetup.exe')
    if os.path.exists(exe):
        os.remove(exe)
    bat = os.path.join(OBJ, 'bau.bat')
    open(bat, 'w', encoding='ascii').write(
        '@echo off\r\ncall "%s" x86 >nul\r\ncd /d "%s"\r\n'
        'rc /nologo /fo obj\\GothicCoopSetup.res GothicCoopSetup.rc || exit /b 1\r\n'
        'cl /nologo /O2 /MT /EHsc /W3 /utf-8 /DUNICODE /D_UNICODE /Foobj\\ /Fe"obj\\GothicCoopSetup.exe" GothicCoopSetup.cpp '
        'obj\\GothicCoopSetup.res /link /SUBSYSTEM:WINDOWS /MANIFEST:NO || exit /b 1\r\n' % (VCVARS, INST))
    r = subprocess.run(['cmd', '/c', bat], capture_output=True, text=True, errors='replace')
    lines = [l for l in (r.stdout + r.stderr).splitlines() if l.strip() and 'vswhere' not in l and 'konnte nicht gefunden werden' not in l
             and l.strip() != 'GothicCoopSetup.cpp']   # vcvarsall meldet ein fehlendes vswhere.exe - ohne Folgen
    for l in lines:
        print('  ' + l)
    if r.returncode or not os.path.exists(exe):
        sys.exit('Bau fehlgeschlagen')

    # Pruefen: der fertige Installer entpackt alles im Speicher und vergleicht die Pruefsummen
    report = os.path.join(OBJ, 'pruefen.txt')
    if os.path.exists(report):
        os.remove(report)
    rc = subprocess.run([exe, '/pruefen', '/bericht:' + report]).returncode
    last = open(report, encoding='utf-8').read().strip().splitlines()[-1] if os.path.exists(report) else 'kein Bericht'
    print('Pruefen: Rueckgabe %d - %s' % (rc, last))
    if rc != 0:
        sys.exit('Installer prueft seine Nutzlast nicht fehlerfrei')
    os.makedirs(os.path.dirname(out), exist_ok=True)
    data = open(exe, 'rb').read()
    open(out, 'wb').write(data)
    print('%s  %d Bytes  SHA-256 %s' % (out, len(data), sha16(data)))


if __name__ == '__main__':
    main()
