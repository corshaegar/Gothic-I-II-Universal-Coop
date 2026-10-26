"""pfade.py - wo die Paket-Skripte (makezip_g1/g2, installer_bau, installer_test) ihre Dateien finden.

Alles relativ zum Wurzelordner des Repos. Was nicht im Repo liegt (gebaute Renderer, steam_api.dll, Union 1.0m), gehoert nach
Bau\\eingaben\\ (siehe README.md, Abschnitt "Bauen").
"""
import os

SKRIPTE = {'g1': os.path.join('Gothic I', 'Skripte'), 'g2': os.path.join('Gothic II', 'Skripte'), 'othello': os.path.join('Othello', 'Skripte')}
EINGABEN = os.path.join('Bau', 'eingaben')
STEAM_API = os.path.join(EINGABEN, 'steam_api.dll')
STEAM_APPID = {'g1': os.path.join('Gothic I', 'steam_appid.txt'), 'g2': os.path.join('Gothic II', 'steam_appid.txt')}
RENDERER = {'g1': os.path.join(EINGABEN, 'g1_avx2-v17.9.7-coop8.dll'), 'g2': os.path.join(EINGABEN, 'g2a_avx2-v17.9.7-coop8.dll'),
            'g2_original': os.path.join(EINGABEN, 'g2a_avx2-v17.9.7-original.dll')}
RENDERER_PATCHES = [os.path.join('Renderer', 'coop%d.patch' % n) for n in (6, 7, 8)]
LIZENZEN = 'Lizenzen'
INSTALLER = 'Installer'
WERKZEUGE = 'Werkzeuge'
BAU = 'Bau'   # Bau\uNN\Gothic1Coop-uNN.dll bzw. Gothic2Coop-uNN.dll, Bau\zip\ die Pakete
UNION_VORLAGE = [os.path.join(EINGABEN, 'Union')]   # Ordner mit Data\Union.vdf und system\... von Union 1.0m


def dll(fassung, spiel):
    """gebaute Coop-DLL einer Fassung ('g1' oder 'g2'; Othello nimmt die von G1)"""
    return os.path.join(BAU, fassung, ('Gothic1Coop-%s.dll' if spiel == 'g1' else 'Gothic2Coop-%s.dll') % fassung)


def zip_ziel(name):
    return os.path.join(BAU, 'zip', name)


def union_vorlage(root):
    for p in UNION_VORLAGE:
        if os.path.isdir(os.path.join(root, p)):
            return os.path.join(root, p)
    return os.path.join(root, UNION_VORLAGE[-1])
