// Supported with union (c) 2020 Union team
// Union SOURCE file

#include <winioctl.h>   // u34: FSCTL_SET_REPARSE_POINT (Ordner-Verknuepfung)
#include <intrin.h>     // u35: _AddressOfReturnAddress (Mitschnitt im Weltprozess)
#include <tlhelp32.h>   // u43: Faeden eines haengenden Weltprozesses (WpDumpThreads)

extern "C" void(__cdecl* CoopProcessDetachHook)(int terminating);   // u35: DllMain.cpp

namespace GOTHIC_ENGINE {

    // --- u33 (P75): Weltprozess - zweites Gothic beim Host (docs\WELTPROZESS-PLAN.md Schritt 1) -----------------------------
    //
    // Nutzer: "machen wir den naechsten Schritt mit einem parallelen Prozess, der laeuft" (Option 3: der Host laedt beide Welten
    // und kann dem anderen Spieler zuschauen). Gothic fuehrt nur eine Welt je Prozess. Schritt 1: der Host startet ein zweites
    // Gothic2.exe, das die Welt eines Mitspielers laedt, der nicht bei ihm ist, und ihm dort zuschaut. Gefuehrt wird die Welt noch
    // nicht (Schritt 2): die NPCs dort rechnet der Weltprozess selbst, der Client in jener Welt weiter auch.
    //
    // Engine-Befunde (Gothic2.exe 2.6, alle geprueft):
    //  - Einzelinstanz-Sperre: die Hilfsfunktion winDoInstanceCheck (0x005035C0) ruft niemand - dieselbe Sperre ("Lock_Gothic II",
    //    ExitProcess(1)) steht aber direkt in WinMain (0x00502ED0, u37/P79, unten WpSkipInstanceLock; u33 hatte sie uebersehen).
    //    Mutexe namenlos, FindWindowA sucht nur "[zSpy]"; Union, SystemPack, GD3D11 sperren nicht.
    //  - Ohne Fokus laeuft Gothic im Fenster weiter: sysEvent (0x005053E0) holt Nachrichten mit PeekMessage und wechselt nur im
    //    Vollbild bei Fokusverlust ins Fenster (0x00505535). Kommandozeile: -zwindow (auch GD3D11), -zres:B,H,Tiefe, -znosound,
    //    -znomusic.
    //  - Spielstaende: Gothic baut jeden Pfad aus zoptions->GetDirString(DIR_SAVEGAMES) (z. B. 0x00438AB7: + "current\") bzw.
    //    wechselt mit zoptions->ChangeDir(DIR_SAVEGAMES) in das Verzeichnis-Objekt (0x00438B6E, ClearCurrent 0x00439694,
    //    0x006C52BF). Gemeinsame Saves\current waeren verheerend (ClearCurrent raeumte beim Laden die Staende der verlassenen
    //    Welten des Hosts aus). u33 stellte beides auf Saves_CoopWelt um; ab u34 hat der Weltprozess einen eigenen Spielordner
    //    (unten) und prueft vor dem Laden nur noch, dass er wirklich dort laeuft.
    //  - Gothic.ini schreibt Gothic erst am Ende von CGameManager::Run (0x00425C09). Der Weltprozess endet immer per
    //    TerminateProcess (auch beim Schliessen des Fensters, eigene Fensterprozedur) - Gothic.ini des Hosts bleibt unberuehrt.
    //  - Spawn-Manager (InitCameraPos 0x007788C0, CheckRemoveNpc 0x00779346): Bezugspunkt ist die Kamera, nicht der Held.
    //    GT selbst entfernt die Figur eines Mitspielers ab 45 m vom eigenen Helden (RemoteNpc::RespawnOrDestroyBasedOnDistance).
    //
    // Ablauf:
    //  - Host, Shift+F12: Mitspieler in einer anderen Welt suchen (CoopPeerWorlds), eigenen Spielordner CoopWelt auffrischen,
    //    neuesten Spielstand des Hosts nach CoopWelt\Saves\savegameN kopieren, dazu die Weltdatei jener Welt aus Saves\current,
    //    wenn sie neuer ist. Zwei Pipes (je Richtung eine), dann CoopWelt\system\Gothic2.exe -coopwelt:<PID>:<Fenster>:<Sitzung>
    //    -zwindow -zres -znosound -znomusic, minimiert, in einem Job-Objekt (endet mit dem Host). Shift+F12 erneut beendet ihn.
    //  - Weltprozess: eigenes Log (GothicCoopWelt.log, kein Archivieren), Pruefung des eigenen Ordners, Fenster minimiert, im
    //    Hauptmenue den Slot laden, danach Weltwechsel in die Welt des Mitspielers (oCGame::TriggerChangeLevel ohne Startpunkt).
    //    GT laeuft dort ohne Netz-Faden (wie allein); statt Netz kommen ueber die Pipe die Figur-Pakete des Mitspielers (Lage,
    //    Blick, Animation, INIT_NPC, Ausruestung, Leben ...), die der Host ohnehin empfaengt, und die Uhrzeit.
    //  - Eigener Held des Weltprozesses: unsichtbar, ohne Kollision und Physik, 30 m ueber dem Mitspieler (nah genug, dass GT
    //    dessen Figur behaelt, weit genug, dass kein NPC ihn wahrnimmt). Die Kamera folgt der Figur des Mitspielers.
    //  - Wechselt der Mitspieler die Welt, wechselt der Weltprozess mit; kommt er in die Welt des Hosts, endet der Weltprozess.
    // Bis u41 nur G2A (Gothic 2 Die Nacht des Raben). u42 (P84): auch G1 (GothicMod.exe 1.08k_mod) - dieselbe Sperre in WinMain
    // (0x004F3F60, Sprung 0x004F3FB2, gleiche Bytes), der Weltprozess startet dieselbe Exe wie das Hauptspiel (WpExeName),
    // Renderer g1_avx2 ab coop8. G1A/G2 (Classic) bleiben aus.
    //
    // u34 (P76, Test u33: auf beiden PCs endete der Weltprozess nach 4-33 s mit 0xC0000417, ohne eine Zeile GT-Log): Union
    // packt bei jedem Start seine Plugins aus Data\Union.vdf nach Data\$Templates$\ (asmcl, zParserExtender, ZBINKFIX, ZMOUSEFIX,
    // ZUNIONUTILS - Zeitstempel = Start des Hauptspiels). Das Hauptspiel hat sie geladen, das zweite Gothic kann sie nicht
    // ueberschreiben; Union schreibt dann in die nicht geoeffnete Datei (fopen -> fwrite ohne Pruefung, z. B. Shw32 0x10032E20)
    // und msvcr100 beendet den Prozess (STATUS_INVALID_CRT_PARAMETER) - noch bevor GT geladen ist. Jetzt laeuft der Weltprozess
    // in einem eigenen Spielordner <Spiel>\CoopWelt\: Exe, DLLs, VDF-Archive als harte Verknuepfungen (kein Platz), _work und
    // Miles als Ordner-Verknuepfung (Junction), Konfiguration als Kopie (Gothic.ini mit Fenster, ohne Ton und Logo-Videos),
    // eigene Data\$Templates$, Saves und Logs. Damit entfaellt das Umstellen des Spielstand-Ordners (Saves_CoopWelt aus u33).
    //
    // u38 (P80): Nutzer: "die Idee war ja dass man mit Shift + F5 das Fenster im Spiel sieht" und "bei beiden Spielen die zweite
    // Instanz bereits zu Anfang starten, die sind dann erstmal im Leerlauf, bis dann wirklich einer die eigene Spielwelt verlaesst
    // oder direkt in der anderen Spielwelt durch einen Savegame startet".
    //  - Vorstart: Host und Client starten ihren Weltprozess, sobald Coop laeuft (5 s nach dem Laden, GothicCoopConfig.json
    //    "coopWorldProcessAuto", Vorgabe true). Er bootet bis ins Hauptmenue und wartet dort mit 5 Bildern/s.
    //  - Auftrag: ist ein Mitspieler (sendend - nicht gerade beim Laden) 4 s lang in einer anderen Welt (beim gemeinsamen
    //    Weltwechsel 20 s, nie waehrend ich gleich nachkomme), bekommt der Weltprozess den neuesten Spielstand und dessen Welt -
    //    gleich, ob der Mitspieler hindurchgegangen ist oder einen Spielstand in einer anderen Welt geladen hat. Der Client fuettert
    //    ihn mit den Figur-Paketen des Hosts (und anderer Clients), der Host mit denen der Clients.
    //  - Kommt der Mitspieler in meine Welt: "idle" - der Weltprozess bleibt in seiner Welt und ruht (5 Bilder/s). Der naechste
    //    Auftrag laedt den dann neuesten Spielstand im Spiel neu (CGameManager::Read_Savegame 0x0042A040) und wechselt in die Welt.
    //  - Bild: bereit (Welt geladen, Figur da) teilt er sein Weltbild als Textur (Renderer coop8), Shift+F5 zeigt es im
    //    Zuschau-Fenster (CoopCamera.cpp, CoopWpImage).
    //  - Shift+F12 beendet ihn (kein Vorstart mehr) bzw. startet ihn wieder. Bricht er unerwartet ab, naechster Vorstart nach 2 min,
    //    nach dem dritten Mal keiner mehr.

#if (ENGINE == Engine_G2A || ENGINE == Engine_G1) && COOP_BUILD(Save)   // u42: auch G1

    // --- Verbindung: zwei Pipes, Rahmen = 4 Byte Laenge + BSON ---------------------------------------------------------
    //
    // Je Richtung eine Pipe: ein synchrones Pipe-Handle arbeitet Lesen und Schreiben nacheinander ab - ein wartendes ReadFile
    // haette jedes WriteFile blockiert. Jede Seite hat einen Lese- und einen Schreibfaden; der Hauptfaden reiht nur ein bzw. aus.
    // Ein WpLink wird nie freigegeben (die Faeden koennen ihn nach dem Beenden noch kurz anfassen) - wenige Hundert Byte je Start.
    struct WpLink {
        HANDLE in = INVALID_HANDLE_VALUE, out = INVALID_HANDLE_VALUE;
        HANDLE readThread = NULL, writeThread = NULL;
        bool server = false;
        SafeQueue<json> inbox, outbox;
        volatile LONG connected = 0;   // 2 = beide Pipes verbunden
        volatile LONG broken = 0;
        volatile LONG stop = 0;
    };

    static bool WpIo(HANDLE h, void* p, DWORD n, bool write) {
        char* c = (char*)p;
        while (n) {
            DWORD done = 0;
            const BOOL ok = write ? WriteFile(h, c, n, &done, NULL) : ReadFile(h, c, n, &done, NULL);
            if (!ok || !done)
                return false;
            c += done;
            n -= done;
        }
        return true;
    }

    static bool WpSendFrame(HANDLE h, const json& j) {
        std::vector<std::uint8_t> b;
        try {
            b = json::to_bson(j);
        }
        catch (...) {
            return true;   // nicht darstellbar - auslassen
        }
        DWORD n = (DWORD)b.size();
        return WpIo(h, &n, 4, true) && WpIo(h, b.data(), n, true);
    }

    static bool WpRecvFrame(HANDLE h, json& j) {
        DWORD n = 0;
        if (!WpIo(h, &n, 4, false) || n == 0 || n > (16u << 20))
            return false;
        std::vector<std::uint8_t> b(n);
        if (!WpIo(h, b.data(), n, false))
            return false;
        try {
            j = json::from_bson(b);
        }
        catch (...) {
            j = json();
        }
        return true;
    }

    static DWORD WINAPI WpReadThread(LPVOID p) {
        WpLink* l = (WpLink*)p;
        if (l->server && !ConnectNamedPipe(l->in, NULL) && GetLastError() != ERROR_PIPE_CONNECTED) {
            InterlockedExchange(&l->broken, 1);
            CloseHandle(l->in);
            return 0;
        }
        InterlockedIncrement(&l->connected);
        while (!l->stop) {
            json j;
            if (!WpRecvFrame(l->in, j))
                break;
            if (j.is_object())
                l->inbox.enqueue(j);
        }
        InterlockedExchange(&l->broken, 1);
        CloseHandle(l->in);
        return 0;
    }

    static DWORD WINAPI WpWriteThread(LPVOID p) {
        WpLink* l = (WpLink*)p;
        if (l->server && !ConnectNamedPipe(l->out, NULL) && GetLastError() != ERROR_PIPE_CONNECTED) {
            InterlockedExchange(&l->broken, 1);
            CloseHandle(l->out);
            return 0;
        }
        InterlockedIncrement(&l->connected);
        while (!l->stop) {
            json j = l->outbox.dequeue();
            if (j.is_null() || l->stop)
                break;   // Ende (WpLinkClose)
            if (!WpSendFrame(l->out, j))
                break;
        }
        InterlockedExchange(&l->broken, 1);
        CloseHandle(l->out);
        return 0;
    }

    static void WpLinkStart(WpLink* l) {
        l->readThread = CreateThread(NULL, 0, WpReadThread, l, 0, NULL);
        l->writeThread = CreateThread(NULL, 0, WpWriteThread, l, 0, NULL);
    }

    // Faeden beenden: Schreibfaden ueber ein leeres Element, wartende Pipe-Aufrufe (ConnectNamedPipe, ReadFile) abbrechen
    static void WpLinkClose(WpLink* l) {
        if (!l)
            return;
        InterlockedExchange(&l->stop, 1);
        l->outbox.enqueue(json());
        if (l->readThread) {
            CancelSynchronousIo(l->readThread);
            CloseHandle(l->readThread);
            l->readThread = NULL;
        }
        if (l->writeThread) {
            CancelSynchronousIo(l->writeThread);
            CloseHandle(l->writeThread);
            l->writeThread = NULL;
        }
    }

    static std::string WpPipeName(unsigned long hostPid, int session, const char* dir) {
        char b[128];
        _snprintf(b, sizeof(b), "\\\\.\\pipe\\GothicCoopWelt-%lu-%d-%s", hostPid, session, dir);
        b[sizeof(b) - 1] = 0;
        return b;
    }

    static std::string WpStr(const json& j, const char* k) {
        return j.contains(k) && j[k].is_string() ? j[k].get<std::string>() : std::string();
    }

    static int WpInt(const json& j, const char* k, int def) {
        return j.contains(k) && j[k].is_number_integer() ? j[k].get<int>() : def;
    }

    static void WpLog(const char* fmt, ...) {
        char buf[900];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Weltprozess: %s", DiagSeconds(), CoopWpIsWorld() ? "Welt" : DiagRole(), buf);
    }

    // --- u35 (P77): Mitschnitt im Weltprozess - wer meldet einen Fehler, wer beendet ----------------------------------------
    //
    // Test u34: mit laufendem Hauptspiel endete der Weltprozess 5 s nach dem Start mit Code 1, gleich nach GTs Game_Entry und vor
    // dem ersten Lesen der Gothic.ini (GD3D11: "Shutting down..."); allein lief er bis Game_Init. Union/VDFS haben mehrere Stellen
    // mit Meldung + exit (z. B. Vdfs32g 0x10001C8F/0x10029A0C, Shw32 0x1000E44C). Nur im Weltprozess (sonst Adresse 0 = kein Haken):
    //  - MessageBoxA/W: Text, Titel und Aufrufer ins Log, dann das Original.
    //  - TerminateProcess auf den eigenen Prozess: Code und Aufrufer.
    //  - exit/ExitProcess: kein Haken - DllMain meldet die Abmeldung beim Prozessende (CoopProcessDetachHook), der Stapel enthaelt
    //    dann noch den Aufrufer. Aufrufer = Ruecksprungadressen auf dem Stapel (Wert in einem Modul, davor ein call).

