// GothicCoop-Installer (ab u53). Nutzer: "einen Installer erstellen. Dort kann man dann waehlen ob man Gothic 1 / 2 oder Othello
// installiert hat, dann wird der entsprechende Coop installiert. Falls nicht vorhanden wird auch der Renderer installiert und der
// Coop renderer darueber gelegt".
//
// Ablauf: Spiel waehlen (Gothic 1, Gothic 2 Die Nacht des Raben, Gothic 1 mit Othello), Spielordner waehlen (Steam, GOG und
// uebliche Orte werden gesucht), Installieren. Geschrieben werden:
//  - der Coop des Spiels: system\Autorun\Gothic1Coop.dll bzw. Gothic2Coop.dll, die Skripte (COOP_MENU.D, COOP_ITEMS.D,
//    PC_MULTIPLAYER_HERO.D - fuer Othello eigene), system\GothicCoopConfig.json (nur, wenn es sie noch nicht gibt), Steam-Dateien,
//    GothicCoop-LIESMICH.txt und GothicCoop-Lizenzen\ im Spielordner;
//  - mit "Coop-Renderer": fehlt GD3D11 oder ist es eine andere Fassung als v17.9.7, erst GD3D11 v17.9.7 (offizielles Release),
//    dann darueber der Coop-Renderer (system\GD3D11\Bin\g1_avx2.dll bzw. g2a_avx2.dll, coop8 - nur zu den Shadern von v17.9.7
//    passend). Ein ddraw.dll ohne GD3D11-Ordner gehoert einem anderen Programm - ersetzt wird es nur auf Nachfrage.
// Alles, was ersetzt wird, kommt vorher nach <Spiel>\CoopBackup\<Zeit>\<Pfad>.sicherung (Endung: Union laedt Plugin-DLLs aus
// jedem Unterordner, P83), die Liste nach COOPINSTALL.TXT dort; "Letzte Installation rueckgaengig" stellt das wieder her und
// entfernt, was neu dazukam. Protokoll: <Spiel>\CoopLogs\Installer\<Zeit>.log - nie geloescht.
// Union: Nutzer "Union sollte vielleciht auch direkt installiert werden" / "außer bei othello da ist es ja schon dabei". u62
// (Nutzer: "ohne Installer sondern die Dateien von Union werden direkt reingepackt. Das macht jede Mod so Othello zb auch"):
// Teil 8 sind die sieben Dateien von Union 1.0m (docs\UNION-DATEIEN.md). Fehlt Union bei Gothic 1/2, kommen sie (nach
// Rueckfrage, ohne Oberflaeche mit /union) in den Plan wie alles andere, system\dinput.dll (Lader von SystemPack - Union laedt
// SystemPack selbst) wird gesichert und entfernt; "rueckgaengig" nimmt beides zurueck. Ist Union da, bleibt es unberuehrt.
// Bei Othello (bringt Union mit) nur ein Hinweis. Bis u61 startete der Installer das mitgelieferte Union-Setup.
// Spiel (u54): GameCheck laesst nur installieren, wenn der Ordner das gewaehlte Spiel ist (Gothic 2 DNdR 2.6, Gothic 1 1.08k -
// je mit oder ohne Mods - oder Othello vollstaendig; erkannt am Inhalt, Merkmale bei GameInfo), auch mit /still. Nach einer gelungenen Installation meldet ein Fenster
// "fertig installiert", OK beendet den Installer.
//
// Nutzlast: Ressource IDR_PAYLOAD von tools\installer_bau.py (Format in LoadPayload), je Datei LZMS ueber die Windows-
// Compression-API (cabinet.dll, ab Windows 8) und SHA-256 (BCrypt).
// Ohne Oberflaeche: /still /spiel:g1|g2|othello /ordner:"..." [/ohnerenderer] [/ddraw] [/union] [/rueckgaengig] - Rueckgabe
// 0 gut, 1 Fehler, 2 Aufruf falsch, 3 keine Schreibrechte; /union installiert bei fehlendem Union die Union-Dateien mit.
// /ohneunion (mit Oberflaeche): nicht nach Union fragen. /pruefen [/bericht:"Datei"]: alles im Speicher entpacken und pruefen.

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <compressapi.h>
#include <bcrypt.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cwctype>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "cabinet.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "version.lib")

#ifndef PF_AVX2_INSTRUCTIONS_AVAILABLE
#define PF_AVX2_INSTRUCTIONS_AVAILABLE 40
#endif

// --- Sprache -----------------------------------------------------------------------------------------------------------
static bool En = false;   // Windows-Oberflaeche nicht Deutsch: Englisch

static const wchar_t* T(const wchar_t* de, const wchar_t* en) {
    return En ? en : de;
}

// --- Nutzlast ------------------------------------------------------------------------------------------------------------
enum Comp { C_COOP_G1 = 1, C_COOP_G2 = 2, C_COOP_OTH = 3, C_RND_G1 = 4, C_RND_G2 = 5, C_GD3D11 = 6, C_DOCS = 7, C_UNION = 8 };
enum EntryFlags { F_KEEP = 1, F_STORED = 2 };
enum Game { G_G1 = 0, G_G2 = 1, G_OTH = 2, G_NONE = -1 };

struct Entry {
    int comp = 0, flags = 0;
    std::wstring path;   // relativ zum Spielordner, mit '\'
    unsigned long long raw = 0, packed = 0, off = 0;
    unsigned char sha[32] = {};
};

static std::wstring Fassung, Gd3d11Ver;
static std::vector<Entry> Entries;
static const unsigned char* Blob = NULL;
static unsigned long long BlobSize = 0;

static std::wstring FromUtf8(const char* s, int n) {
    if (n <= 0)
        return std::wstring();
    const int w = MultiByteToWideChar(CP_UTF8, 0, s, n, NULL, 0);
    std::wstring r(w, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s, n, &r[0], w);
    return r;
}

static std::string ToUtf8(const std::wstring& s) {
    if (s.empty())
        return std::string();
    const int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), NULL, 0, NULL, NULL);
    std::string r(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), &r[0], n, NULL, NULL);
    return r;
}

// "GCPK", u32 Fassung des Formats (1), u16 + UTF-8 Coop-Fassung, u16 + UTF-8 GD3D11-Fassung, u32 Anzahl, je Datei: u8 Teil,
// u8 Merker, u16 + UTF-8 Pfad ('/'), u64 Groesse, u64 gepackt, u64 Abstand im Datenteil, 32 Byte SHA-256; danach der Datenteil.
static bool LoadPayload(std::wstring& why) {
    HRSRC r = FindResourceW(NULL, MAKEINTRESOURCEW(IDR_PAYLOAD), (LPCWSTR)RT_RCDATA);
    HGLOBAL g = r ? LoadResource(NULL, r) : NULL;
    const unsigned char* p = g ? (const unsigned char*)LockResource(g) : NULL;
    const unsigned long long n = r ? SizeofResource(NULL, r) : 0;
    if (!p || n < 16) {
        why = L"Nutzlast fehlt";
        return false;
    }
    unsigned long long pos = 0;
    bool bad = false;
    auto need = [&](unsigned long long k) { if (pos + k > n) bad = true; return !bad; };
    auto u8 = [&]() -> unsigned { if (!need(1)) return 0; return p[pos++]; };
    auto u16 = [&]() -> unsigned { if (!need(2)) return 0; unsigned v = p[pos] | (p[pos + 1] << 8); pos += 2; return v; };
    auto u32 = [&]() -> unsigned { if (!need(4)) return 0; unsigned v = p[pos] | (p[pos + 1] << 8) | (p[pos + 2] << 16) | ((unsigned)p[pos + 3] << 24); pos += 4; return v; };
    auto u64 = [&]() -> unsigned long long { unsigned long long lo = u32(), hi = u32(); return lo | (hi << 32); };
    auto str = [&]() -> std::wstring { const unsigned k = u16(); if (!need(k)) return std::wstring(); std::wstring s = FromUtf8((const char*)p + pos, (int)k); pos += k; return s; };
    if (memcmp(p, "GCPK", 4) != 0) {
        why = L"Nutzlast ohne Kennung";
        return false;
    }
    pos = 4;
    if (u32() != 1) {
        why = L"Nutzlast in unbekannter Fassung";
        return false;
    }
    Fassung = str();
    Gd3d11Ver = str();
    const unsigned count = u32();
    Entries.clear();
    for (unsigned i = 0; i < count && !bad; i++) {
        Entry e;
        e.comp = (int)u8();
        e.flags = (int)u8();
        e.path = str();
        std::replace(e.path.begin(), e.path.end(), L'/', L'\\');
        e.raw = u64();
        e.packed = u64();
        e.off = u64();
        if (need(32)) {
            memcpy(e.sha, p + pos, 32);
            pos += 32;
        }
        Entries.push_back(e);
    }
    if (bad) {
        why = L"Nutzlast abgeschnitten";
        return false;
    }
    Blob = p + pos;
    BlobSize = n - pos;
    for (auto& e : Entries)
        if (e.off + e.packed > BlobSize || e.path.empty() || e.path.find(L"..") != std::wstring::npos || e.path[0] == L'\\') {
            why = L"Nutzlast-Eintrag ungueltig: " + e.path;
            return false;
        }
    return true;
}