    static void WpStackScan(const char* what) {
        char out[1600];
        size_t len = 0;
        out[0] = 0;
        const DWORD* p = (const DWORD*)_AddressOfReturnAddress();
        const NT_TIB* tib = (const NT_TIB*)NtCurrentTeb();
        const DWORD* top = (const DWORD*)tib->StackBase;
        int found = 0;
        for (; p < top && found < 24; p++) {
            const DWORD v = *p;
            HMODULE m = NULL;
            if (v < 0x10000 || !GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCSTR)(ULONG_PTR)v, &m) || !m || v < (DWORD)(ULONG_PTR)m + 0x1000)
                continue;
            MEMORY_BASIC_INFORMATION mbi;
            if (!VirtualQuery((LPCVOID)(ULONG_PTR)(v - 6), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT ||
                !(mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
                continue;
            const unsigned char* c = (const unsigned char*)(ULONG_PTR)v;
            const bool call = c[-5] == 0xE8 || (c[-6] == 0xFF && (c[-5] & 0x38) == 0x10) || (c[-2] == 0xFF && (c[-1] & 0x38) == 0x10) ||
                (c[-3] == 0xFF && (c[-2] & 0x38) == 0x10);
            if (!call)
                continue;
            char mod[128];
            CoopModuleOf((const void*)(ULONG_PTR)v, mod, sizeof(mod));
            const int n = _snprintf(out + len, sizeof(out) - len - 1, "%s%s", found ? " < " : "", mod);
            if (n < 0)
                break;
            len += (size_t)n;
            found++;
        }
        out[sizeof(out) - 1] = 0;
        WpLog("%s - Aufrufer (Stapel, innen zuerst): %s", what, found ? out : "keine gefunden");
    }

    static void __cdecl WpDetach(int terminating) {
        if (!terminating)
            return;   // FreeLibrary, kein Prozessende
        WpLog("Prozess endet ueber exit/ExitProcess - unerwartet (der Weltprozess endet sonst mit TerminateProcess)");
        WpStackScan("Prozessende");
    }

    static uint WpSysProc(const char* dll, const char* name) {
        if (!CoopWpIsWorld())
            return 0;
        HMODULE h = GetModuleHandleA(dll);
        return h ? (uint)(ULONG_PTR)GetProcAddress(h, name) : 0;
    }

    int WINAPI WpMessageBoxA(HWND h, LPCSTR text, LPCSTR cap, UINT flags);
    CInvoke<int(WINAPI*)(HWND, LPCSTR, LPCSTR, UINT)> Ivk_WpMessageBoxA(WpSysProc("user32.dll", "MessageBoxA"), &WpMessageBoxA);
    int WINAPI WpMessageBoxA(HWND h, LPCSTR text, LPCSTR cap, UINT flags) {
        WpLog("Meldung \"%s\": %s", cap ? cap : "", text ? text : "");
        WpStackScan("Meldung");
        return Ivk_WpMessageBoxA(h, text, cap, flags);
    }

    int WINAPI WpMessageBoxW(HWND h, LPCWSTR text, LPCWSTR cap, UINT flags);
    CInvoke<int(WINAPI*)(HWND, LPCWSTR, LPCWSTR, UINT)> Ivk_WpMessageBoxW(WpSysProc("user32.dll", "MessageBoxW"), &WpMessageBoxW);
    int WINAPI WpMessageBoxW(HWND h, LPCWSTR text, LPCWSTR cap, UINT flags) {
        char t[700] = "", c[160] = "";
        if (text)
            WideCharToMultiByte(CP_ACP, 0, text, -1, t, sizeof(t) - 1, NULL, NULL);
        if (cap)
            WideCharToMultiByte(CP_ACP, 0, cap, -1, c, sizeof(c) - 1, NULL, NULL);
        WpLog("Meldung \"%s\": %s", c, t);
        WpStackScan("Meldung");
        return Ivk_WpMessageBoxW(h, text, cap, flags);
    }

    BOOL WINAPI WpTerminateProcess(HANDLE h, UINT code);
    CInvoke<BOOL(WINAPI*)(HANDLE, UINT)> Ivk_WpTerminateProcess(WpSysProc("kernel32.dll", "TerminateProcess"), &WpTerminateProcess);
    BOOL WINAPI WpTerminateProcess(HANDLE h, UINT code) {
        if (h == GetCurrentProcess() || GetProcessId(h) == GetCurrentProcessId()) {
            WpLog("TerminateProcess auf sich selbst, Code %u (0x%08X)", code, code);
            WpStackScan("TerminateProcess");
        }
        return Ivk_WpTerminateProcess(h, code);
    }

    // u37 (P79): Gothics Einzelinstanz-Sperre. Test u36 (Mitschnitt: ExitProcess aus Gothic2.exe+102F2C): WinMain (0x00502ED0) legt
    // CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READONLY, 0, 0x20, "Lock_Gothic II") an; meldet GetLastError
    // ERROR_ALREADY_EXISTS (das Hauptspiel laeuft), ExitProcess(1). Die gleichnamige Hilfsfunktion winDoInstanceCheck (0x005035C0)
    // ruft niemand - die Sperre steht noch einmal direkt in WinMain (u33 hatte nur die Hilfsfunktion geprueft). GTs Game_Entry
    // laeuft davor: im Weltprozess wird der bedingte Sprung 0x00502F22 (jne +8 ueber "push 1; call [ExitProcess]") im Speicher zu
    // jmp. Die Exe-Datei (harte Verknuepfung auf die des Hauptspiels) bleibt unveraendert; das Hauptspiel behaelt seine Sperre.
    static void WpSkipInstanceLock(const char* where = "im Weltprozess") {   // u49: auch in der Testinstanz (CoopAutoTest.cpp)
        unsigned char* p = (unsigned char*)CoopAddr::winMain_LockJne;
        if (!p) {
            WpLog("Einzelinstanz-Sperre: Stelle in dieser Engine unbekannt - nicht geaendert");
            return;
        }
        static const unsigned char want[] = { 0x75, 0x08, 0x6A, 0x01, 0xFF, 0x15 };   // jne +8; push 1; call dword ptr [ExitProcess]
        if (p[0] == 0xEB && p[1] == 0x08) {
            WpLog("Einzelinstanz-Sperre: schon aufgehoben");
            return;
        }
        if (memcmp(p, want, sizeof(want)) != 0) {
            WpLog("Einzelinstanz-Sperre: Bytes bei %p unerwartet (%02X %02X %02X %02X %02X %02X) - nicht geaendert", (void*)p, p[0], p[1], p[2], p[3], p[4], p[5]);
            return;
        }
        DWORD old = 0;
        if (!VirtualProtect(p, 1, PAGE_EXECUTE_READWRITE, &old)) {
            WpLog("Einzelinstanz-Sperre: VirtualProtect fehlgeschlagen (Fehler %lu)", GetLastError());
            return;
        }
        p[0] = 0xEB;   // jne -> jmp
        VirtualProtect(p, 1, old, &old);
        FlushInstructionCache(GetCurrentProcess(), p, 2);
        WpLog("Einzelinstanz-Sperre \"Lock_...\" %s aufgehoben (WinMain %p: jne -> jmp, nur im Speicher)", where, (void*)p);
    }

    // Plugin.cpp Game_Entry (nur im Weltprozess, so frueh wie moeglich)
    void CoopWpEarlyInit() {
        WpSkipInstanceLock();   // u37: sonst beendet Gothic sich gleich nach Game_Entry, wenn das Hauptspiel laeuft
        CoopProcessDetachHook = &WpDetach;
        WpLog("Mitschnitt an (Meldungen %s/%s, TerminateProcess %s, Prozessende ueber DllMain)",
            Ivk_WpMessageBoxA.GetHookInfo().lpOriginal ? "ja" : "nein", Ivk_WpMessageBoxW.GetHookInfo().lpOriginal ? "ja" : "nein",
            Ivk_WpTerminateProcess.GetHookInfo().lpOriginal ? "ja" : "nein");
    }

    // Weltdatei und Ausloeser-Datei einer Welt im Spielstand: "OLDWORLD\OLDWORLD.ZEN" -> OLDWORLD.SAV,
    // TRIGGERS.OLDWORLD.OLDWORLD.SAV (so liegen sie in Saves\current)
    static std::vector<std::string> WpWorldFiles(const std::string& world) {
        std::vector<std::string> out;
        std::string w = world;
        const size_t dot = w.find_last_of('.');
        if (dot != std::string::npos)
            w = w.substr(0, dot);
        std::string base = w;
        const size_t sl = base.find_last_of("\\/");
        if (sl != std::string::npos)
            base = base.substr(sl + 1);
        for (char& c : w)
            if (c == '\\' || c == '/')
                c = '.';
        if (!base.empty())
            out.push_back(base + ".SAV");
        if (!w.empty())
            out.push_back("TRIGGERS." + w + ".SAV");
        return out;
    }

    static bool WpFileTime(const std::string& f, FILETIME& t) {
        WIN32_FILE_ATTRIBUTE_DATA a;
        if (!GetFileAttributesExA(f.c_str(), GetFileExInfoStandard, &a) || (a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            return false;
        t = a.ftLastWriteTime;
        return true;
    }

    template <class F> static void WpForEachFile(const std::string& dir, F f) {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((dir + "*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE)
            return;
        do {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                f(std::string(fd.cFileName));
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    template <class F> static void WpForEachDir(const std::string& dir, F f) {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((dir + "*").c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE)
            return;
        do {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, ".."))
                f(std::string(fd.cFileName));
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }

    static std::string WpUpper(std::string s) {
        for (char& c : s)
            c = (char)toupper((unsigned char)c);
        return s;
    }

    static std::string WpExt(const std::string& n) {
        const size_t d = n.find_last_of('.');
        return d == std::string::npos ? std::string() : WpUpper(n.substr(d));
    }

    // --- u34 (P76): eigener Spielordner <Spiel>\CoopWelt\ --------------------------------------------------------------
    //
    // Nie rekursiv loeschen: in CoopWelt stehen Ordner-Verknuepfungen auf echte Ordner des Spiels. Geloescht werden nur einzelne
    // Dateien in eigenen Ordnern (nie in einer Verknuepfung) und Verknuepfungen selbst (RemoveDirectory entfernt nur den Verweis).

    // u36 (P78): neben dem Spielordner - Test u35: das Hauptspiel fand die Plugin-Dateien in <Spiel>\CoopWelt\Data\$Templates$
    // (sein eigenes $Templates$ hatte danach kein asmcl/zParserExtender mehr) und hielt sie geladen; der Weltprozess konnte sie
    // nicht neu auspacken -> wieder 0xC0000417.
    // u41 (P83): wieder im Spielordner (Nutzer: nie neben dem Spiel) - "C:\Spiele\Gothic II" -> "C:\Spiele\Gothic II\
    // Gothic II_CoopWelt" (immer dasselbe Laufwerk). Die Plugin-Dateien darin heissen, sobald geladen, *.dll.welt (WpHidePlugins).
    static std::string WpCopyName() {
        const std::string root = CoopGameRoot();
        const size_t s = root.find_last_of("\\/");
        std::string leaf = s == std::string::npos ? root : root.substr(s + 1);
        if (leaf.empty() || leaf.find(':') != std::string::npos)
            leaf = "Gothic";   // Spiel direkt im Laufwerk (D:\system)
        return leaf + "_CoopWelt";
    }

    static std::string WpCopyRoot() {
        return CoopGameRoot() + "\\" + WpCopyName();
    }

    // u42: Exe des Hauptspiels (G2A Gothic2.exe, G1 GothicMod.exe bzw. GOTHIC.EXE) - der Weltprozess startet dieselbe
    static std::string WpExeName() {
        char p[MAX_PATH] = "";
        GetModuleFileNameA(NULL, p, MAX_PATH);
        const char* s = strrchr(p, '\\');
        return s ? s + 1 : p;
    }

    // alte Ordner: u34/u35 <Spiel>\CoopWelt (u36-u40 auch, wenn das Spiel direkt im Laufwerk lag), u36-u40 <Spiel>_CoopWelt
    static std::vector<std::string> WpOldCopyRoots() {
        const std::string root = CoopGameRoot();
        std::vector<std::string> v;
        v.push_back(root + "\\CoopWelt");
        if (root.size() > 3)
            v.push_back(root + "_CoopWelt");
        return v;
    }

    static std::wstring WpWide(const std::string& s) {
        const int n = MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, NULL, 0);
        std::wstring w(n > 1 ? n - 1 : 0, L'\0');
        if (n > 1)
            MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, &w[0], n);
        return w;
    }

    // Puffer einer Ordner-Verknuepfung (REPARSE_DATA_BUFFER, MountPointReparseBuffer - steht nur im DDK)
    struct WpMountBuf {
        DWORD tag;
        WORD dataLen, reserved;
        WORD substOff, substLen, printOff, printLen;
        WCHAR path[2 * MAX_PATH + 16];
    };

    static bool WpIsJunctionTo(const std::string& link, const std::string& target) {
        const DWORD a = GetFileAttributesA(link.c_str());
        if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY) || !(a & FILE_ATTRIBUTE_REPARSE_POINT))
            return false;
        HANDLE h = CreateFileA(link.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (h == INVALID_HANDLE_VALUE)
            return false;
        WpMountBuf b;
        ZeroMemory(&b, sizeof(b));
        DWORD got = 0;
        const BOOL ok = DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, &b, sizeof(b), &got, NULL);
        CloseHandle(h);
        if (!ok || b.tag != IO_REPARSE_TAG_MOUNT_POINT || b.substOff + b.substLen > sizeof(b.path))
            return false;
        const std::wstring subst(b.path + b.substOff / 2, b.substLen / 2);
        return _wcsicmp(subst.c_str(), (L"\\??\\" + WpWide(target)).c_str()) == 0;
    }

    // Ordner-Verknuepfung (Junction) link -> target, ohne Adminrechte. Steht dort eine alte Verknuepfung oder ein leerer Ordner,
    // wird er ersetzt; ein Ordner mit Inhalt bleibt unangetastet (Fehler).
    static bool WpMakeJunction(const std::string& link, const std::string& target, std::string& why) {
        if (WpIsJunctionTo(link, target))
            return true;
        const DWORD a = GetFileAttributesA(link.c_str());
        if (a != INVALID_FILE_ATTRIBUTES && (!(a & FILE_ATTRIBUTE_DIRECTORY) || !RemoveDirectoryA(link.c_str()))) {
            why = link + " ist eine Datei oder ein Ordner mit Inhalt - nicht angefasst";
            return false;
        }
        if (!CreateDirectoryA(link.c_str(), NULL)) {
            why = "Ordner " + link + " nicht angelegt (Fehler " + std::to_string(GetLastError()) + ")";
            return false;
        }
        HANDLE h = CreateFileA(link.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (h == INVALID_HANDLE_VALUE) {
            why = link + " nicht geoeffnet (Fehler " + std::to_string(GetLastError()) + ")";
            RemoveDirectoryA(link.c_str());
            return false;
        }
        const std::wstring t = WpWide(target), subst = L"\\??\\" + t;
        WpMountBuf b;
        ZeroMemory(&b, sizeof(b));
        bool ok = (subst.size() + t.size() + 2) * sizeof(WCHAR) <= sizeof(b.path);
        DWORD err = ERROR_FILENAME_EXCED_RANGE;
        if (ok) {
            b.tag = IO_REPARSE_TAG_MOUNT_POINT;
            b.substOff = 0;
            b.substLen = (WORD)(subst.size() * sizeof(WCHAR));
            b.printOff = (WORD)(b.substLen + sizeof(WCHAR));
            b.printLen = (WORD)(t.size() * sizeof(WCHAR));
            memcpy(b.path, subst.c_str(), b.substLen);
            memcpy((char*)b.path + b.printOff, t.c_str(), b.printLen);
            b.dataLen = (WORD)(8 + b.substLen + sizeof(WCHAR) + b.printLen + sizeof(WCHAR));
            DWORD got = 0;
            ok = DeviceIoControl(h, FSCTL_SET_REPARSE_POINT, &b, 8 + b.dataLen, NULL, 0, &got, NULL) != FALSE;
            err = GetLastError();
        }
        CloseHandle(h);
        if (!ok) {
            RemoveDirectoryA(link.c_str());
            why = "Verknuepfung " + link + " nicht gesetzt (Fehler " + std::to_string(err) + ")";
        }
        return ok;
    }

    static bool WpFileId(const std::string& f, BY_HANDLE_FILE_INFORMATION& i) {
        HANDLE h = CreateFileA(f.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
        if (h == INVALID_HANDLE_VALUE)
            return false;
        const BOOL ok = GetFileInformationByHandle(h, &i);
        CloseHandle(h);
        return ok != FALSE;
    }

    struct WpSyncCount {
        int same = 0, linked = 0, copied = 0, junctions = 0, failed = 0;
        std::string firstFail;
        void Fail(const std::string& w) {
            if (!failed++)
                firstFail = w;
        }
    };

    // harte Verknuepfung dst -> src (nichts doppelt auf der Platte); schon dieselbe Datei = nichts tun. Ohne NTFS nur kleine
    // Dateien kopieren (die VDF-Archive haben zusammen ueber 3 GB).
    static void WpLinkFile(const std::string& src, const std::string& dst, WpSyncCount& c) {
        BY_HANDLE_FILE_INFORMATION a, b;
        if (WpFileId(dst, b) && WpFileId(src, a) && a.dwVolumeSerialNumber == b.dwVolumeSerialNumber &&
            a.nFileIndexHigh == b.nFileIndexHigh && a.nFileIndexLow == b.nFileIndexLow) {
            c.same++;
            return;
        }
        const DWORD at = GetFileAttributesA(dst.c_str());
        if (at != INVALID_FILE_ATTRIBUTES) {
            if (at & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) {
                c.Fail(dst + " ist ein Ordner");
                return;
            }
            SetFileAttributesA(dst.c_str(), FILE_ATTRIBUTE_NORMAL);
            if (!DeleteFileA(dst.c_str())) {   // alte Verknuepfung (z. B. GT vor dem Einspielen einer neuen Fassung)
                c.Fail(dst + " nicht ersetzbar (Fehler " + std::to_string(GetLastError()) + ")");
                return;
            }
        }
        if (CreateHardLinkA(dst.c_str(), src.c_str(), NULL)) {
            c.linked++;
            return;
        }
        WIN32_FILE_ATTRIBUTE_DATA s;
        if (GetFileAttributesExA(src.c_str(), GetFileExInfoStandard, &s) && !s.nFileSizeHigh && s.nFileSizeLow < (50u << 20) &&
            CopyFileA(src.c_str(), dst.c_str(), FALSE))
            c.copied++;
        else
            c.Fail(dst + " nicht verknuepft (Fehler " + std::to_string(GetLastError()) + ")");
    }

    // Dateien eines Ordners: was Gothic/Union/GD3D11 zur Laufzeit schreiben, kopieren (Konfiguration; MSSSOFT.M3D packt SystemPack
    // bei jedem Start neu aus, 12:18:25 im Test u33), Logs und Windows-Dateien auslassen, alles andere verknuepfen
    static void WpSyncFiles(const std::string& src, const std::string& dst, WpSyncCount& c) {
        CreateDirectoryA(dst.c_str(), NULL);
        const DWORD da = GetFileAttributesA(dst.c_str());
        if (da == INVALID_FILE_ATTRIBUTES || !(da & FILE_ATTRIBUTE_DIRECTORY) || (da & FILE_ATTRIBUTE_REPARSE_POINT)) {
            c.Fail(dst + " ist kein eigener Ordner (Verknuepfung?) - nicht beschrieben");   // nie durch eine Verknuepfung schreiben
            return;
        }
        WpForEachFile(src + "\\", [&](const std::string& n) {
            const std::string e = WpExt(n), u = WpUpper(n);
            if (e == ".LOG" || (e == ".TXT" && u.compare(0, 3, "LOG") == 0))
                return;
            if (u == "DESKTOP.INI" || u == "THUMBS.DB")   // u35 (Test u34: Autorun\desktop.ini, versteckt - zweites Kopieren Fehler 5)
                return;
            if (e == ".INI" || e == ".D" || e == ".JSON" || e == ".CFG" || e == ".PATCH" || e == ".TXT" || e == ".M3D") {
                const DWORD ta = GetFileAttributesA((dst + "\\" + n).c_str());
                if (ta != INVALID_FILE_ATTRIBUTES && !(ta & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) &&
                    (ta & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
                    SetFileAttributesA((dst + "\\" + n).c_str(), FILE_ATTRIBUTE_NORMAL);   // sonst verweigert CopyFile das Ueberschreiben
                if (CopyFileA((src + "\\" + n).c_str(), (dst + "\\" + n).c_str(), FALSE))
                    c.copied++;
                else
                    c.Fail(dst + "\\" + n + " nicht kopiert (Fehler " + std::to_string(GetLastError()) + ")");
                return;
            }
            WpLinkFile(src + "\\" + n, dst + "\\" + n, c);
        });
    }

    // Unterordner als Verknuepfung, ausser den genannten (die baut der Aufrufer selbst auf)
    static void WpJunctionDirs(const std::string& src, const std::string& dst, std::initializer_list<const char*> own, WpSyncCount& c) {
        WpForEachDir(src + "\\", [&](const std::string& n) {
            for (const char* o : own)
                if (_stricmp(n.c_str(), o) == 0)
                    return;
            std::string why;
            if (WpMakeJunction(dst + "\\" + n, src + "\\" + n, why))
                c.junctions++;
            else
                c.Fail(why);
        });
    }

    // u41 (P83): Union laedt asmcl.dll und zParserExtender.dll (und was in [PLUGINS] PluginList steht) ueber eine Suche, die in jedem
    // Unterordner des Spielordners LoadLibrary(<Ordner>\<Name>) versucht (Shw32 0x10060BD0; folgt Verknuepfungen, versteckte Ordner
    // egal) - nachdem es sein eigenes $Templates$ geleert hat. Im Spielordner fand das Hauptspiel so die Plugin-Dateien des
    // Weltprozesses (Test u35; am 25.10. versteckt gestartet nachgestellt). Darum heissen die DLLs in dessen $Templates$, sobald sie
    // geladen sind, <Name>.welt (geladene DLLs lassen sich umbenennen) - die Suche findet dort nichts mehr. Aufgerufen im Weltprozess
    // nach dem Laden, im Hauptspiel beim Start, vor dem Start des Weltprozesses und wenn er endet.
    static void WpHidePlugins(const std::string& copyRoot, const char* why) {
        const std::string t = copyRoot + "\\Data\\$Templates$\\";
        std::vector<std::string> dlls;
        WpForEachFile(t, [&](const std::string& n) {
            if (WpExt(n) == ".DLL")
                dlls.push_back(n);
        });
        int renamed = 0, failed = 0;
        std::string firstFail;
        for (auto& n : dlls) {
            bool ok = false;
            DWORD err = 0;
            for (int i = 0; i < 10 && !ok; i++) {   // .welt noch geladen (Hauptspiel nach einem Absturz) -> .welt1 ...
                const std::string dst = t + n + ".welt" + (i ? std::to_string(i) : std::string());
                SetFileAttributesA(dst.c_str(), FILE_ATTRIBUTE_NORMAL);   // alter Rest - Union legt dort alles schreibgeschuetzt an
                ok = MoveFileExA((t + n).c_str(), dst.c_str(), MOVEFILE_REPLACE_EXISTING) != FALSE;
                if (!ok)
                    err = GetLastError();
            }
            if (ok)
                renamed++;
            else if (!failed++)
                firstFail = n + " (Fehler " + std::to_string(err) + ")";
        }
        if (renamed || failed)
            WpLog("%s: %d Plugin-Dateien im Weltordner umbenannt (*.dll.welt, fuer Unions Suche im Spielordner unsichtbar)%s%s", why,
                renamed, failed ? " - NICHT umbenannt: " : "", firstFail.c_str());
    }

    // u49: Hilfsordner im Spielordner - Weltordner <Spiel>_CoopWelt und Testordner <Spiel>_CoopTest (CoopAutoTest.cpp). Keiner wird
    // als Verknuepfung in einen anderen aufgenommen (sonst saehe z. B. der Weltprozess des Hosts den Testordner)
    static bool WpIsHelperDir(const std::string& upper) {
        for (const char* s : { "_COOPWELT", "_COOPTEST" }) {
            const size_t n = strlen(s);
            if (upper.size() > n && upper.compare(upper.size() - n, n, s) == 0)
                return true;
        }
        return false;
    }

    // <Spiel>\<Spielordner>_CoopWelt aufbauen bzw. auffrischen (jeder Start: GT, Skripte, Konfiguration koennen sich geaendert haben).
    // u49: Ziel als Parameter - derselbe Aufbau fuer den Testordner (CoopAutoTest.cpp)
    static bool WpEnsureCopyAt(const std::string& c, const char* what, std::string& why) {
        const std::string root = CoopGameRoot();
        WpSyncCount n;
        CreateDirectoryA(c.c_str(), NULL);
        // Spielordner: eigene Data, system, Saves; alle anderen Ordner (_work, Miles ...) als Verknuepfung, nicht CoopLogs/Saves*
        WpForEachDir(root + "\\", [&](const std::string& d) {
            const std::string u = WpUpper(d);
            if (u == "DATA" || u == "SYSTEM" || u == "COOPWELT" || u == "COOPLOGS" || u.compare(0, 5, "SAVES") == 0 ||
                WpIsHelperDir(u))   // u41: der Weltordner selbst liegt im Spielordner - keine Verknuepfung auf sich (u49: kein Hilfsordner)
                return;
            std::string w;
            if (WpMakeJunction(c + "\\" + d, root + "\\" + d, w))
                n.junctions++;
            else
                n.Fail(w);
        });
        WpForEachFile(root + "\\", [&](const std::string& f) {   // vdfs.cfg
            if (WpExt(f) == ".CFG" || WpExt(f) == ".INI") {
                if (CopyFileA((root + "\\" + f).c_str(), (c + "\\" + f).c_str(), FALSE))
                    n.copied++;
                else
                    n.Fail(c + "\\" + f + " nicht kopiert");
            }
        });
        // Data: Archive verknuepft, $Templates$ eigen (Union packt seine Plugins dort aus), andere Unterordner verknuepft
        WpSyncFiles(root + "\\Data", c + "\\Data", n);
        CreateDirectoryA((c + "\\Data\\$Templates$").c_str(), NULL);
        WpJunctionDirs(root + "\\Data", c + "\\Data", { "$Templates$" }, n);
        // system: Exe/DLLs verknuepft, Konfiguration kopiert; Autorun und GD3D11 eigen (Konfiguration dort), Rest verknuepft
        WpSyncFiles(root + "\\system", c + "\\system", n);
        WpJunctionDirs(root + "\\system", c + "\\system", { "Autorun", "GD3D11" }, n);
        const char* own[] = { "Autorun", "GD3D11", "GD3D11\\ZENResources" };   // ZENResources: GD3D11 schreibt <Welt>.INI beim Laden
        for (const char* o : own) {
            const std::string s = root + "\\system\\" + o;
            const DWORD a = GetFileAttributesA(s.c_str());
            if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY))
                continue;
            WpSyncFiles(s, c + "\\system\\" + o, n);
            if (_stricmp(o, "GD3D11") == 0)
                WpJunctionDirs(s, c + "\\system\\" + o, { "ZENResources" }, n);
            else
                WpJunctionDirs(s, c + "\\system\\" + o, {}, n);
        }
        WpHidePlugins(c, "vor dem Start");   // Reste eines abgebrochenen Laufs
        CreateDirectoryA((c + "\\Saves").c_str(), NULL);
        CreateDirectoryA((c + "\\Saves\\current").c_str(), NULL);
        // Gothic.ini der Kopie: Fenster, ohne Ton/Musik, ohne Logo-Videos (das Hauptspiel behaelt seine Einstellungen)
        const std::string ini = c + "\\system\\Gothic.ini";
        bool iniOk = GetFileAttributesA(ini.c_str()) != INVALID_FILE_ATTRIBUTES;
        if (iniOk) {
            iniOk = WritePrivateProfileStringA("VIDEO", "zStartupWindowed", "1", ini.c_str()) &&
                WritePrivateProfileStringA("GAME", "playLogoVideos", "0", ini.c_str()) &&
                WritePrivateProfileStringA("SOUND", "soundEnabled", "0", ini.c_str()) &&
                WritePrivateProfileStringA("SOUND", "musicEnabled", "0", ini.c_str());
        }
        const bool exeOk = GetFileAttributesA((c + "\\system\\" + WpExeName()).c_str()) != INVALID_FILE_ATTRIBUTES;
        WpLog("%s %s: %d Dateien schon verknuepft, %d neu verknuepft, %d kopiert, %d Ordner-Verknuepfungen, %d Fehler%s%s; "
            "Gothic.ini %s", what, c.c_str(), n.same, n.linked, n.copied, n.junctions, n.failed, n.failed ? " - zuerst: " : "",
            n.firstFail.c_str(), iniOk ? "angepasst (Fenster, ohne Ton, ohne Logos)" : "FEHLT");
        if (!exeOk || !iniOk || n.failed) {
            why = !exeOk ? WpExeName() + " fehlt in " + c + "\\system" : !iniOk ? "Gothic.ini fehlt in " + c + "\\system" : n.firstFail;
            return false;
        }
        return true;
    }

    static bool WpHostEnsureCopy(std::string& why) {
        return WpEnsureCopyAt(WpCopyRoot(), "eigener Spielordner", why);
    }

    // u36: Ordnerbaum entfernen, ohne je einer Verknuepfung zu folgen. Ist dir selbst eine Verknuepfung, wird nur sie geloest;
    // darin werden Ordner-Verknuepfungen nur geloest (RemoveDirectory entfernt den Verweis, nicht das Ziel), Dateien geloescht
    // (Attribute vorher zurueck), eigene Ordner erst geleert, dann entfernt. Gesperrte Dateien bleiben stehen (gezaehlt).
    static void WpRemoveTree(const std::string& dir, int& removed, int& kept) {
        const DWORD a = GetFileAttributesA(dir.c_str());
        if (a == INVALID_FILE_ATTRIBUTES)
            return;
        if (a & FILE_ATTRIBUTE_REPARSE_POINT) {
            if ((a & FILE_ATTRIBUTE_DIRECTORY) ? RemoveDirectoryA(dir.c_str()) : DeleteFileA(dir.c_str()))
                removed++;
            else
                kept++;
            return;
        }
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA((dir + "\\*").c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
                    continue;
                const std::string p = dir + "\\" + fd.cFileName;
                if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                    WpRemoveTree(p, removed, kept);   // prueft selbst auf Verknuepfung
                else {
                    SetFileAttributesA(p.c_str(), FILE_ATTRIBUTE_NORMAL);
                    if (DeleteFileA(p.c_str()))
                        removed++;
                    else
                        kept++;
                }
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
        SetFileAttributesA(dir.c_str(), FILE_ATTRIBUTE_NORMAL);
        if (RemoveDirectoryA(dir.c_str()))
            removed++;
        else
            kept++;
    }

    // Logs des Weltprozesses (system\*.log, Log*.txt seines Ordners) nach <Spiel>\CoopLogs\Weltprozess\<Zeit>\ - nie loeschen.
    // Vor jedem Start (GD3D11 leert sein Log beim Start, GT haengt an) und bevor der alte Ordner entfernt wird.
    static void WpArchiveCopyLogs(const std::string& copyRoot, const char* why) {
        const std::string sys = copyRoot + "\\system\\";
        std::vector<std::string> logs;
        WpForEachFile(sys, [&](const std::string& n) {
            const std::string e = WpExt(n), u = WpUpper(n);
            if (e == ".LOG" || (e == ".TXT" && u.compare(0, 3, "LOG") == 0))
                logs.push_back(n);
        });
        if (logs.empty())
            return;
        SYSTEMTIME st;
        GetLocalTime(&st);
        char stamp[48];
        _snprintf(stamp, sizeof(stamp), "%04d-%02d-%02d_%02d-%02d-%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
        stamp[sizeof(stamp) - 1] = 0;
        const std::string base = CoopGameRoot() + "\\CoopLogs";
        CreateDirectoryA(base.c_str(), NULL);
        CreateDirectoryA((base + "\\Weltprozess").c_str(), NULL);
        std::string target = base + "\\Weltprozess\\" + stamp;
        for (int n = 2; !CreateDirectoryA(target.c_str(), NULL) && GetLastError() == ERROR_ALREADY_EXISTS && n < 99; n++)
            target = base + "\\Weltprozess\\" + stamp + "_" + std::to_string(n);
        int moved = 0;
        for (auto& n : logs)
            if (MoveFileExA((sys + n).c_str(), (target + "\\" + n).c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH))
                moved++;
        WpLog("%d von %d Logs des Weltprozesses (%s) nach %s archiviert", moved, (int)logs.size(), why, target.c_str());
    }

    // UnionHooks.cpp _Game_Init (Hauptspiel): alte Weltordner (u34/u35 <Spiel>\CoopWelt, u36-u40 <Spiel>_CoopWelt) entfernen,
    // Logs vorher archivieren. u41: zuerst an die neue Stelle verschieben - das laufende Hauptspiel haelt seine VDF-Archive offen,
    // harte Verknuepfungen darauf lassen sich dann nicht loeschen, der Ordner mit ihnen aber verschieben (gleiches Laufwerk).
    void CoopWpHostCleanupOld() {
        const std::string now = WpCopyRoot();
        for (const std::string& old : WpOldCopyRoots()) {
            if (_stricmp(old.c_str(), now.c_str()) == 0)
                continue;
            const DWORD a = GetFileAttributesA(old.c_str());
            if (a == INVALID_FILE_ATTRIBUTES || !(a & FILE_ATTRIBUTE_DIRECTORY))
                continue;
            if (a & FILE_ATTRIBUTE_REPARSE_POINT) {   // nur der Verweis
                RemoveDirectoryA(old.c_str());
                continue;
            }
            const DWORD sa = GetFileAttributesA((old + "\\system").c_str()), da = GetFileAttributesA((old + "\\Data").c_str());
            if (sa == INVALID_FILE_ATTRIBUTES || !(sa & FILE_ATTRIBUTE_DIRECTORY) || da == INVALID_FILE_ATTRIBUTES || !(da & FILE_ATTRIBUTE_DIRECTORY)) {
                WpLog("%s ist kein Weltordner (system/Data fehlen) - nicht angefasst", old.c_str());
                continue;
            }
            WpArchiveCopyLogs(old, "alter Weltordner");
            if (GetFileAttributesA(now.c_str()) == INVALID_FILE_ATTRIBUTES && MoveFileA(old.c_str(), now.c_str())) {
                WpLog("alter Weltordner %s nach %s verschoben (u41: im Spielordner)", old.c_str(), now.c_str());
                continue;
            }
            const DWORD err = GetLastError();
            int removed = 0, kept = 0;
            WpRemoveTree(old, removed, kept);
            WpLog("alter Weltordner %s entfernt (nicht verschoben, Fehler %lu): %d Eintraege, %d noch gesperrt%s", old.c_str(),
                (unsigned long)err, removed, kept, kept ? " (naechster Start)" : "");
        }
        // u41: hat das Hauptspiel Plugins aus dem Weltordner geladen (Rest eines Absturzes vor dem Umbenennen)? Dann ebenfalls
        // umbenennen - sonst kann der Weltprozess sie nicht neu auspacken (P76/P78).
        const char* names[] = { "asmcl.dll", "zParserExtender.dll" };
        for (const char* n : names) {
            char p[MAX_PATH] = "";
            HMODULE m = GetModuleHandleA(n);
            if (m && GetModuleFileNameA(m, p, MAX_PATH) && _strnicmp(p, (now + "\\").c_str(), now.size() + 1) == 0)
                WpLog("Hauptspiel hat %s aus dem Weltordner geladen (%s) - wird umbenannt", n, p);
        }
        WpHidePlugins(now, "Spielstart");
    }

    // letzte Zeilen eines Logs des Weltprozesses ins eigene Log (geht mit dem Log-Versand auch zum Partner)
    static void WpHostLogTail(const std::string& file, int lines) {
        FILE* f = fopen(file.c_str(), "rb");
        if (!f) {
            WpLog("  %s: nicht vorhanden", file.c_str());
            return;
        }
        fseek(f, 0, SEEK_END);
        const long size = ftell(f);
        const long from = size > 24576 ? size - 24576 : 0;
        fseek(f, from, SEEK_SET);
        std::string s((size_t)(size - from), '\0');
        const size_t got = s.empty() ? 0 : fread(&s[0], 1, s.size(), f);
        fclose(f);
        s.resize(got);
        std::vector<std::string> out;
        size_t p = 0;
        while (p < s.size()) {
            size_t e = s.find('\n', p);
            if (e == std::string::npos)
                e = s.size();
            std::string l = s.substr(p, e - p);
            while (!l.empty() && (l[l.size() - 1] == '\r'))
                l.erase(l.size() - 1);
            if (!l.empty())
                out.push_back(l.size() > 300 ? l.substr(0, 300) : l);
            p = e + 1;
        }
        const size_t first = out.size() > (size_t)lines ? out.size() - lines : 0;
        WpLog("  letzte %d Zeilen von %s:", (int)(out.size() - first), file.c_str());
        for (size_t i = first; i < out.size(); i++)
            WpLog("  | %s", out[i].c_str());
    }

    // Figur-Pakete, die der Weltprozess braucht, um den Mitspieler zu zeigen (LocalNpc 0-22 ohne Treffer, Zauber, Uhrzeit,
    // Namensvergabe, Trennen, Gegenstaende - die wuerden seine Welt veraendern bzw. kommen gesondert)
    // u46 (P87): dazu Treffer und Zauber - der Weltprozess spiegelt jetzt die Welt (NPCs sind dort Puppen), Kaempfe der Figur sollen
    // im Zuschau-Fenster zu sehen sein (Treffer-Reaktion, K.o., Tod wie beim Client)
    static bool WpForwardType(int t) {
        switch (t) {
        case SYNC_POS: case SYNC_HEADING: case SYNC_ANIMATION: case SYNC_WEAPON_MODE: case INIT_NPC: case DESTROY_NPC:
        case SYNC_ARMOR: case SYNC_WEAPONS: case SYNC_HP: case SYNC_HAND: case SYNC_MAGIC_SETUP: case SYNC_REVIVED:
        case SYNC_PROTECTIONS: case SYNC_TALENTS: case SYNC_BODYSTATE: case SYNC_OVERLAYS: case SYNC_ATTACKS: case SYNC_SPELL_CAST:
            return true;
        }
        return false;
    }

    static const char* WpPhaseName(int p) {
        switch (p) {
        case 0: return "wartet";
        case 1: return "laedt";
        case 2: return "geladen";
        case 3: return "wechselt";
        case 4: return "bereit";
        case 5: return "ruht";   // u38: Auftrag zu Ende, bleibt in seiner Welt
        }
        return "fehler";
    }

    // ===== Host ========================================================================================================

    // u38: "Host" heisst hier das Hauptspiel, das den Weltprozess fuehrt - beim Coop-Host wie beim Client
    struct WpHostState {
        bool on = false;
        WpLink* link = NULL;
        HANDLE proc = NULL, job = NULL;
        DWORD pid = 0;
        int session = 0;
        bool active = false;               // u38: hat einen Auftrag (sonst Leerlauf)
        std::string follow, nick, world;   // Mitspieler, dem der Weltprozess zuschaut, und seine Welt (Datei)
        std::string idleWhy;               // u38: warum er ruht (Meldungen)
        int key = 0;
        int slot = -1;
        long long startMs = 0, timeMs = 0, activeMs = 0, readyMs = 0;
        std::string phase;
        int readySeq = 0;
        int fps = 0;
        bool fig = false;
        unsigned long fed = 0;
        unsigned int img = 0;              // u38: Share-Handle des Weltbilds (Renderer coop8 im Weltprozess), 0 = keins
        int imgW = 0, imgH = 0;
    };
    static WpHostState WpHost;
    static HDESK WpDesk = NULL;                      // u39: eigener Desktop "GothicCoopWelt" (u45 auch fuer WpDialogs)
    // u48: letzter Stand je Spieler und Paketart fuer die Figur im Weltprozess (bis u47 nur INIT_NPC, WpLastInit). Der Mitspieler
    // schickt Ruestung, Waffen, Overlays, Waffenmodus usw. nur bei Aenderung und die laufende Animation nur beim Start (die
    // Schleife alle 5 s) - nach jedem (Neu-)Laden des Weltprozesses fehlte das der Figur. Nutzer nach Test u47: "die Synchronisation
    // der Animationen des Helden hat in der Zuschaueransicht gefehlt". Ersatz-Host u48: die Figur entsteht im Puppen-Modus (gemerkte
    // NPC-Pakete kommen mit), ihre eigene Steh-Animation war blockiert - "laufen keine" bis zur naechsten Schleife.
    static std::map<std::string, std::map<int, json>> WpFigCache;
    static std::map<std::string, int> WpFigWorld;   // Weltschluessel der gemerkten Lage je Spieler

    static void WpFigRemember(const std::string& id, int type, const json& j) {
        if (type == DESTROY_NPC) {
            WpFigCache.erase(id);
            return;
        }
        if (type == SYNC_ATTACKS || type == SYNC_SPELL_CAST || type == SYNC_REVIVED || type == SYNC_TIME)
            return;   // Ereignisse, kein Zustand
        auto& c = WpFigCache[id];
        if (j.contains("wk") && j["wk"].is_number_integer() && j["wk"].get<int>()) {
            const int wk = j["wk"].get<int>();
            int& was = WpFigWorld[id];
            if (was && was != wk) {   // andere Welt: Lage, Blickrichtung, Zustand und Animation gelten nicht mehr
                c.erase(SYNC_POS);
                c.erase(SYNC_HEADING);
                c.erase(SYNC_BODYSTATE);
                c.erase(SYNC_ANIMATION);
            }
            was = wk;
        }
        if (type != SYNC_ANIMATION) {
            c[type] = j;
            return;
        }
        // Animation: nur die laufende Schleife merken ("loop" bzw. der letzte S_-Eintrag der Liste), nicht Angriffe oder Uebergaenge
        std::string loop = j.contains("loop") && j["loop"].is_string() ? j["loop"].get<std::string>() : std::string();
        if (loop.empty() && j.contains("al") && j["al"].is_array())
            for (auto& n : j["al"])
                if (n.is_string() && n.get<std::string>().compare(0, 2, "S_") == 0)
                    loop = n.get<std::string>();
        if (loop.empty())
            return;
        json a;
        a["type"] = SYNC_ANIMATION;
        a["id"] = id;
        if (j.contains("wk"))
            a["wk"] = j["wk"];
        a["a"] = -1;
        a["al"] = json::array();
        a["loop"] = loop;
        c[SYNC_ANIMATION] = a;
    }
    // u38: Vorstart (Nutzer: "die zweite Instanz bereits zu Anfang starten, die sind dann erstmal im Leerlauf, bis dann wirklich
    // einer die eigene Spielwelt verlaesst oder direkt in der anderen Spielwelt durch einen Savegame startet")
    static bool WpAutoOff = false;           // Shift+F12 hat ihn beendet - kein Vorstart bis zum naechsten Shift+F12
    static int WpFails = 0;                  // unerwartet beendet bzw. nicht startbar; ab 3 kein Vorstart mehr (bis Shift+F12)
    static long long WpRetryMs = 0;          // naechster Vorstart fruehestens
    static long long WpSettleMs = 0;         // nach dem Laden erst ruhig werden lassen
    static long long WpLastPulseMs = 0;
    static long long WpCoopOffSinceMs = 0;   // Coop aus seit (beendet den Weltprozess nach 30 s)
    static std::string WpCandidate;          // Mitspieler, der gerade in einer anderen Welt ist (Entprellen)
    static long long WpCandidateSinceMs = 0;
    static long long WpActivateRetryMs = 0;  // Auftrag ging nicht (kein Spielstand ...) - erst dann wieder
    static long long WpNoSaveMs = 0;

    // GothicCoopConfig.json "coopWorldProcessAuto" (Vorgabe true): Weltprozess bei Coop-Beginn vorstarten
    static bool WpAutoConfig() {
        if (CoopConfig.contains("coopWorldProcessAuto") && CoopConfig["coopWorldProcessAuto"].is_boolean())
            return CoopConfig["coopWorldProcessAuto"].get<bool>();
        return true;
    }

    // Mitspieler in einer anderen Welt. fresh: er sendet gerade (laedt er, kommt nichts - z. B. beim gemeinsamen Weltwechsel)
    static bool WpHostPickPartner(std::string& id, std::string& file, std::string& nick, int& key, bool fresh) {
        const int mine = CoopWorldKey();
        const std::string me = MyselfId.ToChar();
        if (!mine)
            return false;
        for (auto& kv : CoopPeerWorlds) {
            if (kv.first == me || !kv.second.key || kv.second.key == mine || kv.second.file.empty())
                continue;
            if (fresh && CurrentMs - kv.second.lastMs > 4000)
                continue;
            id = kv.first;
            file = kv.second.file;
            nick = kv.second.nick.empty() ? kv.first : kv.second.nick;
            key = kv.second.key;
            return true;
        }
        return false;
    }

    // neuester Spielstand des Hosts (SAVEDAT.SAV); Slot 0 nicht (Schnellspeichern)
    static int WpHostNewestSlot() {
        int best = -1;
        FILETIME bt = { 0, 0 };
        for (int s = 1; s <= (int)CoopAddr::saveSlotCount; s++) {
            FILETIME t;
            if (!WpFileTime(CoopSlotDir(s) + "SAVEDAT.SAV", t))
                continue;
            if (best < 0 || CompareFileTime(&t, &bt) > 0) {
                best = s;
                bt = t;
            }
        }
        return best;
    }

    // Slot nach CoopWelt\Saves\savegameN kopieren (vorige Kopie dort vorher leeren - eigener Ordner), dazu die Weltdateien
    // der Zielwelt aus Saves\current, wenn sie neuer sind als die im Slot (der Host war seit dem Speichern dort)
    static bool WpHostPrepareSaves(int slot, const std::string& world, std::string& why, std::string& note) {
        const std::string src = CoopSlotDir(slot);
        const std::string root = WpCopyRoot() + "\\Saves\\";
        const std::string dst = root + std::string(TSaveLoadGameInfo::GetSaveSlotName(slot).ToChar()) + "\\";
        CreateDirectoryA(root.c_str(), NULL);
        CreateDirectoryA((root + "current").c_str(), NULL);
        CreateDirectoryA(dst.c_str(), NULL);
        int removed = 0, copied = 0, failed = 0;
        WpForEachFile(dst, [&](const std::string& n) { if (DeleteFileA((dst + n).c_str())) removed++; });
        WpForEachFile(src, [&](const std::string& n) {
            if (CopyFileA((src + n).c_str(), (dst + n).c_str(), FALSE))
                copied++;
            else
                failed++;
        });
        if (!copied || failed) {
            char b[200];
            _snprintf(b, sizeof(b), "%d Dateien kopiert, %d nicht (Fehler %lu)", copied, failed, GetLastError());
            b[sizeof(b) - 1] = 0;
            why = b;
            return false;
        }
        const std::string cur = CoopSavesDir() + "current\\";
        note.clear();
        // SAVEHDR.SAV = Weltpfad als Text ("NEWWORLD\NEWWORLD.ZEN\n"). Ist die Zielwelt die aktuelle Welt des Slots, steckt der Held in
        // deren Datei - dann nichts aus current darueberlegen (dort laege die Welt ohne Helden)
        std::string slotWorld;
        if (FILE* f = fopen((src + "SAVEHDR.SAV").c_str(), "rb")) {
            char line[260] = "";
            if (fgets(line, sizeof(line), f))
                slotWorld = line;
            fclose(f);
            while (!slotWorld.empty() && (slotWorld[slotWorld.size() - 1] == '\n' || slotWorld[slotWorld.size() - 1] == '\r'))
                slotWorld.erase(slotWorld.size() - 1);
        }
        if (!slotWorld.empty() && CoopWcBase(slotWorld) == CoopWcBase(world)) {
            WpLog("Slot %d (%s) nach %s kopiert: %d Dateien, %d alte entfernt; gespeichert in %s = Zielwelt (mit Held), nichts aus current",
                slot, src.c_str(), dst.c_str(), copied, removed, slotWorld.c_str());
            return true;
        }
        for (const std::string& f : WpWorldFiles(world)) {
            FILETIME tc, ts;
            const bool hc = WpFileTime(cur + f, tc), hs = WpFileTime(dst + f, ts);
            if (hc && (!hs || CompareFileTime(&tc, &ts) > 0) && CopyFileA((cur + f).c_str(), (dst + f).c_str(), FALSE))
                note += (note.empty() ? "" : ", ") + f + " aus current";
            else if (!hc && !hs)
                note += (note.empty() ? "" : ", ") + f + " fehlt (Welt neu)";
        }
        WpLog("Slot %d (%s) nach %s kopiert: %d Dateien, %d alte entfernt%s%s", slot, src.c_str(), dst.c_str(), copied, removed,
            note.empty() ? "" : "; ", note.c_str());
        return true;
    }

    // eigene Kommandozeile ohne den Pfad der Exe
    static std::string WpOwnArgs() {
        const char* c = GetCommandLineA();
        if (*c == '"') {
            c = strchr(c + 1, '"');
            c = c ? c + 1 : "";
        }
        else
            while (*c && *c != ' ')
                c++;
        while (*c == ' ')
            c++;
        return c;
    }

    static void WpHostSend(const json& j) {
        if (WpHost.link && !WpHost.link->broken)
            WpHost.link->outbox.enqueue(j);
    }

    // failed (u38): unerwartet (abgestuerzt, Verbindung weg, Fehler) - Vorstart erst nach 2 min wieder, nach 3 Mal nicht mehr
    static void WpHostStop(const char* why, bool chat, bool failed) {
        if (!WpHost.on)
            return;
        WpHost.on = false;
        if (failed) {
            WpFails++;
            WpRetryMs = CurrentMs + 120000;
        }
        WpLinkClose(WpHost.link);
        WpHost.link = NULL;
        DWORD code = STILL_ACTIVE;
        if (WpHost.proc) {
            GetExitCodeProcess(WpHost.proc, &code);
            if (code == STILL_ACTIVE)
                TerminateProcess(WpHost.proc, 0);
            CloseHandle(WpHost.proc);
            WpHost.proc = NULL;
        }
        if (WpHost.job) {
            CloseHandle(WpHost.job);   // KILL_ON_JOB_CLOSE - falls doch noch etwas laeuft
            WpHost.job = NULL;
        }
        WpLog("beendet (%s; Prozess %lu%s, %lu Pakete weitergereicht)", why, (unsigned long)WpHost.pid,
            code == STILL_ACTIVE ? " beendet" : (" war schon zu, Code " + std::to_string(code)).c_str(), WpHost.fed);
        // u34: Logs des Weltprozesses ins eigene Log (Test u33: beim Partner-Host waren sie nicht erreichbar)
        const std::string sys = WpCopyRoot() + "\\system\\";
        WpHostLogTail(sys + CoopProfile().logBase + "Welt.log", 30);
        if (GetFileAttributesA((sys + "Log_CoopWelt.txt").c_str()) != INVALID_FILE_ATTRIBUTES)
            WpHostLogTail(sys + "Log_CoopWelt.txt", 12);
        else
            WpHostLogTail(sys + "Log.txt", 12);   // Renderer coop6
        if (code == 0xC0000417)
            WpLog("  Code 0xC0000417 = ungueltiger Parameter in der alten C-Laufzeit (msvcr100: Union/VDFS)");
        WpHidePlugins(WpCopyRoot(), "Weltprozess beendet");   // u41: falls er vor dem Umbenennen abbrach
        if (failed)
            WpLog("  unerwartet beendet (%d. Mal) - %s", WpFails, WpFails >= 3 ? "kein Vorstart mehr bis Shift+F12" : "naechster Vorstart in 2 min");
        if (chat)
            ChatLog(string::Combine(CoopTr("Weltprozess beendet: %s", "World process stopped: %s"), string(why)), zCOLOR(255, 200, 80, 255));
        WpHost.active = false;
        WpHost.img = 0;
        WpHost.phase.clear();
    }

    // u38: startet im Leerlauf - Gothic bootet bis ins Hauptmenue und wartet dort; den Auftrag (Spielstand, Welt) bekommt er erst,
    // wenn ein Mitspieler in einer anderen Welt ist (WpHostActivate). Beim Coop-Host wie beim Client.
    // GothicCoopConfig.json "coopWorldProcessVisible" (Vorgabe false): Fenster des Weltprozesses minimiert in der Taskleiste statt versteckt
    static bool WpVisibleConfig() {
        return CoopConfig.contains("coopWorldProcessVisible") && CoopConfig["coopWorldProcessVisible"].is_boolean() &&
            CoopConfig["coopWorldProcessVisible"].get<bool>();
    }

    static void WpHostStart(bool manual) {
        if (IsLoadingLevel)
            return;   // u39: auch im Hauptmenue (ab dem COOP-Menue)
        std::string why;
        WpArchiveCopyLogs(WpCopyRoot(), "voriger Lauf");   // u36: GD3D11 leert sein Log beim Start - vorher archivieren
        if (!WpHostEnsureCopy(why)) {   // u34: eigener Spielordner (Union packt sonst in die Plugin-Dateien des Hauptspiels aus)
            ChatLog(string::Combine(CoopTr("Weltprozess: eigener Spielordner nicht angelegt (%s).", "World process: could not set up its game folder (%s)."),
                string(why.c_str())), zCOLOR(255, 80, 80, 255));
            WpLog("nicht gestartet - Spielordner %s: %s", WpCopyRoot().c_str(), why.c_str());
            WpFails++;
            WpRetryMs = CurrentMs + 120000;
            return;
        }

        WpHost = WpHostState();
        WpHost.session = (int)(GetTickCount() & 0x7FFFFFFF);
        WpLink* l = new WpLink();
        l->server = true;
        const unsigned long pid = GetCurrentProcessId();
        const DWORD mode = PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS;
        l->in = CreateNamedPipeA(WpPipeName(pid, WpHost.session, "von").c_str(), PIPE_ACCESS_INBOUND, mode, 1, 1 << 20, 1 << 20, 0, NULL);
        l->out = CreateNamedPipeA(WpPipeName(pid, WpHost.session, "zu").c_str(), PIPE_ACCESS_OUTBOUND, mode, 1, 1 << 20, 1 << 20, 0, NULL);
        if (l->in == INVALID_HANDLE_VALUE || l->out == INVALID_HANDLE_VALUE) {
            WpLog("nicht gestartet - Pipe nicht angelegt (Fehler %lu)", GetLastError());
            ChatLog(CoopTr("Weltprozess: Verbindung nicht angelegt (Log).", "World process: could not create the connection (see log)."), zCOLOR(255, 80, 80, 255));
            if (l->in != INVALID_HANDLE_VALUE) CloseHandle(l->in);
            if (l->out != INVALID_HANDLE_VALUE) CloseHandle(l->out);
            WpFails++;
            WpRetryMs = CurrentMs + 120000;
            return;
        }
        WpLinkStart(l);

        const std::string dir = WpCopyRoot() + "\\system";   // u34: aus dem eigenen Spielordner (harte Verknuepfung der Exe)
        const std::string exeStr = dir + "\\" + WpExeName();   // u42: dieselbe Exe wie das Hauptspiel (G1 GothicMod.exe)
        const char* exe = exeStr.c_str();
        int w = 960, h = 540;
        if (CoopConfig.contains("coopWorldProcessRes") && CoopConfig["coopWorldProcessRes"].is_array() && CoopConfig["coopWorldProcessRes"].size() == 2 &&
            CoopConfig["coopWorldProcessRes"][0].is_number_integer() && CoopConfig["coopWorldProcessRes"][1].is_number_integer()) {
            w = CoopConfig["coopWorldProcessRes"][0].get<int>();
            h = CoopConfig["coopWorldProcessRes"][1].get<int>();
        }
        const bool visible = WpVisibleConfig();
        // u39: eigener Windows-Desktop "GothicCoopWelt" (CreateDesktop - Test mit dem Ersatz-Host: auf dem Desktop des Spielers kein
        // Fenster, kein Union-Splash, kein Taskleisten-Eintrag; Direct3D zeichnet dort weiter, die geteilte Textur oeffnet das Hauptspiel
        // wie sonst). Bis u37 minimiert in der Taskleiste; versteckt allein liess Union seinen Splash in der Bildmitte stehen und das
        // Spielfenster war die ersten 10 s sichtbar. Der Desktop bleibt bis zum Ende des Hauptspiels offen (Handle nie geschlossen).
        HDESK& desk = WpDesk;
        static bool deskTried = false;
        if (!visible && !deskTried) {
            deskTried = true;
            desk = CreateDesktopA("GothicCoopWelt", NULL, NULL, 0, GENERIC_ALL, NULL);
            if (!desk)
                WpLog("eigener Desktop nicht angelegt (Fehler %lu) - Weltprozess auf dem Desktop des Spielers, Fenster versteckt", GetLastError());
        }
        char deskName[] = "WinSta0\\GothicCoopWelt";
        char cmd[2048];
        _snprintf(cmd, sizeof(cmd), "\"%s\" -coopwelt:%lu:%lX:%d%s -zwindow -zres:%d,%d,32 -znosound -znomusic %s", exe, pid,
            (unsigned long)(ULONG_PTR)hWndApp, WpHost.session, visible ? " -coopweltzeigen" : "", w, h, WpOwnArgs().c_str());
        cmd[sizeof(cmd) - 1] = 0;
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        // nicht in den Vordergrund (das Hauptspiel behaelt Fokus und Vollbild); u39: versteckt bzw. auf dem eigenen Desktop
        si.wShowWindow = visible ? SW_SHOWMINNOACTIVE : SW_HIDE;
        if (!visible && desk)
            si.lpDesktop = deskName;
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        if (!CreateProcessA(exe, cmd, NULL, NULL, FALSE, CREATE_SUSPENDED | BELOW_NORMAL_PRIORITY_CLASS, NULL, dir.c_str(), &si, &pi)) {
            WpLog("nicht gestartet - CreateProcess %s fehlgeschlagen (Fehler %lu)", exe, GetLastError());
            ChatLog(CoopTr("Weltprozess: Gothic konnte nicht ein zweites Mal gestartet werden (Log).", "World process: could not start a second Gothic (see log)."), zCOLOR(255, 80, 80, 255));
            WpLinkClose(l);
            WpFails++;
            WpRetryMs = CurrentMs + 120000;
            return;
        }
        HANDLE job = CreateJobObjectA(NULL, NULL);
        bool inJob = false;
        if (job) {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION li;
            ZeroMemory(&li, sizeof(li));
            li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;   // endet mit dem Host, auch nach einem Absturz
            SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof(li));
            inJob = AssignProcessToJobObject(job, pi.hProcess) != FALSE;
        }
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);

        WpHost.on = true;
        WpHost.link = l;
        WpHost.proc = pi.hProcess;
        WpHost.job = job;
        WpHost.pid = pi.dwProcessId;
        WpHost.startMs = CurrentMs;
        WpHost.idleWhy = CoopTr("startet", "starting");
        WpLog("gestartet (Leerlauf, %s, Fenster %s): Prozess %lu (%s), %dx%d - %s", manual ? "Shift+F12" : player ? "Vorstart im Spiel" : "Vorstart im COOP-Menue",
            visible ? "minimiert" : desk ? "auf eigenem Desktop GothicCoopWelt" : "versteckt", (unsigned long)pi.dwProcessId,
            inJob ? "im Job, endet mit dem Hauptspiel" : "nicht im Job - endet beim Beenden des Hauptspiels", w, h, cmd);
        if (manual)   // u39: der Vorstart laeuft still im Hintergrund (nur Log)
            ChatLog(CoopTr("Weltprozess gestartet - wartet im Hintergrund, bis ein Mitspieler in einer anderen Welt ist (Shift+F12 beendet ihn).",
                "World process started - waits in the background until a player is in another world (Shift+F12 stops it)."), zCOLOR(160, 230, 160, 255));
    }

    static void WpHostSendTime() {
        if (!ogame)
            return;
        WpHost.timeMs = CurrentMs;
        int d = 0, hh = 0, mm = 0;
        ogame->GetTime(d, hh, mm);
        json j;
        j["wp"] = "time";
        j["d"] = d;
        j["h"] = hh;
        j["m"] = mm;
        WpHostSend(j);
    }

    // u38: Auftrag - Mitspieler id ist in der Welt file. Ruht der Weltprozess (Hauptmenue oder alte Welt), bekommt er den neuesten
    // Spielstand und laedt ihn (im Spiel ueber Read_Savegame); ist er noch mit einem Auftrag beschaeftigt (laedt, wechselt), nur die
    // neue Welt - seine Spielstand-Dateien werden dann nicht angefasst.
    static void WpHostActivate(const std::string& id, const std::string& file, const std::string& nick, int key) {
        const bool idle = WpHost.phase.empty() || WpHost.phase == "wartet" || WpHost.phase == "ruht";
        int slot = WpHost.slot;
        if (idle || slot < 0) {
            slot = WpHostNewestSlot();
            if (slot < 0) {
                if (CurrentMs > WpNoSaveMs + 120000) {
                    WpNoSaveMs = CurrentMs;
                    ChatLog(CoopTr("Weltprozess: kein Spielstand vorhanden - bitte einmal speichern, dann zeigt er die andere Welt.",
                        "World process: no savegame yet - please save once, then it shows the other world."), zCOLOR(255, 200, 80, 255));
                    WpLog("kein Auftrag - kein Spielstand in %s", CoopSavesDir().c_str());
                }
                WpActivateRetryMs = CurrentMs + 10000;
                return;
            }
            std::string why, note;
            if (!WpHostPrepareSaves(slot, file, why, note)) {
                ChatLog(string::Combine(CoopTr("Weltprozess: Spielstand nicht vorbereitet (%s).", "World process: could not prepare the savegame (%s)."),
                    string(why.c_str())), zCOLOR(255, 80, 80, 255));
                WpLog("kein Auftrag - Spielstand nicht vorbereitet: %s", why.c_str());
                WpActivateRetryMs = CurrentMs + 60000;
                return;
            }
        }
        WpHost.active = true;
        WpHost.follow = id;
        WpHost.nick = nick;
        WpHost.world = file;
        WpHost.key = key;
        WpHost.slot = slot;
        WpHost.activeMs = CurrentMs;
        WpHost.readyMs = 0;
        WpHost.idleWhy.clear();
        json j;
        j["wp"] = "start";
        j["slot"] = slot;
        j["w"] = file;
        j["id"] = id;
        j["nick"] = nick;
        j["reload"] = idle ? 1 : 0;
        WpHostSend(j);
        WpHostSendTime();
        WpLog("Auftrag: %s (%s) ist in %s - %s", nick.c_str(), id.c_str(), file.c_str(),
            idle ? ("Slot " + std::to_string(slot) + " laden, dann dorthin").c_str() : "noch beschaeftigt - nur die neue Welt");
        ChatLog(string::Combine(CoopTr("Weltprozess l\xE4" "dt %s - gleich zeigt Shift+F5 dort %s.", "World process is loading %s - soon Shift+F5 shows %s there."),
            string(CoopWorldShow(file).c_str()), string(nick.c_str())), zCOLOR(160, 230, 160, 255));
    }

    // u38: Auftrag zu Ende (Mitspieler in meiner Welt, getrennt) - der Weltprozess bleibt in seiner Welt und ruht
    static void WpHostDeactivate(const char* why) {
        if (!WpHost.active)
            return;
        WpHost.active = false;
        WpHost.idleWhy = why;
        json j;
        j["wp"] = "idle";
        WpHostSend(j);
        WpLog("ruht (%s) - zeigte %s in %s, %lu Pakete weitergereicht", why, WpHost.nick.c_str(), WpHost.world.c_str(), WpHost.fed);
    }

    // Plugin.cpp: Shift+F12 - beenden (kein Vorstart mehr) bzw. starten
    void CoopWpHostToggle() {
        if (WpHost.on) {
            WpAutoOff = true;
            WpHostStop(CoopTr("Shift+F12 - startet erst wieder mit Shift+F12", "Shift+F12 - starts again only with Shift+F12"), true, false);
            return;
        }
        WpAutoOff = false;
        WpFails = 0;
        WpRetryMs = 0;
        if (!ServerThread && !ClientThread) {
            ChatLog(CoopTr("Weltprozess: erst Coop starten (Host oder Client) - er startet dann von selbst.",
                "World process: start coop first (host or client) - it then starts by itself."), zCOLOR(255, 200, 80, 255));
            return;
        }
        WpHostStart(true);
    }

    // --- u46 (P87): NPCs der gespiegelten Welt fuer das Zuschau-Fenster ----------------------------------------------------
    //
    // Nutzer (Test u44): "die NPCs, Kaempfe etc. mit der zweiten Spielinstanz synchronisieren, damit die Sachen in der
    // Zuschaueransicht richtig angezeigt werden". Bis u45 rechnete der Weltprozess die NPCs seiner Welt selbst (eigene KI) und bekam
    // nur die Figur des Mitspielers. Jetzt fuehrt sie der, der in jener Welt ist:
    //  - Der Host sendet die NPCs um die Figuren seiner Welt ohnehin (BroadcastNpcs, mit "wk"). Ein Client ohne Host in seiner Welt
    //    sendet jetzt die NPCs um seinen Helden ebenso (WpMirrorNpcs, Pakete mit "wpn" - der Host wendet sie nie selbst an) und
    //    meldet die Tode dort (CoopFarSync.cpp).
    //  - Das Hauptspiel verwirft NPC-Pakete aus einer anderen Welt weiter (P65), reicht sie aber seinem Weltprozess durch, wenn der
    //    genau jene Welt zeigt, und merkt sich je NPC das letzte Paket jeder Art (WpMirrorCache) und die gemeldeten Tode - ein frisch
    //    bereiter Weltprozess bekommt damit sofort den ganzen Stand.
    //  - Der Weltprozess fuehrt diese NPCs wie ein Client als Puppen (CoopWpMirroring: Hooks.cpp sperrt eigene Kampf-KI und eigene
    //    Animationen, Treffer auf den unsichtbaren Helden und die Figur gelten nicht; Tode aus NPC_DEATH). Kommen 10 s keine
    //    NPC-Pakete (Mitspieler mit alter Fassung, "coopWorldProcessMirror": false), rechnet er sie wie bisher selbst.

    // GothicCoopConfig.json "coopWorldProcessMirror" (Vorgabe true): Client ohne Host sendet seine NPCs fuer den Weltprozess des Hosts
    static bool WpMirrorConfig() {
        return !(CoopConfig.contains("coopWorldProcessMirror") && CoopConfig["coopWorldProcessMirror"].is_boolean() &&
            !CoopConfig["coopWorldProcessMirror"].get<bool>());
    }

    bool CoopWpMirrorSendOn() {
        return WpMirrorConfig() && ClientThread && !ServerThread && CoopClientState == 2 && CoopClientAlone() && !IsCoopPaused &&
            !(MyselfId == "_player_");
    }

    static std::map<string, LocalNpc*> WpMirrorNpcs;   // Client ohne Host: gesendete NPCs um den eigenen Helden

    static void WpMirrorClear() {
        for (auto& kv : WpMirrorNpcs)
            delete kv.second;
        WpMirrorNpcs.clear();
    }

    // Plugin.cpp Game_Loop (Hauptspiel), nach den eigenen Paketen
    void CoopWpMirrorSendPulse() {
        static bool was = false;
        static long long listMs = 0;
        const bool on = CoopWpMirrorSendOn() && player && ogame && !IsLoadingLevel;
        if (on != was) {
            was = on;
            WpLog(on ? "Client ohne Host in meiner Welt: sende die NPCs um mich fuer den Weltprozess des Hosts (u46)"
                : "NPC-Spiegel aus (Host wieder in meiner Welt, Laden oder kein Coop)");
            if (!on)
                WpMirrorClear();
            listMs = 0;
        }
        if (!on)
            return;
        if (CurrentMs > listMs + 500) {   // wie UpdateVisibleNpc beim Host
            listMs = CurrentMs;
            std::map<string, LocalNpc*> next;
            for (oCNpc* npc : GetVisibleNpcs()) {
                auto it = NpcToUniqueNameList.find(npc);
                if (it == NpcToUniqueNameList.end() || next.count(it->second))
                    continue;
                auto old = WpMirrorNpcs.find(it->second);
                if (old != WpMirrorNpcs.end() && old->second->npc == npc) {
                    next[it->second] = old->second;
                    WpMirrorNpcs.erase(old);
                }
                else {
                    LocalNpc* l = new LocalNpc(npc, it->second);
                    l->mirrorSend = true;
                    next[it->second] = l;
                }
            }
            WpMirrorClear();   // nicht mehr in der Naehe
            WpMirrorNpcs.swap(next);
        }
        for (auto& kv : WpMirrorNpcs) {
            kv.second->Pulse();
            kv.second->PackUpdate();
        }
    }

    // Weltschluessel -> NPC -> Paketart -> letztes Paket; Welt (Datei, gross) -> gemeldete Tote
    static std::map<int, std::map<std::string, std::map<int, json>>> WpMirrorCache;
    static std::map<std::string, std::set<std::string>> WpMirrorDead;
    static unsigned long WpMirrorFed = 0;

    static bool WpMirrorType(int t) {
        switch (t) {
        case SYNC_POS: case SYNC_HEADING: case SYNC_ANIMATION: case SYNC_WEAPON_MODE: case INIT_NPC: case DESTROY_NPC: case SYNC_ATTACKS:
        case SYNC_HP: case SYNC_HAND: case SYNC_MAGIC_SETUP: case SYNC_SPELL_CAST: case SYNC_BODYSTATE: case SYNC_OVERLAYS:
            return true;
        }
        return false;
    }

    static bool WpReadyToFeed() {
        return WpHost.on && WpHost.active && WpHost.link && WpHost.link->connected >= 2 && !WpHost.link->broken && WpHost.phase == "bereit";
    }

    static void WpMirrorFeed(const json& j, int type, const std::string& id) {
        if (type == SYNC_NPC_DEATH) {
            const std::string w = WpUpper(WpStr(j, "w"));
            if (w.empty() || w == WpUpper(CoopWorldName()))
                return;   // eigene Welt: wendet das Hauptspiel selbst an
            std::set<std::string>& dead = WpMirrorDead[w];
            if (j.contains("d") && j["d"].is_array())
                for (auto& d : j["d"])
                    if (d.is_string() && dead.size() < 5000)
                        dead.insert(d.get<std::string>());
            if (WpReadyToFeed() && w == WpUpper(WpHost.world)) {
                WpHost.link->outbox.enqueue(j);
                WpHost.fed++;
                WpMirrorFed++;
            }
            return;
        }
        if (!WpMirrorType(type) || !j.contains("wk") || !j["wk"].is_number_integer())
            return;
        const int wk = j["wk"].get<int>();
        if (!wk || wk == CoopWorldKey())
            return;   // eigene Welt: wendet das Hauptspiel selbst an
        auto& perNpc = WpMirrorCache[wk];
        if (perNpc.size() > 1500 && !perNpc.count(id))
            perNpc.clear();   // Obergrenze
        if (type == DESTROY_NPC)
            perNpc.erase(id);
        else if (type != SYNC_ATTACKS && type != SYNC_SPELL_CAST)   // einmalige Ereignisse nicht nachreichen
            perNpc[id][type] = j;
        if (WpReadyToFeed() && wk == WpHost.key) {
            WpHost.link->outbox.enqueue(j);
            WpHost.fed++;
            WpMirrorFed++;
        }
    }

    // beim "bereit" des Weltprozesses: gemerkten Stand der NPCs seiner Welt und ihre Tode nachreichen
    static int WpMirrorReplay() {
        int n = 0;
        static const int order[] = { INIT_NPC, SYNC_POS, SYNC_HEADING, SYNC_WEAPON_MODE, SYNC_HAND, SYNC_MAGIC_SETUP, SYNC_HP,
            SYNC_BODYSTATE, SYNC_OVERLAYS, SYNC_ANIMATION };
        auto c = WpMirrorCache.find(WpHost.key);
        if (c != WpMirrorCache.end())
            for (auto& npc : c->second)
                for (int t : order) {
                    auto p = npc.second.find(t);
                    if (p != npc.second.end()) {
                        WpHostSend(p->second);
                        n++;
                    }
                }
        auto d = WpMirrorDead.find(WpUpper(WpHost.world));
        if (d != WpMirrorDead.end() && !d->second.empty()) {
            json j;
            j["type"] = SYNC_NPC_DEATH;
            j["id"] = "HOST";
            j["w"] = WpHost.world;
            j["all"] = 1;
            j["d"] = std::vector<std::string>(d->second.begin(), d->second.end());
            WpHostSend(j);
            n++;
        }
        return n;
    }

    // PacketProcessor.cpp (Host: Paket eines Clients; u38 Client: Paket vom Host), vor der Verarbeitung - Figuren der Mitspieler,
    // u46 auch NPCs und Tode aus der Welt, die der Weltprozess zeigt
    void CoopWpHostFeed(const json& j) {
        if (!j.contains("type") || !j["type"].is_number_integer() || !j.contains("id") || !j["id"].is_string())
            return;
        const int type = j["type"].get<int>();
        const std::string id = j["id"].get<std::string>();
        if (type == SYNC_NPC_DEATH || !IsCoopPlayer(id)) {
            WpMirrorFeed(j, type, id);
            return;
        }
        if (id == std::string(MyselfId.ToChar()))
            return;
        if (WpForwardType(type))
            WpFigRemember(id, type, j);   // u48
        if (!WpHost.on || !WpHost.active || !WpHost.link || WpHost.link->connected < 2 || WpHost.link->broken || WpHost.phase != "bereit" ||
            !WpForwardType(type))
            return;
        WpHost.link->outbox.enqueue(j);
        WpHost.fed++;
    }

    // UnionHooks.cpp _Game_Exit
    void CoopWpHostShutdown() {
        WpHostStop("Spiel beendet", false, false);
    }

    // u38: CoopCamera.cpp (Shift+F5) - Bild des Weltprozesses, wenn er bereit ist; sonst sein Stand in Worten
    bool CoopWpImage(unsigned int& handle, std::string& nick, std::string& world, std::string& state) {
        handle = 0;
        nick = WpHost.nick;
        world = WpHost.world;
        if (!WpHost.on) {
            state = WpAutoOff ? CoopTr("aus (Shift+F12 startet ihn)", "off (Shift+F12 starts it)")
                : WpFails >= 3 ? CoopTr("aus - mehrmals abgebrochen (Log; Shift+F12 versucht es neu)", "off - failed several times (log; Shift+F12 retries)")
                : !WpAutoConfig() ? CoopTr("aus (coopWorldProcessAuto)", "off (coopWorldProcessAuto)")
                : CoopTr("startet gleich", "starting soon");
            return false;
        }
        if (!WpHost.active) {
            state = !WpCandidate.empty() ? CoopTr("startet gleich", "starting soon")
                : WpHost.phase.empty() ? CoopTr("startet", "starting") : std::string(CoopTr("ruht", "idle"));
            return false;
        }
        if (WpHost.phase != "bereit") {
            state = WpHost.phase == "laedt" ? CoopTr("l\xE4" "dt den Spielstand", "loading the savegame")
                : WpHost.phase == "wechselt" || WpHost.phase == "geladen" ? CoopTr("wechselt die Welt", "changing the world")
                : CoopTr("bereitet vor", "preparing");
            return false;
        }
        if (!WpHost.fig) {
            state = CoopTr("wartet auf die Figur", "waiting for the figure");
            return false;
        }
        if (!WpHost.img) {
            state = CoopTr("kein Bild (Renderer coop8 im Weltprozess?)", "no picture (renderer coop8 in the world process?)");
            return false;
        }
        handle = WpHost.img;
        state = CoopTr("bereit", "ready");
        return true;
    }

    static void WpHostReceive(const json& j) {
        const std::string op = WpStr(j, "wp");
        if (op == "state") {
            const std::string s = WpStr(j, "s");
            const std::string w = WpStr(j, "w");
            WpHost.fps = WpInt(j, "fps", 0);
            const bool fig = WpInt(j, "fig", 0) != 0;
            // u38: Share-Handle des Weltbilds (int64 in BSON, sonst kaeme ein Handle ueber 0x7FFFFFFF negativ an)
            const unsigned int img = j.contains("img") && j["img"].is_number_integer() ? (unsigned int)j["img"].get<long long>() : 0;
            if (img != WpHost.img) {
                WpLog("Weltbild %s (%dx%d)", img ? ("geteilt, Handle " + std::to_string(img)).c_str() : "nicht geteilt (Renderer vor coop8?)",
                    WpInt(j, "iw", 0), WpInt(j, "ih", 0));
                WpHost.img = img;
                WpHost.imgW = WpInt(j, "iw", 0);
                WpHost.imgH = WpInt(j, "ih", 0);
            }
            if (s != WpHost.phase || fig != WpHost.fig) {
                WpLog("meldet %s (Welt %s, %d fps, Figur %s, Pakete %d)", s.c_str(), w.c_str(), WpHost.fps, fig ? "da" : "fehlt",
                    WpInt(j, "pk", 0));
                if (s == "bereit" && fig && !WpHost.readyMs && WpHost.active) {
                    WpHost.readyMs = CurrentMs;
                    WpLog("bereit nach %lld s Auftrag (%lld s seit dem Start des Weltprozesses)", (CurrentMs - WpHost.activeMs) / 1000,
                        (CurrentMs - WpHost.startMs) / 1000);
                    ChatLog(string::Combine(CoopTr("Weltprozess bereit: %s in %s - Shift+F5 zeigt das Bild.", "World process ready: %s in %s - Shift+F5 shows the picture."),
                        string(WpHost.nick.c_str()), string(CoopWorldShow(w).c_str())), zCOLOR(160, 230, 160, 255));
                }
                WpHost.phase = s;
                WpHost.fig = fig;
            }
            const int seq = WpInt(j, "seq", 0);
            if (s == "bereit" && seq != WpHost.readySeq) {
                WpHost.readySeq = seq;
                // Figuren anlegen (RemoteNpc legt sie nur mit INIT_NPC an), u48 danach ihr ganzer gemerkter Stand
                static const int figOrder[] = { INIT_NPC, SYNC_ARMOR, SYNC_WEAPONS, SYNC_OVERLAYS, SYNC_TALENTS, SYNC_PROTECTIONS,
                    SYNC_MAGIC_SETUP, SYNC_WEAPON_MODE, SYNC_HAND, SYNC_HP, SYNC_BODYSTATE, SYNC_POS, SYNC_HEADING, SYNC_ANIMATION };
                int n = 0, nf = 0;
                std::string loops;
                for (auto& kv : WpFigCache) {
                    if (!kv.second.count(INIT_NPC))
                        continue;
                    n++;
                    for (int t : figOrder) {
                        auto p = kv.second.find(t);
                        if (p == kv.second.end())
                            continue;
                        WpHostSend(p->second);
                        nf++;
                        if (t == SYNC_ANIMATION)
                            loops += " " + kv.first + "=" + p->second.value("loop", std::string());
                    }
                }
                const int m = WpMirrorReplay();   // u46: Stand der NPCs jener Welt und ihre Tode
                WpLog("bereit (Folge %d) - %d Figur(en) mit %d Paketen (Animation:%s) und %d gemerkte NPC-Pakete nachgereicht (bisher %lu "
                    "NPC-Pakete weitergereicht)", seq, n, nf, loops.empty() ? " keine" : loops.c_str(), m, WpMirrorFed);
            }
        }
        else if (op == "fehler") {
            const std::string why = WpStr(j, "why");
            WpLog("Fehler im Weltprozess: %s", why.c_str());
            WpHostStop(why.c_str(), true, true);
        }
    }

    // u43 (P85): Test u41 - der erste Start nach dem Spielstart hing zweimal (Ersatz-Host und echtes Spiel) ohne eine Zeile Log,
    // weder GT noch Renderer; der naechste Start lief jedes Mal. Vor dem Beenden: je Faden Befehlszeiger und Ruecksprung-Kandidaten
    // (Werte auf dem Stapel, die in ein Modul zeigen) als Modul+Abstand ins Log. Die Faeden bleiben angehalten (danach Terminate).
    static void WpDumpThreads(DWORD pid, HANDLE proc) {
        struct Mod { DWORD base, size; std::string name; };
        std::vector<Mod> mods;
        for (int tries = 0; tries < 5 && mods.empty(); tries++) {   // ERROR_BAD_LENGTH, solange der Prozess Module laedt
            HANDLE s = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
            if (s == INVALID_HANDLE_VALUE) {
                Sleep(50);
                continue;
            }
            MODULEENTRY32 me;
            me.dwSize = sizeof(me);
            for (BOOL ok = Module32First(s, &me); ok; ok = Module32Next(s, &me))
                mods.push_back({ (DWORD)(ULONG_PTR)me.modBaseAddr, me.modBaseSize, me.szModule });
            CloseHandle(s);
        }
        auto where = [&](DWORD a) -> std::string {
            for (auto& m : mods)
                if (a >= m.base && a < m.base + m.size) {
                    char b[96];
                    _snprintf(b, sizeof(b), "%s+%X", m.name.c_str(), a - m.base);
                    b[sizeof(b) - 1] = 0;
                    return b;
                }
            return std::string();
        };
        std::string modList;
        for (auto& m : mods)
            modList += (modList.empty() ? "" : ", ") + m.name;
        WpLog("haengt: %d Module geladen: %s", (int)mods.size(), modList.size() > 700 ? (modList.substr(0, 700) + " ...").c_str() : modList.c_str());
        HANDLE s = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (s == INVALID_HANDLE_VALUE)
            return;
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        int shown = 0;
        for (BOOL ok = Thread32First(s, &te); ok && shown < 16; ok = Thread32Next(s, &te)) {
            if (te.th32OwnerProcessID != pid)
                continue;
            HANDLE t = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT, FALSE, te.th32ThreadID);
            if (!t)
                continue;
            SuspendThread(t);
            CONTEXT c;
            ZeroMemory(&c, sizeof(c));
            c.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
            if (GetThreadContext(t, &c)) {
                DWORD stack[1024];
                SIZE_T got = 0;
                ReadProcessMemory(proc, (LPCVOID)(ULONG_PTR)c.Esp, stack, sizeof(stack), &got);
                std::string rets;
                int n = 0;
                for (SIZE_T i = 0; i < got / 4 && n < 12; i++) {
                    const std::string w = where(stack[i]);
                    if (w.empty() || rets.find(w) != std::string::npos)
                        continue;
                    rets += (rets.empty() ? "" : " < ") + w;
                    n++;
                }
                const std::string e = where(c.Eip);
                WpLog("haengt: Faden %lu bei %s - Stapel: %s", (unsigned long)te.th32ThreadID, e.empty() ? "?" : e.c_str(),
                    rets.empty() ? "-" : rets.c_str());
                shown++;
            }
            CloseHandle(t);
        }
        CloseHandle(s);
    }

    // u45 (P85): Meldungsboxen des Weltprozesses. Er laeuft auf einem unsichtbaren Desktop - eine Box (Test u41: der erste Start hing
    // in SoftModalMessageBox, noch vor GT) klickt dort niemand weg. Solange er nicht verbunden ist (und danach, falls GTs eigener
    // Mitschnitt nicht greift), sucht GT jede Sekunde nach Dialogfenstern (#32770) des Prozesses, schreibt Titel, Texte und Knoepfe
    // ins Log und drueckt den vorgegebenen Knopf (DM_GETDEFID, sonst OK) - so laeuft der Start weiter, und der Text steht im Log.
    struct WpDlgScan {
        DWORD pid;
        std::vector<HWND> found;
    };

    static BOOL CALLBACK WpDlgEnum(HWND h, LPARAM lp) {
        WpDlgScan* s = (WpDlgScan*)lp;
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        char cls[64] = "";
        GetClassNameA(h, cls, sizeof(cls));
        if (pid == s->pid && !strcmp(cls, "#32770"))
            s->found.push_back(h);
        return TRUE;
    }

    static BOOL CALLBACK WpDlgChild(HWND c, LPARAM lp) {
        std::string* out = (std::string*)lp;
        char cls[64] = "", txt[700] = "";
        GetClassNameA(c, cls, sizeof(cls));
        GetWindowTextA(c, txt, sizeof(txt));
        if (txt[0]) {
            for (char* p = txt; *p; p++)
                if (*p == '\r' || *p == '\n')
                    *p = ' ';
            *out += std::string(out->empty() ? "" : " | ") + (!_stricmp(cls, "Button") ? "[" : "") + txt + (!_stricmp(cls, "Button") ? "]" : "");
        }
        return TRUE;
    }

    static BOOL CALLBACK WpDlgFirstButton(HWND c, LPARAM lp) {
        char cls[64] = "";
        GetClassNameA(c, cls, sizeof(cls));
        if (!_stricmp(cls, "Button")) {
            *(HWND*)lp = c;
            return FALSE;
        }
        return TRUE;
    }

    // u49: Dialogfenster (#32770) eines Prozesses ins Log und bestaetigen - Weltprozess und Testinstanz (CoopAutoTest.cpp). Test u49:
    // Unions SystemPack zeigt beim ersten Start einer neuen Exe-Kopie "First start!" ("Game system settings have been updated: DEP has
    // been changed ... UI scaling has been changed ... Please restart the game!") mit einem einzigen Knopf OK der ID 2. u45 schickte
    // IDOK (ID 1, gibt es dort nicht) - die Box blieb stehen; das war P85. Jetzt der Standardknopf, sonst der erste Knopf des Fensters;
    // Gothic laeuft danach weiter (im Test ohne Neustart bis ins Spiel).
    static void WpConfirmDialogs(HDESK desk, DWORD pid, std::set<HWND>& handled, const char* who, long long startMs) {
        WpDlgScan s;
        s.pid = pid;
        if (desk)
            EnumDesktopWindows(desk, WpDlgEnum, (LPARAM)&s);
        else
            EnumWindows(WpDlgEnum, (LPARAM)&s);
        for (HWND h : s.found) {
            if (handled.count(h))
                continue;
            handled.insert(h);
            char title[200] = "";
            GetWindowTextA(h, title, sizeof(title));
            for (char* p = title; *p; p++)
                if (*p == '\r' || *p == '\n')
                    *p = ' ';
            std::string body;
            EnumChildWindows(h, WpDlgChild, (LPARAM)&body);
            DWORD_PTR def = 0;
            if (!SendMessageTimeoutA(h, DM_GETDEFID, 0, 0, SMTO_ABORTIFHUNG, 500, &def))
                def = 0;
            int id = HIWORD(def) == DC_HASDEFID ? LOWORD(def) : IDOK;
            HWND btn = GetDlgItem(h, id);
            if (!btn) {
                EnumChildWindows(h, WpDlgFirstButton, (LPARAM)&btn);
                if (btn)
                    id = GetDlgCtrlID(btn);
            }
            char btnText[64] = "";
            if (btn)
                GetWindowTextA(btn, btnText, sizeof(btnText));
            WpLog("Meldungsbox %s (%lld s nach dem Start): \"%s\" - %s - druecke %s (ID %d)", who, (GetCurrentMs() - startMs) / 1000,
                title, body.empty() ? "(ohne Text)" : body.c_str(), btnText[0] ? btnText : "OK", id);
            PostMessageA(h, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)btn);
        }
    }

    static void WpDialogs() {
        static long long next = 0;
        static std::set<HWND> handled;
        if (!WpHost.on || !WpHost.pid || CurrentMs < next)
            return;
        next = CurrentMs + 1000;
        WpConfirmDialogs(WpDesk, WpHost.pid, handled, "im Weltprozess", WpHost.startMs);
    }

    static bool WpPrestartAllowed() {
        return !WpAutoOff && WpAutoConfig() && WpFails < 3 && CurrentMs >= WpRetryMs;
    }

    // Prozess und Verbindung pruefen, Meldungen lesen. false = laeuft nicht (mehr) bzw. noch nicht verbunden
    static bool WpHostMaintain() {
        if (!WpHost.on)
            return false;
        if (WpHost.proc && WaitForSingleObject(WpHost.proc, 0) == WAIT_OBJECT_0) {
            DWORD code = 1;
            GetExitCodeProcess(WpHost.proc, &code);
            if (code == 0) {   // u38: Fenster von Hand geschlossen (WpWndProc endet mit 0) - wie Shift+F12, kein Vorstart mehr
                WpAutoOff = true;
                WpHostStop(CoopTr("Fenster geschlossen - startet erst wieder mit Shift+F12", "window closed - starts again only with Shift+F12"), true, false);
            }
            else
                WpHostStop(CoopTr("abgest\xFC" "rzt", "crashed"), true, true);
            return false;
        }
        WpLink* l = WpHost.link;
        if (!l || (l->broken && l->connected >= 2)) {
            WpHostStop(CoopTr("Verbindung getrennt", "connection lost"), true, true);
            return false;
        }
        WpDialogs();   // u45 (P85): Meldungsbox auf dem unsichtbaren Desktop - Text ins Log, Standardknopf druecken
        if (l->connected < 2) {
            // u43 (P85): ein Start verbindet sonst nach ~12 s. Haengt er 60 s: Faeden ins Log, beenden, nach 3 s noch einmal (die
            // ersten zwei Mal je Spielsitzung ohne Chat und ohne als Fehler zu zaehlen); bis u42 erst nach 3 min, dann 2 min Pause
            if (CurrentMs > WpHost.startMs + 60000) {
                static int hangs = 0;
                hangs++;
                WpDumpThreads(WpHost.pid, WpHost.proc);
                WpHostStop(CoopTr("meldet sich nicht (60 s)", "no answer (60 s)"), hangs > 2, true);
                if (hangs <= 2) {
                    WpFails--;
                    WpRetryMs = CurrentMs + 3000;
                    WpLog("hing beim Start (%d. Mal in dieser Sitzung) - neuer Versuch in 3 s", hangs);
                }
            }
            return false;
        }
        for (int n = 0; n < 200 && !l->inbox.isEmpty(); n++)
            WpHostReceive(l->inbox.dequeue());
        return WpHost.on;
    }

    // u39: UnionHooks.cpp _Game_MenuLoop (Hauptmenue) - Nutzer: "das Fenster soll bereits im COOP-Menue starten". Vorstart, sobald
    // ein COOP-Menue offen ist (MENU_COOP...) oder Coop laeuft; die Verbindung wird gehalten. Ohne beides 60 s: beenden.
    void CoopWpHostMenuPulse() {
        if (player)
            return;   // im Spiel: CoopWpHostPulse (Game_Loop)
        CurrentMs = GetCurrentMs();
        WpLastPulseMs = 0;   // nach dem Laden im Spiel erst ruhig werden lassen
        zCMenu* m = zCMenu::GetActive();
        const bool coopMenu = m && WpUpper(std::string(m->name.ToChar())).compare(0, 9, "MENU_COOP") == 0;
        // u44: Pruefschalter -coopweltvorstart (Kommandozeile des Hauptspiels): Vorstart schon im Hauptmenue wie im COOP-Menue -
        // fuer versteckte Pruefungen ohne Menue-Bedienung (G1-Weltprozess, haengender erster Start P85)
        static const bool testStart = strstr(GetCommandLineA(), "-coopweltvorstart") != NULL;
        const bool wanted = coopMenu || ServerThread || ClientThread || testStart;
        if (wanted)
            WpCoopOffSinceMs = 0;
        else if (!WpCoopOffSinceMs)
            WpCoopOffSinceMs = CurrentMs;
        if (!WpHost.on) {
            if (wanted && WpPrestartAllowed())
                WpHostStart(false);
            return;
        }
        if (!wanted && CurrentMs > WpCoopOffSinceMs + 60000) {
            WpHostStop(CoopTr("kein Coop", "no coop"), false, false);
            return;
        }
        WpHostMaintain();
    }

    // Plugin.cpp Game_Loop (Hauptspiel - Coop-Host wie Client)
    void CoopWpHostPulse() {
        // u38: Vorstart - sobald Coop laeuft und das Spiel nach dem Laden ruhig ist (Game_Loop ruht waehrend des Ladens)
        if (!WpLastPulseMs || CurrentMs - WpLastPulseMs > 2000)
            WpSettleMs = CurrentMs + 5000;
        WpLastPulseMs = CurrentMs;
        const bool coop = ServerThread || (ClientThread && CoopClientState == 2);
        if (coop)
            WpCoopOffSinceMs = 0;
        else if (!WpCoopOffSinceMs)
            WpCoopOffSinceMs = CurrentMs;
        if (!WpHost.on) {
            if (coop && WpPrestartAllowed() && CurrentMs >= WpSettleMs && player && ogame && !IsLoadingLevel && !IsSavingGame &&
                !(MyselfId == "_player_"))
                WpHostStart(false);
            return;
        }
        if (!coop && CurrentMs > WpCoopOffSinceMs + 30000) {
            WpHostStop(CoopTr("Coop beendet", "coop ended"), true, false);
            return;
        }
        if (!WpHostMaintain())
            return;
        // u38: Auftrag geben bzw. beenden. Mit Auftrag: Mitspieler verfolgen (Welt gewechselt -> mit; in meine Welt gekommen oder
        // weg -> ruhen). Ohne: ist ein Mitspieler (sendend, also nicht gerade beim Laden) in einer anderen Welt, nach 4 s Auftrag -
        // beim gemeinsamen Weltwechsel (u32) erst nach 20 s und nie, waehrend ich selbst gleich nachkomme.
        if (WpHost.active) {
            auto it = CoopPeerWorlds.find(WpHost.follow);
            if (it == CoopPeerWorlds.end())
                WpHostDeactivate(CoopTr("Mitspieler nicht mehr verbunden", "player disconnected"));
            else if (it->second.key && CoopWorldKey() && it->second.key == CoopWorldKey())
                WpHostDeactivate(CoopTr("Mitspieler ist in deiner Welt", "player is in your world"));
            else if (it->second.key && it->second.key != WpHost.key && !it->second.file.empty()) {
                WpHost.key = it->second.key;
                WpHost.world = it->second.file;
                WpHost.readyMs = 0;
                json j;
                j["wp"] = "world";
                j["w"] = WpHost.world;
                WpHostSend(j);
                WpLog("%s ist jetzt in %s - Weltprozess wechselt mit", WpHost.follow.c_str(), WpHost.world.c_str());
            }
        }
        if (!WpHost.active) {
            std::string id, file, nick;
            int key = 0;
            if (WpHostPickPartner(id, file, nick, key, true)) {
                if (id != WpCandidate) {
                    WpCandidate = id;
                    WpCandidateSinceMs = CurrentMs;
                }
                const long long delay = CoopRuleWorldTogether() ? 20000 : 4000;
                if (!CoopWcFollowNow.on && CurrentMs >= WpCandidateSinceMs + delay && CurrentMs >= WpActivateRetryMs)
                    WpHostActivate(id, file, nick, key);
            }
            else
                WpCandidate.clear();
        }
        if (CurrentMs > WpHost.timeMs + 5000)   // Uhrzeit wie an die Clients (SYNC_TIME: Stunde, Minute)
            WpHostSendTime();
        if (screen && WpHost.active && !(WpHost.phase == "bereit" && WpHost.fig)) {   // Zeile unter der Fassung, solange er vorbereitet
            char b[200];
            _snprintf(b, sizeof(b), CoopTr("Weltprozess: %s - %s, %s, %d fps", "World process: %s - %s, %s, %d fps"), WpHost.phase.empty() ? "..." : WpHost.phase.c_str(),
                CoopWorldShow(WpHost.world).c_str(), WpHost.nick.c_str(), WpHost.fps);
            b[sizeof(b) - 1] = 0;
            auto font = screen->GetFontName();
            auto color = screen->fontColor;
            screen->SetFont(zSTRING("Font_Old_10_White_Hi.TGA"));
            screen->SetFontColor(zCOLOR(160, 230, 160, 255));
            screen->Print(50, 180, zSTRING(b));
            screen->SetFont(font);
            screen->SetFontColor(color);
        }
    }

    // ===== Weltprozess =================================================================================================

    struct WpWorldState {
        WpLink* link = NULL;
        unsigned long hostPid = 0;
        HWND hostWnd = NULL;
        int session = 0;
        bool dirOk = false;
        bool haveStart = false;
        int slot = -1;
        std::string target, follow, nick;
        int phase = 0;              // WpPhaseName
        int changeTries = 0;
        int readySeq = 0;
        long long phaseMs = 0, stateMs = 0, fpsMs = 0, lastFrameMs = 0, logMs = 0;
        int frames = 0, fps = 0;
        unsigned long packets = 0;
        bool timeSet = false;
        bool menuSeen = false;
        std::string title;
        bool wantIdle = false;        // u38: Auftrag zu Ende - nach dem Laden/Wechseln ruhen
        bool reloadPending = false;   // u38: neuer Auftrag im Spiel - Spielstand neu laden (Read_Savegame)
        long long mirrorMs = 0;       // u46: letztes NPC-Paket der gespiegelten Welt (0 = noch keins)
        unsigned long mirrorPackets = 0;
        long long namesMs = 0;        // u46: NPC-Namen dieser Welt zuletzt vergeben (BuildGlobalNpcList)
        unsigned int img = 0;         // u38: Share-Handle des Weltbilds (Renderer coop8)
        int imgW = 0, imgH = 0;
    };
    static WpWorldState WpW;

    // u46 (P87): der Weltprozess spiegelt gerade die Welt eines Mitspielers - ihre NPCs sind Puppen dessen, der dort ist (Hooks.cpp:
    // keine eigene Kampf-KI, keine eigenen Animationen, Treffer auf Held/Figur zaehlen nicht; CoopFarSync.cpp: Tode anwenden).
    // Nur solange NPC-Pakete kommen (10 s) - sonst rechnet er die NPCs wie bis u45 selbst.
    bool CoopWpMirroring() {
        return CoopWpIsWorld() && WpW.phase == 4 && !IsLoadingLevel && WpW.mirrorMs && CurrentMs < WpW.mirrorMs + 10000;
    }
    static WNDPROC WpOldWndProc = NULL;

    static void WpWorldSend(const json& j) {
        if (WpW.link && !WpW.link->broken)
            WpW.link->outbox.enqueue(j);
    }

    static void WpWorldFail(const std::string& why) {
        WpLog("FEHLER: %s - Weltprozess endet", why.c_str());
        json j;
        j["wp"] = "fehler";
        j["why"] = why;
        WpWorldSend(j);
        Sleep(400);   // Schreibfaden die Meldung schicken lassen
        TerminateProcess(GetCurrentProcess(), 1);
    }

    static RemoteNpc* WpFollowNpc() {
        if (WpW.follow.empty())
            return NULL;
        const string key(WpW.follow.c_str());
        auto it = SyncNpcs.find(key);
        return it == SyncNpcs.end() ? NULL : it->second;
    }

    static void WpWorldState_Send(bool force) {
        if (!force && CurrentMs < WpW.stateMs + 2000)
            return;
        WpW.stateMs = CurrentMs;
        RemoteNpc* r = WpFollowNpc();
        json j;
        j["wp"] = "state";
        j["s"] = WpPhaseName(WpW.phase);
        j["w"] = (player && !IsLoadingLevel) ? CoopWorldName() : std::string();
        j["fps"] = WpW.fps;
        j["fig"] = (r && r->npc && r->hasModel) ? 1 : 0;
        j["pk"] = (int)WpW.packets;
        j["seq"] = WpW.readySeq;
        j["img"] = (long long)WpW.img;   // u38
        j["iw"] = WpW.imgW;
        j["ih"] = WpW.imgH;
        if (player && !IsLoadingLevel) {   // u37: Lage des (unsichtbaren) Helden - Diagnose im Host-Log, Ersatz-Host im Test
            const zVEC3 hp = player->GetPositionWorld();
            j["hx"] = (int)hp.n[0];
            j["hy"] = (int)hp.n[1];
            j["hz"] = (int)hp.n[2];
        }
        WpWorldSend(j);
    }

    static void WpSetPhase(int p) {
        if (p == WpW.phase)
            return;
        WpLog("%s -> %s (Welt %s)", WpPhaseName(WpW.phase), WpPhaseName(p), (player && !IsLoadingLevel) ? CoopWorldName().c_str() : "-");
        WpW.phase = p;
        WpW.phaseMs = CurrentMs;
        if (p == 4)
            WpW.readySeq++;
        WpWorldState_Send(true);
    }

    static void WpSetTitle() {
        if (!hWndApp)
            return;
        std::string t = "Gothic II - Coop-Weltprozess";
        if (!WpW.target.empty())
            t += ": " + CoopWorldShow(WpW.target);
        if (!WpW.nick.empty())
            t += std::string(CoopTr(" (zeigt ", " (showing ")) + WpW.nick + ")";
        if (t != WpW.title) {
            WpW.title = t;
            SetWindowTextA(hWndApp, t.c_str());
        }
    }

    // Fenster schliessen = sofort enden (Gothic schriebe sonst am Ende von CGameManager::Run die Gothic.ini des Hosts neu)
    static LRESULT CALLBACK WpWndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
        if (m == WM_CLOSE || (m == WM_SYSCOMMAND && (w & 0xFFF0) == SC_CLOSE)) {
            WpLog("Fenster geschlossen - endet sofort (ohne Gothic.ini zu schreiben)");
            TerminateProcess(GetCurrentProcess(), 0);
            return 0;
        }
        return CallWindowProcA(WpOldWndProc, h, m, w, l);
    }

    // u39: Nutzer "am besten subtiler, dass man es nicht sieht, als Hintergrundprozess" - das Fenster ist versteckt (kein Eintrag
    // in der Taskleiste; Gothic und GD3D11 zeichnen weiter, das Bild kommt ueber die geteilte Textur). Mit "-coopweltzeigen"
    // (GothicCoopConfig.json "coopWorldProcessVisible": true beim Hauptspiel) minimiert wie bis u37.
    static bool WpWindowVisible() {
        return strstr(GetCommandLineA(), "-coopweltzeigen") != NULL;
    }

    static void WpWindowSetup(bool first) {
        if (!hWndApp)
            return;
        if (!WpOldWndProc)
            WpOldWndProc = (WNDPROC)SetWindowLongA(hWndApp, GWL_WNDPROC, (LONG)(LONG_PTR)WpWndProc);
        const bool visible = WpWindowVisible();
        ShowWindow(hWndApp, visible ? SW_SHOWMINNOACTIVE : SW_HIDE);
        if (WpW.hostWnd && IsWindow(WpW.hostWnd))
            SetForegroundWindow(WpW.hostWnd);   // Fokus (und Vollbild) zurueck an das Hauptspiel
        if (first)
            WpLog("Fenster %p %s, Fokus an das Hauptspiel %p", (void*)hWndApp, visible ? "minimiert" : "versteckt (Hintergrundprozess)", (void*)WpW.hostWnd);
    }

    // u39: jede Sekunde - sichtbare eigene Fenster (Spielfenster nach einem Groessenwechsel, Unions Splash) wieder verstecken. Auf dem
    // eigenen Desktop (Normalfall) sieht sie ohnehin niemand; das hier ist der Rueckfall, wenn GT keinen Desktop anlegen konnte.
    static BOOL CALLBACK WpHideOwnWindow(HWND h, LPARAM count) {
        DWORD pid = 0;
        GetWindowThreadProcessId(h, &pid);
        if (pid == GetCurrentProcessId() && !GetWindow(h, GW_OWNER) && IsWindowVisible(h)) {
            ShowWindowAsync(h, SW_HIDE);   // asynchron: der Splash gehoert ggf. einem anderen Faden
            (*(int*)count)++;
        }
        return TRUE;
    }

    static void WpWindowKeepHidden() {
        static long long next = 0;
        if (WpWindowVisible() || CurrentMs < next)
            return;
        next = CurrentMs + 1000;
        int n = 0;
        EnumWindows(WpHideOwnWindow, (LPARAM)&n);
        static int logged = 0;
        if (n && logged++ < 5)
            WpLog("%d sichtbare Fenster des Weltprozesses versteckt", n);
    }

    // u34: der Weltprozess laeuft aus <Spiel>\CoopWelt\system - sein Spielordner ist dann CoopWelt, und Gothics Spielstaende
    // (DIR_SAVEGAMES "\Saves\" aus Paths.d, relativ zum Spielordner) liegen in CoopWelt\Saves. Ohne das wird nicht geladen.
    static bool WpCheckOwnRoot() {
        const std::string root = CoopGameRoot();   // u36: <Spiel>_CoopWelt (bzw. CoopWelt, wenn das Spiel direkt im Laufwerk liegt)
        const std::string leaf = WpUpper(root.substr(root.find_last_of("\\/") == std::string::npos ? 0 : root.find_last_of("\\/") + 1));
        const bool rootOk = leaf == "COOPWELT" || (leaf.size() > 9 && leaf.compare(leaf.size() - 9, 9, "_COOPWELT") == 0);
        zFILE* d = zoptions ? zoptions->GetDir(DIR_SAVEGAMES) : NULL;
        const std::string full = d ? std::string(d->GetFullPath().ToChar()) : std::string();
        const bool savesOk = !full.empty() && WpUpper(full).find("COOPWELT\\SAVES") != std::string::npos;
        WpLog("Spielordner %s, Spielstaende %s - %s", root.c_str(), full.c_str(), rootOk && savesOk ? "eigener Ordner" : "NICHT der eigene Ordner");
        return rootOk && savesOk;
    }

    // UnionHooks.cpp _Game_Init (nur im Weltprozess)
    void CoopWpWorldInit() {
        const char* a = strstr(GetCommandLineA(), "-coopwelt:");
        unsigned long hwnd = 0;
        if (!a || sscanf(a + 10, "%lu:%lX:%d", &WpW.hostPid, &hwnd, &WpW.session) != 3) {
            WpLog("Kommandozeile nicht lesbar: %s", GetCommandLineA());
            TerminateProcess(GetCurrentProcess(), 1);
            return;
        }
        WpW.hostWnd = (HWND)(ULONG_PTR)hwnd;
        WpLog("startet (Host-Prozess %lu, Sitzung %d, %s)", WpW.hostPid, WpW.session, CoopVersionName().c_str());
        WpW.dirOk = WpCheckOwnRoot();
        if (WpW.dirOk)
            WpHidePlugins(CoopGameRoot(), "Weltprozess, Plugins geladen");   // u41 (P83): nur im eigenen Ordner
        WpWindowSetup(true);
        WpLink* l = new WpLink();
        const std::string zu = WpPipeName(WpW.hostPid, WpW.session, "zu"), von = WpPipeName(WpW.hostPid, WpW.session, "von");
        for (int i = 0; i < 150 && l->in == INVALID_HANDLE_VALUE; i++) {
            l->in = CreateFileA(zu.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (l->in == INVALID_HANDLE_VALUE)
                Sleep(100);
        }
        for (int i = 0; i < 150 && l->in != INVALID_HANDLE_VALUE && l->out == INVALID_HANDLE_VALUE; i++) {
            l->out = CreateFileA(von.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (l->out == INVALID_HANDLE_VALUE)
                Sleep(100);
        }
        if (l->in == INVALID_HANDLE_VALUE || l->out == INVALID_HANDLE_VALUE) {
            WpLog("keine Verbindung zum Host (%s, Fehler %lu) - endet", zu.c_str(), GetLastError());
            TerminateProcess(GetCurrentProcess(), 1);
            return;
        }
        l->connected = 2;
        WpW.link = l;
        WpLinkStart(l);
        WpLog("mit dem Host verbunden");
        if (!WpW.dirOk)
            WpWorldFail("nicht aus dem eigenen Spielordner CoopWelt gestartet (Log des Weltprozesses)");
        WpWorldState_Send(true);
    }

    // Auftraege des Hosts; Spielpakete nur, wenn die Welt bereit ist (sonst verworfen - INIT_NPC kommt nach "bereit" neu)
    static void WpWorldInbox(bool inGame) {
        if (!WpW.link)
            return;
        if (WpW.link->broken) {
            WpLog("Verbindung zum Host weg - endet");
            TerminateProcess(GetCurrentProcess(), 0);
            return;
        }
        for (int n = 0; n < 3000 && !WpW.link->inbox.isEmpty(); n++) {
            json j = WpW.link->inbox.dequeue();
            const std::string op = WpStr(j, "wp");
            if (op == "start") {
                WpW.haveStart = true;
                WpW.wantIdle = false;
                WpW.slot = WpInt(j, "slot", -1);
                WpW.target = WpStr(j, "w");
                WpW.follow = WpStr(j, "id");
                WpW.nick = WpStr(j, "nick");
                const bool reload = WpInt(j, "reload", 0) != 0;
                WpLog("Auftrag: %s%s, %s (%s) zeigen", reload ? ("Slot " + std::to_string(WpW.slot) + " laden, dann ").c_str() : "",
                    WpW.target.c_str(), WpW.nick.c_str(), WpW.follow.c_str());
                WpSetTitle();
                // u38: im Spiel (vorher geruht) neu laden; noch beschaeftigt -> nur die neue Welt; im Hauptmenue laedt CoopWpWorldMenuPulse
                if (inGame && reload)
                    WpW.reloadPending = true;
                else if (inGame && (WpW.phase == 4 || WpW.phase == 5))
                    WpSetPhase(2);
            }
            else if (op == "idle") {   // u38: Auftrag zu Ende - in der Welt bleiben und ruhen (im Hauptmenue: nicht laden)
                WpLog("Auftrag zu Ende - ruht");
                WpW.wantIdle = true;
                WpW.reloadPending = false;
                if (WpW.phase == 0)
                    WpW.haveStart = false;
                else if (inGame && (WpW.phase == 2 || WpW.phase == 4))
                    WpSetPhase(5);
            }
            else if (op == "world") {
                WpW.target = WpStr(j, "w");
                WpLog("Mitspieler jetzt in %s", WpW.target.c_str());
                WpSetTitle();
                if (inGame && WpW.phase == 4)
                    WpSetPhase(2);   // Weltwechsel im naechsten Bild
            }
            else if (op == "time") {
                if (inGame && WpW.phase == 4 && ogame) {
                    const int dd = WpInt(j, "d", -1), hh = WpInt(j, "h", -1), mm = WpInt(j, "m", -1);
                    int d0 = 0, h0 = 0, m0 = 0;
                    ogame->GetTime(d0, h0, m0);
                    const int diff = abs((hh * 60 + mm) - (h0 * 60 + m0));
                    const bool dayDiff = dd >= 0 && dd != d0;   // u51: Spieltag (WpHostSendTime schickt ihn seit u38 mit)
                    if (hh >= 0 && mm >= 0 && (dayDiff || (diff > 2 && diff < 24 * 60 - 2))) {
                        CoopApplyHostTime(dd, hh, mm, "Weltprozess");
                        if (!WpW.timeSet || (diff > 30 && diff < 24 * 60 - 30))
                            rtnMan->RestartRoutines();   // Tagesablaeufe zur neuen Zeit
                        WpLog("Uhrzeit vom Host Tag %d %02d:%02d (hier Tag %d %02d:%02d)", dd, hh, mm, d0, h0, m0);
                    }
                    WpW.timeSet = true;
                }
            }
            else if (op == "stop") {
                WpLog("Host beendet den Weltprozess");
                TerminateProcess(GetCurrentProcess(), 0);
            }
            else if (op.empty() && inGame && WpW.phase == 4 && player && !IsLoadingLevel && j.contains("type")) {
                // u46: NPC-Paket bzw. Todesmeldung der gespiegelten Welt - ab jetzt Puppen (CoopWpMirroring); Namen vorher vergeben
                const int type = j["type"].is_number_integer() ? j["type"].get<int>() : -1;
                if (type == SYNC_NPC_DEATH || !IsCoopPlayer(WpStr(j, "id"))) {
                    if (!WpW.mirrorMs)
                        WpLog("erstes NPC-Paket der gespiegelten Welt - NPCs hier sind ab jetzt Puppen dessen, der dort ist (u46)");
                    WpW.mirrorMs = CurrentMs;
                    WpW.mirrorPackets++;
                    if (CurrentMs > WpW.namesMs + 1000) {
                        WpW.namesMs = CurrentMs;
                        BuildGlobalNpcList();
                    }
                }
                ENetEvent ev;
                ZeroMemory(&ev, sizeof(ev));
                ProcessCoopPacket(j, ev);   // wie beim Client: RemoteNpc der Figur (nie SYNC_PLAYER_NAME - braucht ev.peer), u46 auch der NPCs
                WpW.packets++;
            }
        }
    }

    // Bildrate begrenzen (Vorgabe 30, GothicCoopConfig.json "coopWorldProcessFps") - der Weltprozess teilt sich CPU und Grafikkarte.
    // u38: im Leerlauf (Hauptmenue ohne Auftrag, ruht) 5 Bilder/s.
    static void WpFrameLimit() {
        static int target = -1;
        if (target < 0) {
            target = 30;
            if (CoopConfig.contains("coopWorldProcessFps") && CoopConfig["coopWorldProcessFps"].is_number_integer())
                target = CoopConfig["coopWorldProcessFps"].get<int>();
            if (target < 5)
                target = 5;
        }
        const bool idle = WpW.phase == 5 || (WpW.phase == 0 && !WpW.haveStart);
        const long long frameMs = 1000 / (idle ? 5 : target);
        long long now = GetCurrentMs();
        if (WpW.lastFrameMs && now - WpW.lastFrameMs < frameMs)
            Sleep((DWORD)(frameMs - (now - WpW.lastFrameMs)));
        WpW.lastFrameMs = GetCurrentMs();
        WpW.frames++;
        if (WpW.lastFrameMs >= WpW.fpsMs + 1000) {
            WpW.fps = (int)(WpW.frames * 1000 / (WpW.lastFrameMs - WpW.fpsMs + 1));
            WpW.frames = 0;
            WpW.fpsMs = WpW.lastFrameMs;
        }
    }

    // UnionHooks.cpp _Game_MenuLoop (nur im Weltprozess): Slot laden, sobald das Hauptmenue offen ist und der Auftrag da ist
    void CoopWpWorldMenuPulse() {
        CurrentMs = GetCurrentMs();
        PluginState = "Weltprozess-Menue";
        if (!WpW.menuSeen) {
            WpW.menuSeen = true;
            WpWindowSetup(false);   // falls das Hauptmenue das Fenster wieder nach vorn geholt hat
        }
        WpWorldInbox(false);
        if (WpW.phase == 0 && WpW.haveStart && WpW.dirOk && !player) {
            zCMenu* menu = zCMenu::GetActive();
            if (menu && gameMan && gameMan->menu_load_savegame && WpW.slot >= 0) {
                if (gameMan->savegameManager)
                    gameMan->savegameManager->Reinit();   // Slot-Infos aus CoopWelt\Saves (der Slot kam nach dem Start)
                gameMan->menu_load_savegame->m_selSlot = WpW.slot;
                WpSetPhase(1);
                WpLog("schliesse Menue %s mit SAVEGAME_LOAD (Slot %d)", menu->name.ToChar(), WpW.slot);
                menu->ForceSelAction(SEL_ACTION_CLOSE, zSTRING(CoopProfileIsOthello() ? "OTHELLO_SAVEGAME_LOAD" : "SAVEGAME_LOAD"), NULL);
            }
        }
        if (WpW.phase == 1 && CurrentMs > WpW.phaseMs + 120000 && !player)
            WpWorldFail("Spielstand nach 2 min nicht geladen");
        WpWindowKeepHidden();   // u39
        WpWorldState_Send(false);
        WpFrameLimit();
    }

    // UnionHooks.cpp _LoadBegin (nur im Weltprozess)
    void CoopWpWorldLoadBegin() {
        CoopPartnerMarkerWorldGone();
        IsLoadingLevel = true;
        CoopWorldsLoadBegin();
        Myself = NULL;
    }

    // UnionHooks.cpp _LoadEnd / Plugin.cpp Game_LoadEnd_SaveGame (nur im Weltprozess) - wie LoadEnd: Figuren der Spieler behalten,
    // alle anderen Zuordnungen der alten Welt vergessen
    void CoopWpWorldLoadEnd(bool fromSave) {
        std::map<string, RemoteNpc*> keep;
        for (auto& pn : PlayerNpcs)
            if (SyncNpcs.count(pn.second))
                keep[pn.second] = SyncNpcs[pn.second];
        SyncNpcs.clear();
        for (auto& k : keep)
            SyncNpcs[k.first] = k.second;
        BroadcastNpcs.clear();
        UniqueNameToNpcList.clear();
        NpcToUniqueNameList.clear();
        NamesCounter.clear();
        NpcToFirstRoutineWp.clear();
        KilledByPlayerNpcNames.clear();
        WpW.mirrorMs = 0;           // u46: Spiegel der neuen Welt beginnt neu (Namen beim ersten NPC-Paket)
        WpW.namesMs = 0;
        CoopFarSyncWorldLoaded();   // u46: gemeldete Tode gelten fuer die alte Welt
        if (fromSave && ogame) {   // im Spielstand mitgespeicherte Figuren (wie Game_LoadEnd_SaveGame)
            auto coopFriendInstanceId = GetFriendDefaultInstanceId();
            auto* list = CoopWorldNpcListFirst(ogame->GetGameWorld());
            while (list) {
                auto npc = list->data;
                list = list->next;
                if (npc && npc->GetInstance() == coopFriendInstanceId && npc->GetAttribute(NPC_ATR_STRENGTH) == COOP_MAGIC_NUMBER)
                    CoopDeleteNpc(ogame->spawnman, npc);
            }
        }
        auto totWp = ogame && ogame->GetWorld() ? ogame->GetWorld()->wayNet->GetWaypoint("TOT") : NULL;
        CurrentWorldTOTPosition = totWp ? &totWp->GetPositionWorld() : NULL;
        IsLoadingLevel = false;
        CoopRemoveStrayHeroes("Weltprozess geladen");   // u39 (P81): Held des Mitspielers aus der getauschten Weltdatei (CoopPlace.cpp)
        // u51 (Durchsicht u50, Befund 5): wie LoadEnd im Hauptspiel (P94) - der Weltprozess wechselt selbst die Welt
        // (TriggerChangeLevel in CoopWpWorldLoop), sein Held kaeme sonst mit dem Bodenpolygon der verworfenen Welt an
        CoopRefloorAfterLoad("Weltprozess geladen");
        CoopWorldsLoadEnd();
        CurrentMs = GetCurrentMs();
        WpLog("geladen: %s (%s)", CoopWorldName().c_str(), fromSave ? "Spielstand" : "Weltwechsel");
        if (WpW.phase == 1 || WpW.phase == 3 || WpW.phase == 4)
            WpSetPhase(2);
    }

    // Held des Weltprozesses: unsichtbar, ohne Kollision/Physik, 30 m ueber dem Mitspieler; Kamera auf dessen Figur
    // u48: alle 3 s, wenn sich etwas getan hat - laufende Animationen der Figur und die Starts seit dem letzten Bericht (Hooks.cpp,
    // CoopWpFigAni: nachgespielt = Kennung 1337 aus RemoteNpc::UpdateAnimation, eigene = Gothic selbst, blockiert = Puppen-Regel)
    static void WpFigAniReport() {
        static long long nextMs = 0;
        static std::string lastActive;
        static int lines = 0;
        if (CurrentMs < nextMs || lines >= 400)
            return;
        nextMs = CurrentMs + 3000;
        RemoteNpc* r = WpFollowNpc();
        zCModel* m = r && r->npc && r->hasModel ? r->npc->GetModel() : NULL;
        if (!m)
            return;
        std::string active;
        for (int i = 0; i < m->numActiveAnis && i < zMDL_MAX_ANIS_PARALLEL; i++) {
            zCModelAniActive* c = m->aniChannels[i];
            if (c && c->protoAni) {
                if (!active.empty())
                    active += ", ";
                active += c->protoAni->aniName.ToChar();
            }
        }
        if (active == lastActive && !CoopWpFigAni.replay && !CoopWpFigAni.own)
            return;
        lastActive = active;
        lines++;
        WpLog("Figur-Animationen %s: laufen %s | seit dem letzten Bericht nachgespielt %u, eigene %u (davon blockiert %u%s%s), Puppen %s",
            WpW.follow.c_str(), active.empty() ? "keine" : active.c_str(), CoopWpFigAni.replay, CoopWpFigAni.own, CoopWpFigAni.blocked,
            CoopWpFigAni.blocked ? ", zuletzt " : "", CoopWpFigAni.blocked ? CoopWpFigAni.lastBlocked.c_str() : "",
            CoopWpMirroring() ? "ja" : "nein");
        CoopWpFigAni = CoopWpFigAniCount();
    }

    static void WpGhost() {
        if (player->showVisual)
            player->showVisual = FALSE;
        if (player->collDetectionStatic || player->collDetectionDynamic)
            player->SetCollDet(FALSE);
        if (player->physicsEnabled)
            player->SetPhysicsEnabled(FALSE);
        // unverwundbar (Fallschaden beim Abwaertsschweben): direkt ins Skriptfeld flags (+0x1B4) - oCNpc::SetFlag (0x007309C0)
        // verknuepft mit UND statt ODER und loeschte damit alle anderen Bits
        if (!(player->variousFlags & NPC_FLAG_IMMORTAL))
            player->variousFlags |= NPC_FLAG_IMMORTAL;
        RemoteNpc* r = WpFollowNpc();
        if (r && r->lastPositionFromServer) {
            zVEC3 t = *r->lastPositionFromServer;
            t.n[1] += 3000.f;
            if ((player->GetPositionWorld() - t).LengthApprox() > 100.f)
                player->SetPositionWorld(t);
        }
        oCNpc* fig = (r && r->npc && r->hasModel) ? r->npc : NULL;
        zCAICamera* cam = zCAICamera::GetCurrent();
        zCVob* want = fig ? (zCVob*)fig : (zCVob*)player;
        if (cam && cam->target != want) {
            cam->SetTarget(want);
            WpLog("Kamera folgt %s", fig ? WpW.follow.c_str() : "dem eigenen (unsichtbaren) Helden - Figur noch nicht da");
        }
    }

    // u38: Weltbild fuer das Zuschau-Fenster des Hauptspiels teilen (Renderer coop8, GDX_CoopFrameShare) - nur wenn bereit
    typedef unsigned int(__cdecl* WpGdxFrameShareFn)(int enabled, int* w, int* h);
    static void WpFrameShare(bool on) {
        static bool looked = false;
        static WpGdxFrameShareFn fn = NULL;
        if (!looked) {
            if (!on)
                return;   // erst beim ersten Bedarf suchen (der Renderer ist dann sicher geladen)
            looked = true;
#if ENGINE == Engine_G2A
            static const char* mods[] = { "g2a_avx2.dll", "g2a_avx.dll", "g2a.dll" };
#else
            static const char* mods[] = { "g1_avx2.dll", "g1_avx.dll", "g1.dll", "g1a.dll" };   // u42: G1 (wie CoopCamera.cpp)
#endif
            bool any = false;
            for (const char* m : mods)
                if (HMODULE h = GetModuleHandleA(m)) {
                    any = true;
                    fn = (WpGdxFrameShareFn)GetProcAddress(h, "GDX_CoopFrameShare");
                    WpLog("Renderer %s %s", m, fn ? "mit GDX_CoopFrameShare - Bild fuer das Zuschau-Fenster des Hauptspiels"
                        : "ohne GDX_CoopFrameShare (vor coop8) - kein Bild fuer Shift+F5");
                    break;
                }
            if (!any)
                WpLog("kein GD3D11-Renderer - kein Bild fuer Shift+F5");
        }
        if (!fn)
            return;
        int w = 0, h = 0;
        const unsigned int img = fn(on ? 1 : 0, &w, &h);
        if (img != WpW.img || w != WpW.imgW || h != WpW.imgH) {
            WpW.img = img;
            WpW.imgW = w;
            WpW.imgH = h;
            WpLog("Weltbild geteilt: Handle %08X, %dx%d", img, w, h);
            WpWorldState_Send(true);
        }
    }

    static void WpOverlay() {
        if (!screen)
            return;
        RemoteNpc* r = WpFollowNpc();
        const bool fig = r && r->npc && r->hasModel;
        std::string l1 = std::string(CoopTr("Coop-Weltprozess: ", "Coop world process: ")) + CoopWorldShow(WpW.target.empty() ? CoopWorldName() : WpW.target);
        std::string l2;
        if (WpW.phase != 4)
            l2 = std::string(CoopTr("Stand: ", "State: ")) + WpPhaseName(WpW.phase);
        else if (!fig)
            l2 = std::string(CoopTr("warte auf ", "waiting for ")) + WpW.nick + " ...";
        else
            l2 = std::string(CoopTr("zeigt ", "showing ")) + WpW.nick;
        char b[48];
        _snprintf(b, sizeof(b), "  -  %d fps", WpW.fps);
        b[sizeof(b) - 1] = 0;
        l2 += b;
        auto font = screen->GetFontName();
        auto color = screen->fontColor;
        screen->SetFont(zSTRING("Font_Old_10_White_Hi.TGA"));
        screen->SetFontColor(zCOLOR(255, 230, 150, 255));
        screen->Print(50, 50, zSTRING(l1.c_str()));
        screen->SetFontColor(zCOLOR(255, 255, 255, 255));
        screen->Print(50, 50 + screen->FontY() + 20, zSTRING(l2.c_str()));
        screen->SetFont(font);
        screen->SetFontColor(color);
    }

    // UnionHooks.cpp _Game_Loop (nur im Weltprozess) - statt Game_Loop: kein Netz, keine Coop-Module, nur Figur, Held, Kamera
    void CoopWpWorldLoop() {
        PluginState = "Weltprozess";
        if (IsLoadingLevel)
            return;
        CurrentMs = GetCurrentMs();
        WpWorldInbox(true);
        if (!player || !ogame)
            return;
        if (WpW.reloadPending) {   // u38: neuer Auftrag nach dem Ruhen - frischer Spielstand des Hauptspiels
            WpW.reloadPending = false;
            WpFrameShare(false);
            WpSetPhase(1);
            WpLog("lade Slot %d neu (im Spiel)", WpW.slot);
            if (ogame->savegameManager)
                ogame->savegameManager->Reinit();   // Slot-Infos neu - das Hauptspiel hat den Slot eben kopiert
            gameMan->Read_Savegame(WpW.slot);
            return;   // laedt synchron; LoadEnd setzt "geladen"
        }
        if (WpW.phase == 2 && WpW.wantIdle)
            WpSetPhase(5);
        if (WpW.phase == 2) {
            if (WpW.target.empty() || CoopWcInTarget(WpW.target)) {
                WpW.changeTries = 0;
                WpSetPhase(4);
            }
            else if (WpW.changeTries >= 3)
                WpWorldFail("Weltwechsel nach " + WpW.target + " klappt nicht");
            else {
                WpW.changeTries++;
                WpSetPhase(3);
                WpLog("Weltwechsel %s -> %s (Versuch %d)", CoopWorldName().c_str(), WpW.target.c_str(), WpW.changeTries);
                ogame->TriggerChangeLevel(zSTRING(WpW.target.c_str()), zSTRING(""));   // ohne Startpunkt: Gothics Start der Welt
            }
        }
        else if (WpW.phase == 3 && CurrentMs > WpW.phaseMs + 90000)
            WpWorldFail("Weltwechsel nach " + WpW.target + " nach 90 s nicht fertig");
        if (WpW.phase == 4) {
            PluginState = "Weltprozess-Figuren";
            std::vector<string> gone;
            for (auto const& kv : SyncNpcs) {
                if (!kv.second) {
                    gone.push_back(kv.first);
                    continue;
                }
                kv.second->Update();
                if (kv.second->destroyed)
                    gone.push_back(kv.first);
            }
            for (auto const& k : gone)
                SyncNpcs.erase(k);
            CoopFarSyncMirrorPulse();   // u46: gemeldete Tode der gespiegelten Welt nachtragen
            WpFigAniReport();           // u48
            PluginState = "Weltprozess-Held";
            WpGhost();
            if (CurrentMs > WpW.logMs + 30000) {
                WpW.logMs = CurrentMs;
                RemoteNpc* r = WpFollowNpc();
                const zVEC3 p = r && r->lastPositionFromServer ? *r->lastPositionFromServer : zVEC3(0, 0, 0);
                WpLog("%s: %lu Pakete (%lu NPC-Pakete, NPCs %s, %d gefuehrt), Figur %s, Lage %.0f %.0f %.0f, %d fps", CoopWorldName().c_str(),
                    WpW.packets, WpW.mirrorPackets, CoopWpMirroring() ? "Puppen" : "eigene KI", (int)SyncNpcs.size() - (int)PlayerNpcs.size(),
                    (r && r->npc && r->hasModel) ? "da" : "fehlt", p.n[0], p.n[1], p.n[2], WpW.fps);
            }
        }
        WpFrameShare(WpW.phase == 4);   // u38: Bild fuer Shift+F5 im Hauptspiel
        WpWindowKeepHidden();
        WpSetTitle();
        WpOverlay();
        WpWorldState_Send(false);
        WpFrameLimit();
    }

#else   // nicht G2A: Weltprozess noch nicht freigegeben

    void CoopWpHostToggle() {
        ChatLog(CoopTr("Weltprozess: in dieser Gothic-Fassung noch nicht freigegeben.", "World process: not yet available for this Gothic version."), zCOLOR(255, 200, 80, 255));
    }
    bool CoopWpImage(unsigned int& handle, std::string& nick, std::string& world, std::string& state) {
        handle = 0;
        nick.clear();
        world.clear();
        state = CoopTr("in dieser Gothic-Fassung noch nicht", "not yet in this Gothic version");
        return false;
    }
    void CoopWpEarlyInit() {}
    void CoopWpHostFeed(const json&) {}
    void CoopWpHostShutdown() {}
    void CoopWpHostCleanupOld() {}
    bool CoopWpMirroring() { return false; }
    bool CoopWpMirrorSendOn() { return false; }
    void CoopWpMirrorSendPulse() {}
    void CoopWpHostPulse() {}
    void CoopWpHostMenuPulse() {}
    void CoopWpWorldInit() { TerminateProcess(GetCurrentProcess(), 1); }
    void CoopWpWorldMenuPulse() {}
    void CoopWpWorldLoadBegin() { IsLoadingLevel = true; }
    void CoopWpWorldLoadEnd(bool) { IsLoadingLevel = false; }
    void CoopWpWorldLoop() {}

#endif
}