static bool Sha256(const void* d, size_t n, unsigned char out[32]) {
    static const unsigned char none = 0;
    BCRYPT_ALG_HANDLE a = NULL;
    BCRYPT_HASH_HANDLE h = NULL;
    bool ok = false;
    if (BCryptOpenAlgorithmProvider(&a, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0) {
        if (BCryptCreateHash(a, &h, NULL, 0, NULL, 0, 0) == 0) {
            ok = BCryptHashData(h, (PUCHAR)(n ? d : &none), (ULONG)n, 0) == 0 && BCryptFinishHash(h, out, 32, 0) == 0;
            BCryptDestroyHash(h);
        }
        BCryptCloseAlgorithmProvider(a, 0);
    }
    return ok;
}

static bool Unpack(const Entry& e, std::vector<unsigned char>& out) {
    out.assign((size_t)e.raw, 0);
    const unsigned char* src = Blob + e.off;
    if (e.raw == 0) {
    }
    else if (e.flags & F_STORED) {
        if (e.packed != e.raw)
            return false;
        memcpy(out.data(), src, (size_t)e.raw);
    }
    else {
        DECOMPRESSOR_HANDLE h = NULL;
        if (!CreateDecompressor(COMPRESS_ALGORITHM_LZMS, NULL, &h))
            return false;
        SIZE_T got = 0;
        const BOOL ok = Decompress(h, src, (SIZE_T)e.packed, out.data(), (SIZE_T)e.raw, &got);
        CloseDecompressor(h);
        if (!ok || got != e.raw)
            return false;
    }
    unsigned char sha[32];
    return Sha256(out.data(), out.size(), sha) && memcmp(sha, e.sha, 32) == 0;
}

static const Entry* FindEntry(int comp, const wchar_t* path) {
    for (auto& e : Entries)
        if (e.comp == comp && _wcsicmp(e.path.c_str(), path) == 0)
            return &e;
    return NULL;
}

// u62: die mitgelieferten Union-Dateien (Teil 8, Pfade im Spielordner) - Anzahl
static int UnionFiles() {
    int n = 0;
    for (auto& e : Entries)
        if (e.comp == C_UNION)
            n++;
    return n;
}

static const wchar_t* const UNION_REMOVE = L"system\\dinput.dll";   // u62: entfernt der Union-Installer (SystemPack-Lader)

// --- Dateien -------------------------------------------------------------------------------------------------------------
static bool FileExists(const std::wstring& f) {
    const DWORD a = GetFileAttributesW(f.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool DirExists(const std::wstring& d) {
    const DWORD a = GetFileAttributesW(d.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool ReadAll(const std::wstring& f, std::vector<unsigned char>& d) {
    HANDLE h = CreateFileW(f.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    LARGE_INTEGER sz;
    bool ok = GetFileSizeEx(h, &sz) && sz.QuadPart < (1LL << 31);
    if (ok) {
        d.resize((size_t)sz.QuadPart);
        DWORD got = 0;
        ok = d.empty() || (ReadFile(h, d.data(), (DWORD)d.size(), &got, NULL) && got == d.size());
    }
    CloseHandle(h);
    return ok;
}

static bool FileSha(const std::wstring& f, unsigned char sha[32]) {
    std::vector<unsigned char> d;
    return ReadAll(f, d) && Sha256(d.data(), d.size(), sha);
}

static std::wstring Hex(const unsigned char* s, int n) {
    static const wchar_t* x = L"0123456789ABCDEF";
    std::wstring r;
    for (int i = 0; i < n; i++) {
        r += x[s[i] >> 4];
        r += x[s[i] & 15];
    }
    return r;
}

static std::wstring Parent(const std::wstring& p) {
    const size_t s = p.find_last_of(L'\\');
    return s == std::wstring::npos ? std::wstring() : p.substr(0, s);
}

static DWORD MakeDirs(const std::wstring& d) {
    if (d.empty() || DirExists(d))
        return 0;
    const DWORD e = MakeDirs(Parent(d));
    if (e)
        return e;
    if (!CreateDirectoryW(d.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        return GetLastError();
    return 0;
}

// Datei sicher ersetzen: <Ziel>.coopneu schreiben, dann tauschen - ein Abbruch laesst das Ziel stehen
static DWORD WriteReplace(const std::wstring& f, const void* d, size_t n) {
    DWORD err = MakeDirs(Parent(f));
    if (err)
        return err;
    const std::wstring tmp = f + L".coopneu";
    HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return GetLastError();
    DWORD put = 0;
    const bool ok = n == 0 || (WriteFile(h, d, (DWORD)n, &put, NULL) && put == n);
    err = ok ? 0 : GetLastError();
    CloseHandle(h);
    if (ok) {
        SetFileAttributesW(f.c_str(), FILE_ATTRIBUTE_NORMAL);
        if (!MoveFileExW(tmp.c_str(), f.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
            err = GetLastError();
    }
    if (err)
        DeleteFileW(tmp.c_str());
    return err;
}

static std::wstring Stamp() {
    SYSTEMTIME t;
    GetLocalTime(&t);
    wchar_t b[40];
    swprintf_s(b, L"%04d-%02d-%02d_%02d-%02d-%02d", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    return b;
}

static std::wstring ErrText(DWORD e) {
    wchar_t* m = NULL;
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, e, 0, (LPWSTR)&m, 0, NULL);
    std::wstring s = m ? m : L"";
    if (m)
        LocalFree(m);
    while (!s.empty() && (s.back() == L'\n' || s.back() == L'\r' || s.back() == L' ' || s.back() == L'.'))
        s.pop_back();
    wchar_t b[24];
    swprintf_s(b, L" (%lu)", e);
    return s + b;
}

// --- Spielordner pruefen ---------------------------------------------------------------------------------------------------
// Nutzer: "wir sollten noch eine möglichkeit haben zu prüfen ob tatsächlich Othello insatlliert ist damit man nciht nicht Gothic 1
// installation bei othello durchführt und Gothic 1 bei Gothic 2 usw." / "einfahc nur ordnername reicht doch nicht das ist zu
// fehleranfällig" (u54). Der Ordnername zaehlt nie. Erkannt wird am Inhalt: die Fassung steht in der Versionsressource der
// Exe, das Spiel im Inhaltsverzeichnis der VDFs in Data (Gothic laedt jede davon; gelesen werden nur Kopf und Katalog, an den
// Spielordnern hier ~0,2 s):
//  - Gothic 2 DNdR: system\Gothic2.exe 2.6 (fuer diese Exe ist der G2A-Coop gebaut) und eine VDF mit
//    _WORK\DATA\WORLDS\ADDON\ADDONWORLD.ZEN (Worlds_Addon.vdf);
//  - Gothic 1: system\GOTHIC.EXE bzw. GothicMod.exe 1.0.8.11 ("1.08k_mod") und eine VDF mit _WORK\DATA\WORLDS\WORLD.ZEN;
//  - Othello (ueber Gothic 1): eine VDF mit SYSTEM\AUTORUN\OTHELLO_ABI.DLL und _WORK\DATA\SCRIPTS\_COMPILED\GOTHIC.DAT
//    (ScriptsOthello4.0_de.vdf - aus ihr laedt Union Othellos DLL) und eine mit Othellos Welt GOBLINCAVE.ZEN. Spuren von
//    Othello (Name *othello*.vdf, OTHELLO_ABI.DLL oder Othello-Welten im Katalog, Deinstaller) machen ein Gothic 1 unrein.
//    Die Fassung (Anzeige) aus dem Namen des Deinstallers Othello-<Fassung>-uninstall.exe.
struct GameInfo {
    bool exeG1 = false, exeG2 = false, unionOk = false;
    std::wstring exeName, exeVer;   // gefundene Spiel-Exe und ihre Fassung (Versionsressource, leer ohne)
    bool exeOk = false;             // Exe in der Fassung, fuer die der Coop gebaut ist (G2 2.6, G1 1.0.8.11)
    std::wstring g2AddonVdf, g1WorldVdf;   // VDF mit ADDONWORLD.ZEN bzw. WORLD.ZEN (leer: fehlt)
    std::vector<std::wstring> othelloFiles;   // alle Spuren von Othello
    bool othello = false;           // Othello vollstaendig: Skripte mit OTHELLO_ABI.DLL und Othello-Welt
    std::wstring othelloMissing, othelloVer, othelloKey;   // was fehlt / Fassung / woran erkannt
    bool ddraw = false, gd3d11 = false, gd3d11Match = false;   // ddraw.dll, GD3D11-Ordner, Fassung wie in der Nutzlast
    int coopState = 0;   // 0 kein Coop, 1 andere Fassung, 2 diese Fassung
};

static Game DetectGame(const GameInfo& i) {
    return i.exeG2 ? G_G2 : i.exeG1 ? (i.othello ? G_OTH : G_G1) : G_NONE;
}

// Fassung aus der Versionsressource (VS_FIXEDFILEINFO), z. B. "2.6.0.0"; leer ohne Ressource
static std::wstring ExeVersion(const std::wstring& f, unsigned v[4]) {
    v[0] = v[1] = v[2] = v[3] = 0;
    DWORD dummy = 0;
    const DWORD n = GetFileVersionInfoSizeW(f.c_str(), &dummy);
    if (!n)
        return std::wstring();
    std::vector<unsigned char> b(n);
    VS_FIXEDFILEINFO* fi = NULL;
    UINT len = 0;
    if (!GetFileVersionInfoW(f.c_str(), 0, n, b.data()) || !VerQueryValueW(b.data(), L"\\", (void**)&fi, &len) || !fi || len < sizeof(*fi))
        return std::wstring();
    v[0] = HIWORD(fi->dwFileVersionMS);
    v[1] = LOWORD(fi->dwFileVersionMS);
    v[2] = HIWORD(fi->dwFileVersionLS);
    v[3] = LOWORD(fi->dwFileVersionLS);
    wchar_t s[48];
    swprintf_s(s, L"%u.%u.%u.%u", v[0], v[1], v[2], v[3]);
    return s;
}

static void FindNames(const std::wstring& pattern, const std::wstring& prefix, std::vector<std::wstring>& out) {
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            out.push_back(prefix + fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
}

// Inhaltsverzeichnis einer VDF: Kopf (256 Byte Kommentar, "PSVDSC_V2.00", Anzahl, ..., Katalog-Abstand, Eintragsgroesse 80),
// je Eintrag 64 Byte Name (mit Leerzeichen aufgefuellt), Abstand (bei Ordnern: Index des ersten Kindes), Groesse, Art
// (0x80000000 Ordner, 0x40000000 letzter der Ebene). Liest nur Kopf und Katalog; out = volle Pfade in Grossbuchstaben.
static bool VdfPaths(const std::wstring& f, std::vector<std::string>& out) {
    HANDLE h = CreateFileW(f.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    unsigned char hd[296];
    DWORD got = 0;
    bool ok = ReadFile(h, hd, sizeof(hd), &got, NULL) && got == sizeof(hd) && memcmp(hd + 256, "PSVDSC_V2.00", 12) == 0;
    std::vector<unsigned char> cat;
    if (ok) {
        unsigned count = 0, off = 0, es = 0;
        memcpy(&count, hd + 272, 4);
        memcpy(&off, hd + 288, 4);
        memcpy(&es, hd + 292, 4);
        ok = es == 80 && count > 0 && count < 1000000;
        if (ok) {
            cat.resize((size_t)count * 80);
            LARGE_INTEGER p;
            p.QuadPart = off;
            ok = SetFilePointerEx(h, p, NULL, FILE_BEGIN) && ReadFile(h, cat.data(), (DWORD)cat.size(), &got, NULL) && got == cat.size();
        }
    }
    CloseHandle(h);
    if (!ok)
        return false;
    const unsigned n = (unsigned)(cat.size() / 80);
    struct Walker {
        const std::vector<unsigned char>& c;
        unsigned n;
        std::vector<std::string>& out;
        void Walk(unsigned i, const std::string& path, int depth) {
            if (depth > 32)
                return;
            for (; i < n && out.size() < 2000000; i++) {
                const unsigned char* e = &c[(size_t)i * 80];
                std::string name((const char*)e, 64);
                while (!name.empty() && (name.back() == ' ' || name.back() == '\0'))
                    name.pop_back();
                for (auto& ch : name)
                    ch = (char)toupper((unsigned char)ch);
                unsigned sub = 0, type = 0;
                memcpy(&sub, e + 64, 4);
                memcpy(&type, e + 72, 4);
                if (type & 0x80000000) {
                    if (sub > i)   // Kinder stehen immer dahinter - schuetzt vor Schleifen
                        Walk(sub, path + name + "\\", depth + 1);
                }
                else
                    out.push_back(path + name);
                if (type & 0x40000000)
                    return;
            }
        }
    } w{ cat, n, out };
    w.Walk(0, std::string(), 0);
    return true;
}

static GameInfo Inspect(const std::wstring& dir, Game g) {
    GameInfo i;
    if (dir.empty() || !DirExists(dir))
        return i;
    const std::wstring sys = dir + L"\\system\\";
    unsigned v[4];
    i.exeG2 = FileExists(sys + L"Gothic2.exe");
    if (i.exeG2) {
        i.exeName = L"Gothic2.exe";
        i.exeVer = ExeVersion(sys + L"Gothic2.exe", v);
        i.exeOk = v[0] == 2 && v[1] == 6;
    }
    else
        for (const wchar_t* e : { L"GOTHIC.EXE", L"GothicMod.exe" })
            if (FileExists(sys + e)) {
                const std::wstring ver = ExeVersion(sys + e, v);
                const bool ok = v[0] == 1 && v[1] == 0 && v[2] == 8 && v[3] == 11;
                if (!i.exeG1 || (ok && !i.exeOk)) {
                    i.exeName = e;
                    i.exeVer = ver;
                    i.exeOk = ok;
                }
                i.exeG1 = true;
            }
    // Inhalt der VDFs in Data (nur Kataloge)
    std::vector<std::wstring> vdfs;
    FindNames(dir + L"\\Data\\*.vdf", L"", vdfs);
    std::wstring othScripts, othWorld;
    for (auto& name : vdfs) {
        std::vector<std::string> paths;
        VdfPaths(dir + L"\\Data\\" + name, paths);   // keine gueltige VDF: leer
        auto has = [&](const char* p) { return std::find(paths.begin(), paths.end(), p) != paths.end(); };
        const bool abi = has("SYSTEM\\AUTORUN\\OTHELLO_ABI.DLL"), dat = has("_WORK\\DATA\\SCRIPTS\\_COMPILED\\GOTHIC.DAT");
        const bool othWorlds = has("_WORK\\DATA\\WORLDS\\GOBLINCAVE.ZEN") || has("_WORK\\DATA\\WORLDS\\OLDTEMPLE.ZEN");
        std::wstring low = name;
        for (auto& ch : low)
            ch = (wchar_t)towlower(ch);
        if (abi || othWorlds || low.find(L"othello") != std::wstring::npos)
            i.othelloFiles.push_back(L"Data\\" + name);
        if (abi && dat && othScripts.empty())
            othScripts = L"Data\\" + name;
        if (has("_WORK\\DATA\\WORLDS\\GOBLINCAVE.ZEN") && othWorld.empty())
            othWorld = L"Data\\" + name;
        if (has("_WORK\\DATA\\WORLDS\\ADDON\\ADDONWORLD.ZEN") && i.g2AddonVdf.empty())
            i.g2AddonVdf = L"Data\\" + name;
        if (has("_WORK\\DATA\\WORLDS\\WORLD.ZEN") && i.g1WorldVdf.empty())
            i.g1WorldVdf = L"Data\\" + name;
    }
    FindNames(sys + L"OTHELLO_ABI.DLL", L"system\\", i.othelloFiles);
    std::vector<std::wstring> un;
    FindNames(dir + L"\\Othello-*-uninstall.exe", L"", un);
    for (auto& u : un) {
        i.othelloFiles.push_back(u);
        if (i.othelloVer.empty() && u.size() > 22)
            i.othelloVer = u.substr(8, u.size() - 8 - 14);   // "Othello-4.4.2.1-uninstall.exe" -> 4.4.2.1
    }
    i.othello = i.exeG1 && !othScripts.empty() && !othWorld.empty();
    if (i.othello)
        i.othelloKey = (i.othelloVer.empty() ? std::wstring(L"Othello") : L"Othello " + i.othelloVer) + L": OTHELLO_ABI.DLL " +
            T(L"und Skripte in ", L"and scripts in ") + othScripts + T(L", Othello-Welt in ", L", Othello world in ") + othWorld;
    if (othScripts.empty())
        i.othelloMissing = T(L"Othello-Skripte mit OTHELLO_ABI.DLL (ScriptsOthello4.0_de.vdf)", L"Othello scripts with OTHELLO_ABI.DLL (ScriptsOthello4.0_de.vdf)");
    if (othWorld.empty())
        i.othelloMissing += std::wstring(i.othelloMissing.empty() ? L"" : L", ") + T(L"Othello-Welten (WorldsOthello4.0.vdf)", L"Othello worlds (WorldsOthello4.0.vdf)");
    i.unionOk = FileExists(sys + L"Shw32.dll") && FileExists(sys + L"Union.patch");
    i.ddraw = FileExists(sys + L"ddraw.dll");
    i.gd3d11 = DirExists(sys + L"GD3D11\\Bin");
    if (i.ddraw && i.gd3d11) {
        // v17.9.7, wenn die Lader-DLL und die Renderer ohne AVX2 (vom Coop nie ersetzt) Byte fuer Byte die der Nutzlast sind
        i.gd3d11Match = true;
        for (const wchar_t* f : { L"system\\ddraw.dll", L"system\\GD3D11\\Bin\\g1.dll", L"system\\GD3D11\\Bin\\g2a.dll" }) {
            const Entry* e = FindEntry(C_GD3D11, f);
            unsigned char sha[32];
            if (!e || !FileSha(dir + L"\\" + f, sha) || memcmp(sha, e->sha, 32) != 0)
                i.gd3d11Match = false;
        }
    }
    const Entry* coop = FindEntry(g == G_G2 ? C_COOP_G2 : g == G_OTH ? C_COOP_OTH : C_COOP_G1,
        g == G_G2 ? L"system\\Autorun\\Gothic2Coop.dll" : L"system\\Autorun\\Gothic1Coop.dll");
    if (coop && FileExists(dir + L"\\" + coop->path)) {
        unsigned char sha[32];
        i.coopState = FileSha(dir + L"\\" + coop->path, sha) && memcmp(sha, coop->sha, 32) == 0 ? 2 : 1;
    }
    return i;
}

static bool GothicRunning(std::wstring& which) {
    HANDLE s = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (s == INVALID_HANDLE_VALUE)
        return false;
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(pe);
    bool found = false;
    for (BOOL ok = Process32FirstW(s, &pe); ok && !found; ok = Process32NextW(s, &pe))
        for (const wchar_t* n : { L"Gothic2.exe", L"GothicMod.exe", L"GOTHIC.EXE", L"Gothic.exe" })
            if (_wcsicmp(pe.szExeFile, n) == 0) {
                which = pe.szExeFile;
                found = true;
            }
    CloseHandle(s);
    return found;
}

static const wchar_t* GameName(Game g) {
    switch (g) {
    case G_G1: return T(L"Gothic 1", L"Gothic 1");
    case G_G2: return T(L"Gothic 2 – Die Nacht des Raben", L"Gothic 2 – Night of the Raven");
    case G_OTH: return T(L"Gothic 1 mit Othello", L"Gothic 1 with Othello");
    default: return L"?";
    }
}

static const wchar_t* GameKey(Game g) {
    return g == G_G2 ? L"g2" : g == G_OTH ? L"othello" : L"g1";
}

static std::wstring JoinList(const std::vector<std::wstring>& v, size_t most = 4) {
    std::wstring s;
    for (size_t k = 0; k < v.size() && k < most; k++)
        s += (k ? L", " : L"") + v[k];
    if (v.size() > most)
        s += L", …";
    return s;
}

// Passt der Ordner zum gewaehlten Spiel? Sonst why = Grund - Oberflaeche: Installieren gesperrt, /still: Rueckgabe 1
static bool GameCheck(const GameInfo& i, Game g, std::wstring& why) {
    const std::wstring exe = L"system\\" + i.exeName + L" " + (i.exeVer.empty() ? T(L"ohne Fassungsangabe", L"without version") : i.exeVer);
    if (!i.exeG1 && !i.exeG2) {
        why = T(L"kein Gothic in diesem Ordner (system\\Gothic2.exe bzw. system\\GOTHIC.EXE / GothicMod.exe).",
            L"no Gothic in this folder (system\\Gothic2.exe or system\\GOTHIC.EXE / GothicMod.exe).");
        return false;
    }
    if (g == G_G2 && !i.exeG2) {
        why = std::wstring(T(L"FALSCHES SPIEL: Dieser Ordner ist ", L"WRONG GAME: this folder is ")) + GameName(DetectGame(i)) +
            T(L" – bitte oben umstellen.", L" – please switch above.");
        return false;
    }
    if (g != G_G2 && i.exeG2) {
        why = T(L"FALSCHES SPIEL: Dieser Ordner ist Gothic 2 – bitte oben umstellen.", L"WRONG GAME: this folder is Gothic 2 – please switch above.");
        return false;
    }
    if (g == G_G2 && (!i.exeOk || i.g2AddonVdf.empty())) {
        why = std::wstring(T(L"KEIN GOTHIC 2 DIE NACHT DES RABEN 2.6: ", L"NOT GOTHIC 2 NIGHT OF THE RAVEN 2.6: ")) + exe +
            (!i.g2AddonVdf.empty() ? L"" : T(L", in Data keine VDF mit der Addon-Welt (ADDON\\ADDONWORLD.ZEN, sonst in Worlds_Addon.vdf)",
                L", no VDF in Data with the addon world (ADDON\\ADDONWORLD.ZEN, normally in Worlds_Addon.vdf)")) +
            T(L". Der Coop braucht Gothic 2 mit dem Addon in Fassung 2.6 (fix), wie bei Steam und GOG.",
                L". The coop needs Gothic 2 with the addon in version 2.6 (fix), as on Steam and GOG.");
        return false;
    }
    if (g != G_G2 && !i.exeOk) {
        why = std::wstring(T(L"GOTHIC 1 IN ANDERER FASSUNG: ", L"GOTHIC 1 IN ANOTHER VERSION: ")) + exe +
            T(L". Der Coop braucht Gothic 1 in Fassung 1.08k (1.0.8.11), wie bei Steam und GOG.", L". The coop needs Gothic 1 version 1.08k (1.0.8.11), as on Steam and GOG.");
        return false;
    }
    if (g != G_G2 && i.g1WorldVdf.empty()) {
        why = std::wstring(T(L"KEIN VOLLSTÄNDIGES GOTHIC 1: ", L"NOT A COMPLETE GOTHIC 1: ")) + exe +
            T(L", aber in Data keine VDF mit der Welt von Gothic 1 (WORLDS\\WORLD.ZEN, sonst in worlds.VDF).",
                L", but no VDF in Data with the Gothic 1 world (WORLDS\\WORLD.ZEN, normally in worlds.VDF).");
        return false;
    }
    if (g == G_G1 && i.othello) {
        why = std::wstring(T(L"OTHELLO GEFUNDEN (", L"OTHELLO FOUND (")) + i.othelloKey +
            T(L") – bitte \"Gothic 1 mit Othello\" wählen (eigene Coop-Skripte für Othello).", L") – please choose \"Gothic 1 with Othello\" (own coop scripts for Othello).");
        return false;
    }
    // Mods fuer Gothic 1 sind erlaubt (Nutzer: "es soll ja auch erstmal möglich sein auch gothic 1 und 2 mit mods zu spielen") -
    // nur Othello hat eigene Coop-Skripte, Teile davon lassen offen, welche passen
    if (g == G_G1 && !i.othelloFiles.empty()) {
        why = std::wstring(T(L"TEILE VON OTHELLO GEFUNDEN (", L"PARTS OF OTHELLO FOUND (")) + JoinList(i.othelloFiles) +
            T(L"), aber kein vollständiges Othello (es fehlt ", L"), but no complete Othello (missing ") + i.othelloMissing +
            T(L"). Gothic lädt jede VDF in Data – erst Othello vollständig installieren oder diese Dateien entfernen.",
                L"). Gothic loads every VDF in Data – install Othello completely or remove these files first.");
        return false;
    }
    if (g == G_OTH && !i.othello) {
        why = i.othelloFiles.empty()
            ? std::wstring(T(L"KEIN OTHELLO: In diesem Ordner ist nur Gothic 1 (es fehlt ", L"NO OTHELLO: this folder is plain Gothic 1 (missing ")) + i.othelloMissing +
                T(L") – bitte \"Gothic 1\" wählen oder erst Othello installieren.", L") – please choose \"Gothic 1\" or install Othello first.")
            : std::wstring(T(L"OTHELLO UNVOLLSTÄNDIGE ODER ANDERE FASSUNG: gefunden ", L"OTHELLO INCOMPLETE OR ANOTHER VERSION: found ")) + JoinList(i.othelloFiles) +
                T(L", es fehlt ", L", missing ") + i.othelloMissing + T(L". Der Coop kennt Othello 4.x mit ScriptsOthello4.0.", L". The coop knows Othello 4.x with ScriptsOthello4.0.");
        return false;
    }
    return true;
}

// woran das Spiel erkannt wurde (Statuszeile)
static std::wstring GameEvidence(const GameInfo& i) {
    std::wstring s = L"system\\" + i.exeName + L" " + i.exeVer;
    if (i.exeG2)
        s += L", " + i.g2AddonVdf + T(L" mit ADDONWORLD.ZEN", L" with ADDONWORLD.ZEN");
    else if (i.othello)
        s += L", " + i.othelloKey;
    else
        s += L", " + i.g1WorldVdf + T(L" mit WORLD.ZEN", L" with WORLD.ZEN");
    return s;
}

// --- Suche nach Spielordnern ---------------------------------------------------------------------------------------------
struct Candidate {
    std::wstring dir;
    Game game;
};

static void AddCandidate(std::vector<Candidate>& v, std::wstring d) {
    while (!d.empty() && (d.back() == L'\\' || d.back() == L'/'))
        d.pop_back();
    std::replace(d.begin(), d.end(), L'/', L'\\');
    if (d.size() < 3)
        return;
    for (auto& c : v)
        if (_wcsicmp(c.dir.c_str(), d.c_str()) == 0)
            return;
    const Game g = DetectGame(Inspect(d, G_NONE));
    if (g != G_NONE)
        v.push_back({ d, g });
}

static std::wstring RegString(HKEY root, const wchar_t* key, const wchar_t* val, DWORD flags = 0) {
    wchar_t b[1024];
    DWORD n = sizeof(b);
    if (RegGetValueW(root, key, val, RRF_RT_REG_SZ | flags, NULL, b, &n) == ERROR_SUCCESS)
        return b;
    return std::wstring();
}

static std::vector<Candidate> FindGames() {
    std::vector<Candidate> v;
    // neben dem Installer (z. B. in den Spielordner gelegt) und zwei Ebenen darueber
    wchar_t self[MAX_PATH] = L"";
    GetModuleFileNameW(NULL, self, MAX_PATH);
    std::wstring d = Parent(self);
    for (int k = 0; k < 3 && !d.empty(); k++, d = Parent(d))
        AddCandidate(v, d);
    // Steam: Bibliotheken aus libraryfolders.vdf
    std::vector<std::wstring> libs;
    std::wstring steam = RegString(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", L"SteamPath");
    if (steam.empty())
        steam = RegString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Valve\\Steam", L"InstallPath");
    if (!steam.empty()) {
        std::replace(steam.begin(), steam.end(), L'/', L'\\');
        libs.push_back(steam);
        std::vector<unsigned char> vdf;
        if (ReadAll(steam + L"\\steamapps\\libraryfolders.vdf", vdf)) {
            const std::string t(vdf.begin(), vdf.end());
            for (size_t p = t.find("\"path\""); p != std::string::npos; p = t.find("\"path\"", p + 6)) {
                const size_t a = t.find('"', p + 6);
                const size_t b = a == std::string::npos ? a : t.find('"', a + 1);
                if (b == std::string::npos)
                    break;
                std::string s = t.substr(a + 1, b - a - 1);
                std::string u;
                for (size_t k = 0; k < s.size(); k++) {   // "D:\\SteamLibrary" -> D:\SteamLibrary
                    if (s[k] == '\\' && k + 1 < s.size() && s[k + 1] == '\\')
                        k++;
                    u += s[k];
                }
                libs.push_back(FromUtf8(u.c_str(), (int)u.size()));
            }
        }
    }
    for (auto& l : libs)
        for (const wchar_t* g : { L"Gothic II", L"Gothic 2", L"Gothic", L"Gothic 1" })
            AddCandidate(v, l + L"\\steamapps\\common\\" + g);
    // GOG
    HKEY gog;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\GOG.com\\Games", 0, KEY_READ, &gog) == ERROR_SUCCESS) {
        wchar_t sub[256];
        for (DWORD i = 0; ; i++) {
            DWORD n = 256;
            if (RegEnumKeyExW(gog, i, sub, &n, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
                break;
            const std::wstring k = std::wstring(L"SOFTWARE\\WOW6432Node\\GOG.com\\Games\\") + sub;
            const std::wstring p = RegString(HKEY_LOCAL_MACHINE, k.c_str(), L"path");
            if (!p.empty())
                AddCandidate(v, p);
        }
        RegCloseKey(gog);
    }
    // uebliche Orte
    wchar_t pf[MAX_PATH] = L"";
    if (SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILESX86, NULL, 0, pf) == S_OK)
        for (const wchar_t* g : { L"\\JoWooD\\Gothic II", L"\\JoWooD\\Gothic 2 Gold", L"\\Piranha Bytes\\Gothic", L"\\JoWooD\\Gothic",
                L"\\Gothic II", L"\\Gothic" })
            AddCandidate(v, std::wstring(pf) + g);
    for (const wchar_t* r : { L"C:\\Spiele", L"C:\\Games", L"D:\\Spiele", L"D:\\Games", L"C:" })
        for (const wchar_t* g : { L"\\Gothic II", L"\\Gothic 2", L"\\Gothic", L"\\Othello" })
            AddCandidate(v, std::wstring(r) + g);
    return v;
}

// --- Plan: welche Dateien ------------------------------------------------------------------------------------------------
enum RendererMode { R_NONE = 0, R_COOP_ONLY = 1, R_BASE_AND_COOP = 2 };

static RendererMode PlanRenderer(const GameInfo& i, bool wanted, bool replaceForeignDdraw, std::wstring& why) {
    if (!wanted) {
        why = T(L"Coop-Renderer abgewählt", L"coop renderer not selected");
        return R_NONE;
    }
    if (i.ddraw && !i.gd3d11 && !replaceForeignDdraw) {
        why = T(L"system\\ddraw.dll gehört einem anderen Programm (kein GD3D11-Ordner)", L"system\\ddraw.dll belongs to another program (no GD3D11 folder)");
        return R_NONE;
    }
    if (i.ddraw && i.gd3d11 && i.gd3d11Match) {
        why = T(L"GD3D11 ", L"GD3D11 ") + Gd3d11Ver + T(L" gefunden – nur der Coop-Renderer wird darübergelegt",
            L" found – only the coop renderer is laid over it");
        return R_COOP_ONLY;
    }
    if (i.ddraw && i.gd3d11)
        why = T(L"GD3D11 in einer anderen Fassung gefunden – wird auf ", L"GD3D11 found in another version – updated to ") + Gd3d11Ver +
        T(L" gebracht (der Coop-Renderer passt nur zu dessen Shadern), dann der Coop-Renderer darüber",
            L" (the coop renderer only fits its shaders), then the coop renderer over it");
    else
        why = T(L"GD3D11 fehlt – GD3D11 ", L"GD3D11 missing – GD3D11 ") + Gd3d11Ver + T(L" wird installiert, dann der Coop-Renderer darüber",
            L" is installed, then the coop renderer over it");
    return R_BASE_AND_COOP;
}

static std::vector<const Entry*> Plan(Game g, RendererMode r, bool withUnion) {
    std::vector<const Entry*> v;
    const int coop = g == G_G2 ? C_COOP_G2 : g == G_OTH ? C_COOP_OTH : C_COOP_G1;
    const int rnd = g == G_G2 ? C_RND_G2 : C_RND_G1;
    auto add = [&](int comp) {
        for (auto& e : Entries)
            if (e.comp == comp) {
                for (auto it = v.begin(); it != v.end(); ++it)
                    if (_wcsicmp((*it)->path.c_str(), e.path.c_str()) == 0) {   // spaeterer Teil gewinnt (Coop-Renderer ueber GD3D11)
                        v.erase(it);
                        break;
                    }
                v.push_back(&e);
            }
    };
    if (withUnion)   // u62: Union zuerst (der Coop braucht es)
        add(C_UNION);
    if (r == R_BASE_AND_COOP)
        add(C_GD3D11);
    if (r != R_NONE)
        add(rnd);
    add(coop);
    add(C_DOCS);
    return v;
}

// --- Installieren / rueckgaengig ------------------------------------------------------------------------------------------
struct Ctx {
    std::wstring dir;
    Game game = G_NONE;
    bool renderer = true, replaceForeignDdraw = false;
    bool unionFiles = false, unionDeclined = false;   // u62: Union-Dateien mitinstallieren, falls Union fehlt / Nutzer will ohne Union
    HWND ui = NULL;
    std::vector<std::wstring> log;
};

static void Log(Ctx& c, const std::wstring& s) {
    c.log.push_back(s);
    if (c.ui) {
        HWND e = GetDlgItem(c.ui, IDC_STATUS);
        const int n = GetWindowTextLengthW(e);
        SendMessageW(e, EM_SETSEL, n, n);
        SendMessageW(e, EM_REPLACESEL, FALSE, (LPARAM)(s + L"\r\n").c_str());
        MSG m;   // Fenster neu zeichnen lassen (lange Laeufe)
        while (PeekMessageW(&m, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
            DispatchMessageW(&m);
    }
}

static void SaveLog(Ctx& c, const std::wstring& stamp, const wchar_t* what) {
    const std::wstring d = c.dir + L"\\CoopLogs\\Installer";
    if (MakeDirs(d))
        return;
    std::string t = "\xEF\xBB\xBF";
    t += ToUtf8(std::wstring(L"GothicCoop-Installer ") + Fassung + L" – " + what + L" – " + stamp + L"\r\n");
    for (auto& l : c.log)
        t += ToUtf8(l + L"\r\n");
    std::wstring f = d + L"\\" + stamp + L".log";
    for (int k = 2; FileExists(f) && k < 100; k++)
        f = d + L"\\" + stamp + L"_" + std::to_wstring(k) + L".log";
    HANDLE h = CreateFileW(f.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD put = 0;
        WriteFile(h, t.data(), (DWORD)t.size(), &put, NULL);
        CloseHandle(h);
    }
}

struct Done {
    int kind;   // 0 neu, 1 ersetzt, 2 entfernt (u62: system\dinput.dll beim Mitinstallieren von Union; sha = alte Datei)
    std::wstring rel;
    unsigned char sha[32];
};

static bool WriteManifest(const std::wstring& runDir, const Ctx& c, const std::vector<Done>& done, bool complete) {
    std::string t = ToUtf8(std::wstring(L"GothicCoop-Installer v=1\r\nfassung=") + Fassung + L"\r\nspiel=" + GameKey(c.game) +
        L"\r\nordner=" + c.dir + L"\r\n");
    for (auto& d : done)
        t += ToUtf8(std::wstring(L"datei=") + (d.kind == 0 ? L"neu|" : d.kind == 1 ? L"ersetzt|" : L"entfernt|") + Hex(d.sha, 32) + L"|" +
            d.rel + L"\r\n");
    t += complete ? "ende=1\r\n" : "abgebrochen=1\r\n";
    return WriteReplace(runDir + L"\\COOPINSTALL.TXT", t.data(), t.size()) == 0;
}

// Eine Installation (Ordner in CoopBackup) zurueckdrehen: Neues entfernen, Ersetztes zurueck - nur, wenn die Datei noch die
// installierte ist (sonst hat sie jemand inzwischen geaendert: bleibt, mit Hinweis)
static int UndoRun(Ctx& c, const std::wstring& runDir) {
    std::vector<unsigned char> raw;
    if (!ReadAll(runDir + L"\\COOPINSTALL.TXT", raw)) {
        Log(c, T(L"FEHLER: COOPINSTALL.TXT nicht lesbar", L"ERROR: COOPINSTALL.TXT not readable"));
        return 1;
    }
    const std::wstring t = FromUtf8((const char*)raw.data(), (int)raw.size());
    std::vector<std::wstring> lines;
    for (size_t a = 0; a < t.size();) {
        size_t e = t.find(L'\n', a);
        if (e == std::wstring::npos)
            e = t.size();
        std::wstring l = t.substr(a, e - a);
        if (!l.empty() && l.back() == L'\r')
            l.pop_back();
        lines.push_back(l);
        a = e + 1;
    }
    int removed = 0, restored = 0, kept = 0, failed = 0;
    for (auto it = lines.rbegin(); it != lines.rend(); ++it) {   // rueckwaerts: zuletzt Geschriebenes zuerst
        const std::wstring& l = *it;
        if (l.compare(0, 6, L"datei=") != 0)
            continue;
        const size_t p1 = l.find(L'|', 6), p2 = p1 == std::wstring::npos ? p1 : l.find(L'|', p1 + 1);
        if (p2 == std::wstring::npos)
            continue;
        const std::wstring kind = l.substr(6, p1 - 6), sha = l.substr(p1 + 1, p2 - p1 - 1), rel = l.substr(p2 + 1);
        if (rel.empty() || rel.find(L"..") != std::wstring::npos)
            continue;
        const std::wstring target = c.dir + L"\\" + rel;
        unsigned char now[32];
        const bool exists = FileExists(target);
        if (kind == L"entfernt") {   // u62: zurueckholen - ausser, dort liegt inzwischen wieder etwas
            if (exists) {
                Log(c, std::wstring(T(L"  bleibt (seit der Installation neu angelegt): ", L"  kept (created again since the installation): ")) + rel);
                kept++;
                continue;
            }
            std::vector<unsigned char> old;
            if (!ReadAll(runDir + L"\\" + rel + L".sicherung", old)) {
                Log(c, std::wstring(T(L"  FEHLER: Sicherung fehlt: ", L"  ERROR: backup missing: ")) + rel);
                failed++;
                continue;
            }
            const DWORD e = WriteReplace(target, old.data(), old.size());
            if (e) {
                Log(c, std::wstring(T(L"  FEHLER beim Zurücklegen: ", L"  ERROR restoring: ")) + rel + L" – " + ErrText(e));
                failed++;
                continue;
            }
            restored++;
            continue;
        }
        const bool same = exists && FileSha(target, now) && Hex(now, 32) == sha;
        if (exists && !same) {
            Log(c, std::wstring(T(L"  bleibt (seit der Installation geändert): ", L"  kept (changed since the installation): ")) + rel);
            kept++;
            continue;
        }
        if (kind == L"neu") {
            if (exists && !DeleteFileW(target.c_str())) {
                Log(c, std::wstring(T(L"  FEHLER beim Entfernen: ", L"  ERROR removing: ")) + rel + L" – " + ErrText(GetLastError()));
                failed++;
                continue;
            }
            removed++;
            for (std::wstring d = Parent(target); d.size() > c.dir.size(); d = Parent(d))   // leere Ordner, die dazukamen
                if (!RemoveDirectoryW(d.c_str()))
                    break;
        }
        else if (kind == L"ersetzt") {
            std::vector<unsigned char> old;
            if (!ReadAll(runDir + L"\\" + rel + L".sicherung", old)) {
                Log(c, std::wstring(T(L"  FEHLER: Sicherung fehlt: ", L"  ERROR: backup missing: ")) + rel);
                failed++;
                continue;
            }
            const DWORD e = WriteReplace(target, old.data(), old.size());
            if (e) {
                Log(c, std::wstring(T(L"  FEHLER beim Zurücklegen: ", L"  ERROR restoring: ")) + rel + L" – " + ErrText(e));
                failed++;
                continue;
            }
            restored++;
        }
    }
    std::string mark = ToUtf8(std::wstring(L"rueckgaengig=") + Stamp() + L"\r\n");
    HANDLE h = CreateFileW((runDir + L"\\COOPINSTALL.TXT").c_str(), FILE_APPEND_DATA, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD put = 0;
        WriteFile(h, mark.data(), (DWORD)mark.size(), &put, NULL);
        CloseHandle(h);
    }
    Log(c, std::wstring(T(L"Rückgängig: ", L"Undo: ")) + std::to_wstring(removed) + T(L" entfernt, ", L" removed, ") +
        std::to_wstring(restored) + T(L" zurückgelegt, ", L" restored, ") + std::to_wstring(kept) + T(L" geändert gelassen, ", L" left changed, ") +
        std::to_wstring(failed) + T(L" Fehler", L" errors"));
    return failed ? 1 : 0;
}

// juengster Lauf in <Spiel>\CoopBackup, der fertig ist und noch nicht zurueckgedreht wurde
static std::wstring LastRun(const std::wstring& dir) {
    std::vector<std::wstring> runs;
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW((dir + L"\\CoopBackup\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return std::wstring();
    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && fd.cFileName[0] != L'.')
            runs.push_back(fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    std::sort(runs.rbegin(), runs.rend());
    for (auto& r : runs) {
        std::vector<unsigned char> raw;
        if (!ReadAll(dir + L"\\CoopBackup\\" + r + L"\\COOPINSTALL.TXT", raw))
            continue;
        const std::string t(raw.begin(), raw.end());
        if (t.find("ende=1") != std::string::npos && t.find("rueckgaengig=") == std::string::npos)
            return dir + L"\\CoopBackup\\" + r;
    }
    return std::wstring();
}

// Bedienelemente waehrend der Installation sperren/freigeben
static void UiLock(HWND h, bool lock) {
    for (int id : { IDC_RB_G1, IDC_RB_G2, IDC_RB_OTH, IDC_DIR, IDC_BROWSE, IDC_RENDERER, IDC_INSTALL, IDC_UNDO, (int)IDCANCEL })
        EnableWindow(GetDlgItem(h, id), !lock);
}

static int DoInstall(Ctx& c) {
    const std::wstring stamp = Stamp();
    c.log.clear();
    Log(c, std::wstring(T(L"Installation GothicCoop ", L"Installing GothicCoop ")) + Fassung + L" – " + GameName(c.game) + L" – " + c.dir);
    GameInfo i = Inspect(c.dir, c.game);
    std::wstring gwhy;
    if (!GameCheck(i, c.game, gwhy)) {
        Log(c, std::wstring(T(L"FEHLER – nichts installiert: ", L"ERROR – nothing installed: ")) + gwhy);
        if (i.exeG1 || i.exeG2)   // ein Gothic-Ordner: Grund ins Protokoll (bei /still sonst nirgends zu sehen)
            SaveLog(c, stamp, L"falsches Spiel");
        return 1;
    }
    Log(c, std::wstring(T(L"Spiel: ", L"Game: ")) + GameName(DetectGame(i)) + L" (" + GameEvidence(i) + L")");
    std::wstring which;
    if (GothicRunning(which)) {
        Log(c, std::wstring(T(L"FEHLER: Gothic läuft noch (", L"ERROR: Gothic is still running (")) + which + T(L") – bitte beenden.", L") – please quit it."));
        return 1;
    }
    // u62: fehlt Union bei Gothic 1/2 und soll es mit, kommen seine Dateien in den Plan (vorhandenes Union bleibt unberuehrt)
    const bool withUnion = !i.unionOk && c.game != G_OTH && c.unionFiles && UnionFiles() > 0;
    if (withUnion)
        Log(c, T(L"Union: fehlt – Union 1.0m wird mitinstalliert (Rückgängig nimmt es wieder heraus).",
            L"Union: missing – Union 1.0m is installed along (undo removes it again)."));
    std::wstring rwhy;
    const RendererMode rm = PlanRenderer(i, c.renderer, c.replaceForeignDdraw, rwhy);
    Log(c, std::wstring(T(L"Renderer: ", L"Renderer: ")) + rwhy);
    const std::vector<const Entry*> plan = Plan(c.game, rm, withUnion);
    // erst alles entpacken und pruefen - kaputte Nutzlast aendert nichts am Spielordner
    std::vector<std::vector<unsigned char>> data(plan.size());
    for (size_t k = 0; k < plan.size(); k++)
        if (!Unpack(*plan[k], data[k])) {
            Log(c, std::wstring(T(L"FEHLER: Nutzlast beschädigt bei ", L"ERROR: payload damaged at ")) + plan[k]->path);
            return 1;
        }
    const std::wstring runDir = c.dir + L"\\CoopBackup\\" + stamp;
    DWORD err = MakeDirs(runDir);
    if (err) {
        Log(c, std::wstring(T(L"FEHLER: CoopBackup nicht anlegbar – ", L"ERROR: cannot create CoopBackup – ")) + ErrText(err));
        return err == ERROR_ACCESS_DENIED ? 3 : 1;
    }
    std::vector<Done> done;
    int fresh = 0, replaced = 0, same = 0, keptOwn = 0;
    bool failed = false;
    // u62: mit Union - system\dinput.dll (SystemPack-Lader) sichern und entfernen wie der Union-Installer
    if (withUnion && FileExists(c.dir + L"\\" + UNION_REMOVE)) {
        const std::wstring target = c.dir + L"\\" + UNION_REMOVE;
        const std::wstring bak = runDir + L"\\" + UNION_REMOVE + L".sicherung";
        Done d;
        d.kind = 2;
        d.rel = UNION_REMOVE;
        err = MakeDirs(Parent(bak));
        if (err || !FileSha(target, d.sha) || !CopyFileW(target.c_str(), bak.c_str(), FALSE)) {
            err = err ? err : GetLastError();
            Log(c, std::wstring(T(L"FEHLER beim Sichern von ", L"ERROR backing up ")) + UNION_REMOVE + L" – " + ErrText(err));
            failed = true;
        }
        else if (!DeleteFileW(target.c_str())) {
            err = GetLastError();
            Log(c, std::wstring(T(L"FEHLER beim Entfernen von ", L"ERROR removing ")) + UNION_REMOVE + L" – " + ErrText(err));
            failed = true;
        }
        else {
            done.push_back(d);
            Log(c, std::wstring(T(L"  entfernt (Union lädt SystemPack selbst): ", L"  removed (Union loads SystemPack itself): ")) + UNION_REMOVE);
        }
    }
    for (size_t k = 0; k < plan.size() && !failed; k++) {
        const Entry& e = *plan[k];
        const std::wstring target = c.dir + L"\\" + e.path;
        const bool exists = FileExists(target);
        if (exists && (e.flags & F_KEEP)) {
            keptOwn++;
            Log(c, std::wstring(T(L"  behalten (eigene Einstellungen): ", L"  kept (own settings): ")) + e.path);
            continue;
        }
        unsigned char now[32];
        if (exists && FileSha(target, now) && memcmp(now, e.sha, 32) == 0) {
            same++;
            continue;
        }
        if (exists) {
            const std::wstring bak = runDir + L"\\" + e.path + L".sicherung";
            err = MakeDirs(Parent(bak));
            if (err || !CopyFileW(target.c_str(), bak.c_str(), FALSE)) {
                err = err ? err : GetLastError();
                Log(c, std::wstring(T(L"FEHLER beim Sichern von ", L"ERROR backing up ")) + e.path + L" – " + ErrText(err));
                failed = true;
                break;
            }
        }
        err = WriteReplace(target, data[k].data(), data[k].size());
        if (err) {
            Log(c, std::wstring(T(L"FEHLER beim Schreiben von ", L"ERROR writing ")) + e.path + L" – " + ErrText(err));
            failed = true;
            break;
        }
        Done d;
        d.kind = exists ? 1 : 0;
        d.rel = e.path;
        memcpy(d.sha, e.sha, 32);
        done.push_back(d);
        if (exists)
            replaced++;
        else
            fresh++;
        if (e.comp != C_GD3D11 || exists)   // die ~1700 neuen GD3D11-Dateien nur zaehlen
            Log(c, std::wstring(exists ? T(L"  ersetzt: ", L"  replaced: ") : T(L"  neu: ", L"  new: ")) + e.path);
    }
    if (failed) {
        WriteManifest(runDir, c, done, false);
        Log(c, T(L"Abgebrochen – schon Geschriebenes wird zurückgedreht:", L"Aborted – undoing what was written:"));
        UndoRun(c, runDir);
        SaveLog(c, stamp, L"abgebrochen");
        return err == ERROR_ACCESS_DENIED ? 3 : 1;
    }
    if (done.empty())
        RemoveDirectoryW(runDir.c_str());   // nichts geaendert - kein leerer Lauf in CoopBackup
    else if (!WriteManifest(runDir, c, done, true))
        Log(c, T(L"WARNUNG: COOPINSTALL.TXT nicht schreibbar – Rückgängig geht für diesen Lauf nicht.",
            L"WARNING: COOPINSTALL.TXT not writable – undo is not possible for this run."));
    Log(c, std::wstring(T(L"Fertig: ", L"Done: ")) + std::to_wstring(fresh) + T(L" neu, ", L" new, ") + std::to_wstring(replaced) +
        T(L" ersetzt (Sicherung in CoopBackup\\", L" replaced (backup in CoopBackup\\") + stamp + L"), " + std::to_wstring(same) +
        T(L" schon aktuell, ", L" already up to date, ") + std::to_wstring(keptOwn) + T(L" eigene behalten.", L" own kept."));
    if (!i.unionOk && c.game == G_OTH)
        Log(c, T(L"HINWEIS: Union fehlt (system\\Shw32.dll) – bei Othello gehört es dazu; ohne Union lädt Gothic den Coop nicht. Othello-Installation prüfen.",
            L"NOTE: Union is missing (system\\Shw32.dll) – it is part of Othello; without Union Gothic does not load the coop. Check the Othello installation."));
    else if (!i.unionOk && !withUnion)
        Log(c, T(L"HINWEIS: Union fehlt (system\\Shw32.dll) – ohne Union lädt Gothic den Coop nicht. Den Installer noch einmal starten und Union mitinstallieren lassen.",
            L"NOTE: Union is missing (system\\Shw32.dll) – without Union Gothic does not load the coop. Run the installer again and let it install Union along."));
    SaveLog(c, stamp, L"Installation");
    return 0;
}

static int DoUndo(Ctx& c) {
    c.log.clear();
    std::wstring which;
    if (GothicRunning(which)) {
        Log(c, std::wstring(T(L"FEHLER: Gothic läuft noch (", L"ERROR: Gothic is still running (")) + which + L").");
        return 1;
    }
    const std::wstring run = LastRun(c.dir);
    if (run.empty()) {
        Log(c, T(L"Keine Installation zum Rückgängigmachen gefunden (CoopBackup).", L"No installation to undo found (CoopBackup)."));
        return 1;
    }
    Log(c, std::wstring(T(L"Rückgängig: ", L"Undo: ")) + run);
    const int r = UndoRun(c, run);
    SaveLog(c, Stamp(), L"rueckgaengig");
    return r;
}

// --- Oberflaeche ---------------------------------------------------------------------------------------------------------
static std::vector<Candidate> Found;
static Ctx UiCtx;
static bool AutoStart = false;

static Game UiGame(HWND h) {
    return IsDlgButtonChecked(h, IDC_RB_G2) ? G_G2 : IsDlgButtonChecked(h, IDC_RB_OTH) ? G_OTH : G_G1;
}

static void UiSetGame(HWND h, Game g) {
    CheckRadioButton(h, IDC_RB_G1, IDC_RB_OTH, g == G_G2 ? IDC_RB_G2 : g == G_OTH ? IDC_RB_OTH : IDC_RB_G1);
}

static std::wstring UiDir(HWND h) {
    wchar_t b[MAX_PATH * 2] = L"";
    GetDlgItemTextW(h, IDC_DIR, b, MAX_PATH * 2);
    std::wstring d = b;
    while (!d.empty() && (d.back() == L'\\' || d.back() == L' '))
        d.pop_back();
    return d;
}

static void UiRefresh(HWND h) {
    const std::wstring dir = UiDir(h);
    const Game g = UiGame(h);
    const GameInfo i = Inspect(dir, g);
    std::wstring s;
    bool ok = true;
    auto line = [&](const std::wstring& l) { s += l + L"\r\n"; };
    line(std::wstring(T(L"GothicCoop ", L"GothicCoop ")) + Fassung + T(L" – Coop für ", L" – coop for ") + GameName(g));
    if (dir.empty() || !DirExists(dir)) {
        line(T(L"Spielordner wählen (der Ordner, in dem \"system\" liegt).", L"Choose the game folder (the one containing \"system\")."));
        ok = false;
    }
    else {
        std::wstring gwhy;
        if (!GameCheck(i, g, gwhy)) {   // falsches Spiel: nur der Grund (Union/Renderer-Zeilen passten hier nicht)
            line(gwhy);
            ok = false;
            if (!LastRun(dir).empty())
                line(T(L"Eine frühere Installation kann rückgängig gemacht werden (CoopBackup).", L"An earlier installation can be undone (CoopBackup)."));
            SetDlgItemTextW(h, IDC_STATUS, s.c_str());
            EnableWindow(GetDlgItem(h, IDC_INSTALL), FALSE);
            EnableWindow(GetDlgItem(h, IDC_UNDO), !LastRun(dir).empty());
            return;
        }
        line(std::wstring(T(L"Spiel: ", L"Game: ")) + GameName(g) + T(L" gefunden (", L" found (") + GameEvidence(i) + L").");
        if (g == G_OTH && !i.othelloVer.empty() && i.othelloVer != L"4.4.2.1")
            line(std::wstring(T(L"HINWEIS: Othello ", L"NOTE: Othello ")) + i.othelloVer +
                T(L" – der Coop ist mit Othello 4.4.2.1 erprobt.", L" – the coop was tested with Othello 4.4.2.1."));
        if (i.unionOk)
            line(T(L"Union: gefunden.", L"Union: found."));
        else if (g == G_OTH)
            line(T(L"Union: FEHLT – bei Othello gehört Union dazu (Othello-Installation prüfen); ohne Union lädt Gothic den Coop nicht.",
                L"Union: MISSING – Union is part of Othello (check the Othello installation); without Union Gothic does not load the coop."));
        else if (UnionFiles())
            line(T(L"Union: FEHLT – beim Installieren wird Union 1.0m mitinstalliert (Rückfrage; system\\dinput.dll kommt nach CoopBackup).",
                L"Union: MISSING – Union 1.0m is installed along (you will be asked; system\\dinput.dll goes to CoopBackup)."));
        else
            line(T(L"Union: FEHLT – der Coop braucht Union 1.0m mit zParserExtender (selbst installieren).",
                L"Union: MISSING – the coop needs Union 1.0m with zParserExtender (install it yourself)."));
        line(i.coopState == 2 ? std::wstring(T(L"Coop: ", L"Coop: ")) + Fassung + T(L" ist schon installiert.", L" is already installed.")
            : i.coopState == 1 ? std::wstring(T(L"Coop: andere Fassung vorhanden – wird durch ", L"Coop: another version present – replaced by ")) + Fassung + T(L" ersetzt.", L".")
            : std::wstring(T(L"Coop: noch nicht installiert.", L"Coop: not installed yet.")));
        std::wstring rwhy;
        PlanRenderer(i, IsDlgButtonChecked(h, IDC_RENDERER) == BST_CHECKED, false, rwhy);
        if (i.ddraw && !i.gd3d11 && IsDlgButtonChecked(h, IDC_RENDERER) == BST_CHECKED)
            rwhy += T(L" – beim Installieren wird gefragt", L" – you will be asked when installing");
        line(std::wstring(T(L"Renderer: ", L"Renderer: ")) + rwhy + L".");
        if (IsDlgButtonChecked(h, IDC_RENDERER) == BST_CHECKED && !IsProcessorFeaturePresent(PF_AVX2_INSTRUCTIONS_AVAILABLE))
            line(T(L"HINWEIS: Dieser Prozessor kann kein AVX2 – GD3D11 nimmt dann einen Renderer ohne Coop-Teile (kein Zuschau-Fenster, keine 3D-Vorschau).",
                L"NOTE: this CPU has no AVX2 – GD3D11 then uses a renderer without coop parts (no spectator window, no 3D preview)."));
        std::wstring which;
        if (GothicRunning(which)) {
            line(std::wstring(T(L"Gothic läuft (", L"Gothic is running (")) + which + T(L") – vor dem Installieren beenden.", L") – quit it before installing."));
            ok = false;
        }
        if (!LastRun(dir).empty())
            line(T(L"Eine frühere Installation kann rückgängig gemacht werden (CoopBackup).", L"An earlier installation can be undone (CoopBackup)."));
        if (ok)
            line(T(L"Bereit. Ersetzte Dateien kommen vorher nach CoopBackup\\<Zeit>.", L"Ready. Replaced files are backed up to CoopBackup\\<time> first."));
    }
    SetDlgItemTextW(h, IDC_STATUS, s.c_str());
    EnableWindow(GetDlgItem(h, IDC_INSTALL), ok);
    EnableWindow(GetDlgItem(h, IDC_UNDO), DirExists(dir) && !LastRun(dir).empty());
}

static void UiDirChosen(HWND h, const std::wstring& dir) {
    const Game here = DetectGame(Inspect(dir, G_NONE));
    if (here != G_NONE)
        UiSetGame(h, here);
    UiRefresh(h);
}

static void UiBrowse(HWND h) {
    IFileOpenDialog* d = NULL;
    if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&d))))
        return;
    DWORD o = 0;
    d->GetOptions(&o);
    d->SetOptions(o | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
    d->SetTitle(T(L"Gothic-Spielordner wählen (der Ordner, in dem \"system\" liegt)", L"Choose the Gothic game folder (the one containing \"system\")"));
    if (SUCCEEDED(d->Show(h))) {
        IShellItem* it = NULL;
        if (SUCCEEDED(d->GetResult(&it))) {
            wchar_t* p = NULL;
            if (SUCCEEDED(it->GetDisplayName(SIGDN_FILESYSPATH, &p))) {
                std::wstring dir = p;
                CoTaskMemFree(p);
                // "system" selbst gewaehlt: eine Ebene hoeher
                if (dir.size() > 7 && _wcsicmp(dir.c_str() + dir.size() - 7, L"\\system") == 0)
                    dir.resize(dir.size() - 7);
                SetDlgItemTextW(h, IDC_DIR, dir.c_str());
                UiDirChosen(h, dir);
            }
            it->Release();
        }
    }
    d->Release();
}

static std::wstring SelfArgs(HWND h) {
    std::wstring a = std::wstring(L"/spiel:") + GameKey(UiGame(h)) + L" /ordner:\"" + UiDir(h) + L"\"";
    if (IsDlgButtonChecked(h, IDC_RENDERER) != BST_CHECKED)
        a += L" /ohnerenderer";
    if (UiCtx.unionDeclined)
        a += L" /ohneunion";
    return a + L" /los";
}

static bool UiBusy = false;    // Installation laeuft (Fenster nicht schliessen)
static bool NoUnionAsk = false;   // /ohneunion

static void UiInstall(HWND h) {
    Ctx& c = UiCtx;
    c.dir = UiDir(h);
    c.game = UiGame(h);
    c.renderer = IsDlgButtonChecked(h, IDC_RENDERER) == BST_CHECKED;
    c.replaceForeignDdraw = false;
    c.ui = h;
    const GameInfo i = Inspect(c.dir, c.game);
    if (c.renderer && i.ddraw && !i.gd3d11) {
        const int r = MessageBoxW(h, T(L"Im Spielordner liegt eine system\\ddraw.dll, aber kein GD3D11 – sie gehört einem anderen Programm (z. B. einem anderen Grafik-Aufsatz).\n\nDurch GD3D11 ersetzen? Die alte Datei kommt nach CoopBackup.\n\nNein = ohne Renderer installieren.",
            L"The game folder contains system\\ddraw.dll but no GD3D11 – it belongs to another program (e.g. another graphics wrapper).\n\nReplace it with GD3D11? The old file goes to CoopBackup.\n\nNo = install without renderer."),
            L"GothicCoop", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (r == IDCANCEL)
            return;
        c.replaceForeignDdraw = r == IDYES;
        c.renderer = r == IDYES;
    }
    c.unionFiles = false;
    c.unionDeclined = NoUnionAsk;
    if (!i.unionOk && c.game != G_OTH && UnionFiles() && !NoUnionAsk) {
        const int r = MessageBoxW(h, (std::wstring(T(L"Union fehlt – ohne Union lädt Gothic den Coop nicht.\n\nUnion 1.0m mitinstallieren (",
            L"Union is missing – without Union Gothic does not load the coop.\n\nInstall Union 1.0m along (")) + std::to_wstring(UnionFiles()) +
            T(L" Dateien; system\\dinput.dll von SystemPack kommt nach CoopBackup)? „Letzte Installation rückgängig“ nimmt es wieder heraus.\n\nNein = Coop ohne Union installieren.",
                L" files; SystemPack's system\\dinput.dll goes to CoopBackup)? \"Undo last installation\" removes it again.\n\nNo = install the coop without Union.")).c_str(),
            L"GothicCoop", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (r == IDCANCEL)
            return;
        c.unionFiles = r == IDYES;
        c.unionDeclined = r == IDNO;
    }
    SetDlgItemTextW(h, IDC_STATUS, L"");
    UiLock(h, true);
    HCURSOR old = SetCursor(LoadCursor(NULL, IDC_WAIT));
    UiBusy = true;
    const int r = DoInstall(c);
    UiBusy = false;
    SetCursor(old);
    UiLock(h, false);
    if (r == 0) {
        // Nutzer (u54): "nach der Union Installtion auch direkt das Fenster erscbeinen dass Coop fertig installiert ist und nach
        // dem click auf OK beendet sich das fenster" - so nach jeder gelungenen Installation
        std::wstring msg = std::wstring(L"GothicCoop ") + Fassung + T(L" ist fertig installiert – ", L" is installed – ") + GameName(c.game) + L"\n" + c.dir;
        if (!Inspect(c.dir, c.game).unionOk)
            msg += T(L"\n\nUnion fehlt noch – ohne Union lädt Gothic den Coop nicht.", L"\n\nUnion is still missing – without Union Gothic does not load the coop.");
        msg += T(L"\n\nProtokoll: CoopLogs\\Installer. OK beendet den Installer.", L"\n\nLog: CoopLogs\\Installer. OK closes the installer.");
        MessageBoxW(h, msg.c_str(), L"GothicCoop", MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
        EndDialog(h, 0);
        return;
    }
    if (r == 3) {
        if (MessageBoxW(h, T(L"Keine Schreibrechte im Spielordner (z. B. unter \"Programme\").\n\nInstaller als Administrator neu starten?",
            L"No write access to the game folder (e.g. under \"Program Files\").\n\nRestart the installer as administrator?"),
            L"GothicCoop", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            wchar_t self[MAX_PATH] = L"";
            GetModuleFileNameW(NULL, self, MAX_PATH);
            const std::wstring args = SelfArgs(h);
            SHELLEXECUTEINFOW se = { sizeof(se) };
            se.lpVerb = L"runas";
            se.lpFile = self;
            se.lpParameters = args.c_str();
            se.nShow = SW_SHOWNORMAL;
            if (ShellExecuteExW(&se))
                EndDialog(h, 0);
        }
    }
    else
        MessageBoxW(h, T(L"Installation nicht ausgeführt bzw. abgebrochen – Einzelheiten im Fenster.", L"Installation not done or aborted – see the window for details."),
            L"GothicCoop", MB_ICONWARNING | MB_SETFOREGROUND);
    EnableWindow(GetDlgItem(h, IDC_UNDO), !LastRun(c.dir).empty());
}

static void UiUndo(HWND h) {
    Ctx& c = UiCtx;
    c.dir = UiDir(h);
    c.game = UiGame(h);
    c.ui = h;
    const std::wstring run = LastRun(c.dir);
    if (run.empty())
        return;
    if (MessageBoxW(h, (std::wstring(T(L"Die letzte Installation rückgängig machen?\n\n", L"Undo the last installation?\n\n")) + run).c_str(),
        L"GothicCoop", MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;
    SetDlgItemTextW(h, IDC_STATUS, L"");
    const int r = DoUndo(c);
    MessageBoxW(h, r == 0 ? T(L"Rückgängig gemacht.", L"Undone.") : T(L"Nicht vollständig – Einzelheiten im Fenster.", L"Not complete – see the window."),
        L"GothicCoop", r == 0 ? MB_ICONINFORMATION : MB_ICONWARNING);
    EnableWindow(GetDlgItem(h, IDC_UNDO), !LastRun(c.dir).empty());
}

static std::wstring InitDir;
static Game InitGame = G_NONE;
static bool InitRenderer = true;

static INT_PTR CALLBACK DlgProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_INITDIALOG: {
        SetWindowTextW(h, (std::wstring(L"GothicCoop ") + Fassung + T(L" – Installation", L" – Setup")).c_str());
        SetDlgItemTextW(h, IDC_TITLE, (std::wstring(T(L"GothicCoop ", L"GothicCoop ")) + Fassung +
            T(L" für Gothic 1, Gothic 2 Die Nacht des Raben und Othello", L" for Gothic 1, Gothic 2 Night of the Raven and Othello")).c_str());
        SetDlgItemTextW(h, IDC_GRP_GAME, T(L"1. Welches Spiel ist installiert?", L"1. Which game is installed?"));
        SetDlgItemTextW(h, IDC_RB_G1, T(L"Gothic 1", L"Gothic 1"));
        SetDlgItemTextW(h, IDC_RB_G2, T(L"Gothic 2 Die Nacht des Raben", L"Gothic 2 Night of the Raven"));
        SetDlgItemTextW(h, IDC_RB_OTH, T(L"Gothic 1 mit Othello", L"Gothic 1 with Othello"));
        SetDlgItemTextW(h, IDC_GRP_DIR, T(L"2. Spielordner (der Ordner, in dem \"system\" liegt)", L"2. Game folder (the one containing \"system\")"));
        SetDlgItemTextW(h, IDC_BROWSE, T(L"Durchsuchen …", L"Browse …"));
        SetDlgItemTextW(h, IDC_RENDERER, T(L"Coop-Renderer installieren (fehlt GD3D11, wird es mitinstalliert und der Coop-Renderer darübergelegt)",
            L"Install the coop renderer (if GD3D11 is missing, it is installed and the coop renderer laid over it)"));
        SetDlgItemTextW(h, IDC_UNDO, T(L"Letzte Installation rückgängig", L"Undo last installation"));
        SetDlgItemTextW(h, IDC_INSTALL, T(L"Installieren", L"Install"));
        SetDlgItemTextW(h, IDCANCEL, T(L"Beenden", L"Close"));
        CheckDlgButton(h, IDC_RENDERER, InitRenderer ? BST_CHECKED : BST_UNCHECKED);
        Found = FindGames();
        HWND cb = GetDlgItem(h, IDC_DIR);
        for (auto& c : Found)
            SendMessageW(cb, CB_ADDSTRING, 0, (LPARAM)c.dir.c_str());
        Game g = InitGame != G_NONE ? InitGame : !InitDir.empty() ? DetectGame(Inspect(InitDir, G_NONE)) : G_NONE;
        if (g == G_NONE)
            g = !Found.empty() && InitDir.empty() ? Found[0].game : G_G2;
        UiSetGame(h, g);
        if (!InitDir.empty())
            SetDlgItemTextW(h, IDC_DIR, InitDir.c_str());
        else if (!Found.empty())
            SendMessageW(cb, CB_SETCURSEL, 0, 0);
        UiRefresh(h);
        if (AutoStart)
            PostMessageW(h, WM_COMMAND, MAKEWPARAM(IDC_INSTALL, BN_CLICKED), 0);
        return TRUE;
    }
    case WM_TIMER:
        if (w == 1 && !UiBusy) {
            KillTimer(h, 1);
            UiRefresh(h);
        }
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(w)) {
        case IDC_RB_G1:
        case IDC_RB_G2:
        case IDC_RB_OTH:
        case IDC_RENDERER:
            if (HIWORD(w) == BN_CLICKED)
                UiRefresh(h);
            return TRUE;
        case IDC_DIR:
            if (HIWORD(w) == CBN_SELCHANGE) {
                const int k = (int)SendDlgItemMessageW(h, IDC_DIR, CB_GETCURSEL, 0, 0);
                if (k >= 0 && k < (int)Found.size()) {
                    SetDlgItemTextW(h, IDC_DIR, Found[k].dir.c_str());
                    UiSetGame(h, Found[k].game);
                }
                UiRefresh(h);
            }
            else if (HIWORD(w) == CBN_EDITCHANGE)
                SetTimer(h, 1, 500, NULL);
            else if (HIWORD(w) == CBN_KILLFOCUS)
                UiDirChosen(h, UiDir(h));
            return TRUE;
        case IDC_BROWSE:
            UiBrowse(h);
            return TRUE;
        case IDC_INSTALL:
            UiInstall(h);
            return TRUE;
        case IDC_UNDO:
            UiUndo(h);
            return TRUE;
        case IDCANCEL:
            EndDialog(h, 0);
            return TRUE;
        }
        break;
    case WM_CLOSE:
        if (!UiBusy)   // nicht waehrend der Installation
            EndDialog(h, 0);
        return TRUE;
    }
    return FALSE;
}

// --- Start ---------------------------------------------------------------------------------------------------------------
static bool ArgValue(const std::wstring& a, const wchar_t* key, std::wstring& v) {
    const size_t n = wcslen(key);
    if (_wcsnicmp(a.c_str(), key, n) != 0)
        return false;
    v = a.substr(n);
    if (v.size() >= 2 && v.front() == L'"' && v.back() == L'"')
        v = v.substr(1, v.size() - 2);
    return true;
}

static Game GameFromKey(const std::wstring& k) {
    if (_wcsicmp(k.c_str(), L"g1") == 0)
        return G_G1;
    if (_wcsicmp(k.c_str(), L"g2") == 0)
        return G_G2;
    if (_wcsicmp(k.c_str(), L"othello") == 0)
        return G_OTH;
    return G_NONE;
}

static int Verify(const std::wstring& report) {
    std::string out;
    int bad = 0;
    unsigned long long total = 0;
    for (auto& e : Entries) {
        std::vector<unsigned char> d;
        const bool ok = Unpack(e, d);
        if (!ok)
            bad++;
        total += e.raw;
        out += ToUtf8(std::wstring(ok ? L"ok     " : L"FEHLER ") + std::to_wstring(e.comp) + L" " + e.path + L"\r\n");
    }
    out += ToUtf8(std::wstring(L"Fassung ") + Fassung + L", GD3D11 " + Gd3d11Ver + L", " + std::to_wstring(Entries.size()) + L" Dateien, " +
        std::to_wstring(total) + L" Bytes, " + std::to_wstring(bad) + L" Fehler\r\n");
    if (!report.empty()) {
        HANDLE h = CreateFileW(report.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (h != INVALID_HANDLE_VALUE) {
            DWORD put = 0;
            WriteFile(h, out.data(), (DWORD)out.size(), &put, NULL);
            CloseHandle(h);
        }
    }
    return bad ? 1 : 0;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, LPWSTR, int) {
    En = PRIMARYLANGID(GetUserDefaultUILanguage()) != LANG_GERMAN;
    std::wstring why;
    const bool payload = LoadPayload(why);
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    bool silent = false, undo = false, verify = false;
    std::wstring report;
    for (int k = 1; k < argc; k++) {
        const std::wstring a = argv[k];
        std::wstring v;
        if (_wcsicmp(a.c_str(), L"/still") == 0)
            silent = true;
        else if (_wcsicmp(a.c_str(), L"/rueckgaengig") == 0)
            undo = true;
        else if (_wcsicmp(a.c_str(), L"/pruefen") == 0)
            verify = true;
        else if (_wcsicmp(a.c_str(), L"/ohnerenderer") == 0)
            InitRenderer = false;
        else if (_wcsicmp(a.c_str(), L"/ddraw") == 0)
            UiCtx.replaceForeignDdraw = true;
        else if (_wcsicmp(a.c_str(), L"/union") == 0)
            UiCtx.unionFiles = true;
        else if (_wcsicmp(a.c_str(), L"/ohneunion") == 0)
            NoUnionAsk = true;
        else if (_wcsicmp(a.c_str(), L"/los") == 0)
            AutoStart = true;
        else if (ArgValue(a, L"/spiel:", v))
            InitGame = GameFromKey(v);
        else if (ArgValue(a, L"/ordner:", v))
            InitDir = v;
        else if (ArgValue(a, L"/bericht:", v))
            report = v;
    }
    if (argv)
        LocalFree(argv);
    if (!payload) {
        if (!silent && !verify)
            MessageBoxW(NULL, (std::wstring(T(L"Der Installer ist beschädigt: ", L"The installer is damaged: ")) + why).c_str(), L"GothicCoop", MB_ICONERROR);
        return 1;
    }
    if (verify)
        return Verify(report);
    if (silent) {
        Ctx c;
        c.dir = InitDir;
        while (!c.dir.empty() && c.dir.back() == L'\\')
            c.dir.pop_back();
        c.game = InitGame;
        c.renderer = InitRenderer;
        c.replaceForeignDdraw = UiCtx.replaceForeignDdraw;
        c.unionFiles = UiCtx.unionFiles;
        if (c.dir.empty() || (c.game == G_NONE && !undo))
            return 2;
        return undo ? DoUndo(c) : DoInstall(c);
    }
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);
    DialogBoxParamW(inst, MAKEINTRESOURCEW(IDD_MAIN), NULL, DlgProc, 0);
    CoUninitialize();
    return 0;
}
