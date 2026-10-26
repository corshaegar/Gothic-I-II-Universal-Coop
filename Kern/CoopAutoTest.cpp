// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- u49: automatischer Coop-Testlauf auf einem PC ------------------------------------------------------------------
    //
    // Nutzer (25.10.2026): "kannst du diese Simulation nicht selbst laufen lassen?" - "okay versuche das". Zwei Gothic auf diesem
    // PC, ohne Tastatur: tools\cooptest_lauf.py startet das Hauptspiel auf einem eigenen Desktop mit -cooptest:host:<Slot>. Der Host
    // legt den Testordner <Spiel>\<Spielordner>_CoopTest an (wie den Weltordner: Verknuepfungen, eigene Data/system/Saves - damit
    // auch eigene PC-Kennung in Saves\coop_archiv), kopiert den Slot hinein und startet dort ein zweites Gothic mit
    // -cooptest:client:<Slot> (gleicher Desktop, im Job - endet mit dem Host). Beide laden den Slot; der Host hostet (wie Shift+F1),
    // der Client verbindet ueber 127.0.0.1 (wie Shift+F2, nie Steam) und uebernimmt den Stand des Hosts. Dann laeuft der
    // Welten-Testlauf des Hosts, danach der des Clients (Ende ueber SELFTEST "end", CoopSelfTest.cpp). Ergebnis und Ablauf je
    // Instanz in system\GothicCoopAutoTest.log (wird wie die anderen GothicCoop*.log beim naechsten Start archiviert); danach
    // beenden sich beide. Der Weltprozess erbt die Kommandozeile - dort ohne Wirkung. Allgemein (G1/G2).

#if (ENGINE == Engine_G2A || ENGINE == Engine_G1) && COOP_BUILD(Save) && COOP_BUILD(SelfTest)   // wie Weltprozess und Testlauf

    enum CoopAutoPhase {
        CA_OFF, CA_MENU, CA_LOADING, CA_LOADED, CA_HOSTING, CA_CONNECTING, CA_PAIRED, CA_WAIT_HOST_RUN, CA_START_RUN, CA_RUNNING,
        CA_WAIT_PARTNER_RUN, CA_DONE
    };


    struct CoopAutoState {
        int role = -1;              // -1 noch nicht gelesen, 0 aus, 1 Host, 2 Client
        int slot = 1;
        int phase = CA_OFF;
        long long phaseMs = 0, startMs = 0;
        long long pairedSinceMs = 0;   // Partner ununterbrochen da (Figur oder in einer anderen Welt) seit
        int loadsSeen = -1;
        int tries = 0;
        long long runEndMs = 0, connectedMs = 0;
        std::string result;         // eigene Ergebniszeile
        HANDLE clientProc = NULL, clientJob = NULL;
        long long clientStartMs = 0;
        bool menuStarted = false;
    };
    static CoopAutoState CA;

    static int CoopAutoRole() {
        if (CA.role >= 0)
            return CA.role;
        CA.role = 0;
        if (CoopWpIsWorld())
            return 0;   // erbt -cooptest vom Hauptspiel
        const char* c = strstr(GetCommandLineA(), "-cooptest:");
        if (!c)
            return 0;
        c += 10;
        if (!_strnicmp(c, "host", 4)) {
            CA.role = 1;
            c += 4;
        }
        else if (!_strnicmp(c, "client", 6)) {
            CA.role = 2;
            c += 6;
        }
        if (CA.role && *c == ':' && atoi(c + 1) > 0)
            CA.slot = atoi(c + 1);
        return CA.role;
    }

    static std::string CoopAutoFile() {
        return std::string(GothicExeFolderPath) + "\\GothicCoopAutoTest.log";
    }

    static void CoopAutoLog(const char* fmt, ...) {
        char buf[900];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Autotest: %s", DiagSeconds(), DiagRole(), buf);
        if (FILE* f = fopen(CoopAutoFile().c_str(), "a")) {
            SYSTEMTIME st;
            GetLocalTime(&st);
            fprintf(f, "%02d:%02d:%02d %s %s\n", st.wHour, st.wMinute, st.wSecond, CA.role == 1 ? "Host  " : "Client", buf);
            fclose(f);
        }
    }

    static void CoopAutoSetPhase(int p) {
        CA.phase = p;
        CA.phaseMs = GetCurrentMs();
        CA.tries = 0;
    }

    static std::string CoopAutoRoot() {   // <Spiel>\<Spielordner>_CoopTest (vom Host aus)
        const std::string root = CoopGameRoot();
        const size_t s = root.find_last_of("\\/");
        std::string leaf = s == std::string::npos ? root : root.substr(s + 1);
        if (leaf.empty() || leaf.find(':') != std::string::npos)
            leaf = "Gothic";
        return root + "\\" + leaf + "_CoopTest";
    }

    // Ende: Ergebnis steht in der Datei; kurz warten (Log-Versand), dann ohne Rueckfrage beenden. Der Host beendet mit sich den
    // Client und beide Weltprozesse (Job-Objekte mit KILL_ON_JOB_CLOSE).
    static void CoopAutoExit(const char* why) {
        CoopAutoLog("ENDE (%s)%s%s", why, CA.result.empty() ? "" : " - ", CA.result.c_str());
        if (CA.role == 1 && CA.clientProc) {
            DWORD code = STILL_ACTIVE;
            GetExitCodeProcess(CA.clientProc, &code);
            CoopAutoLog("Testinstanz (Client) %s", code == STILL_ACTIVE ? "laeuft noch - wird mit dem Host beendet" : ("schon beendet, Code " + std::to_string(code)).c_str());
        }
        Sleep(500);
        TerminateProcess(GetCurrentProcess(), 0);
    }

    // Host: Testordner aufbauen, Slot hineinkopieren, Client starten
    static bool CoopAutoStartClient(std::string& why) {
        const std::string c = CoopAutoRoot();
        if (!WpEnsureCopyAt(c, "Testordner", why))
            return false;
        const std::string ini = c + "\\system\\Gothic.ini";
        WritePrivateProfileStringA("GOTHICCOOP", "coopName", "Testclient", ini.c_str());
        WritePrivateProfileStringA("GOTHICCOOP", "coopServer", "127.0.0.1", ini.c_str());
        WritePrivateProfileStringA("GOTHICCOOP", "coopPort", std::to_string(ConnectionPort).c_str(), ini.c_str());
        // Slot des Hosts in den Testordner (vorige Kopie dort vorher leeren - eigener Ordner)
        const std::string src = CoopSlotDir(CA.slot);
        const std::string dst = c + "\\Saves\\" + std::string(TSaveLoadGameInfo::GetSaveSlotName(CA.slot).ToChar()) + "\\";
        CreateDirectoryA(dst.c_str(), NULL);
        int copied = 0, failed = 0;
        WpForEachFile(dst, [&](const std::string& n) { DeleteFileA((dst + n).c_str()); });
        WpForEachFile(src, [&](const std::string& n) {
            if (CopyFileA((src + n).c_str(), (dst + n).c_str(), FALSE))
                copied++;
            else
                failed++;
        });
        if (!copied || failed) {
            why = "Slot " + std::to_string(CA.slot) + " (" + src + "): " + std::to_string(copied) + " Dateien kopiert, " + std::to_string(failed) + " nicht";
            return false;
        }
        // Kommandozeile: eigene Argumente ohne Rolle, Fenster/Aufloesung/Ton; dazu die des Tests
        std::string pass;
        {
            const std::string own = WpOwnArgs();
            size_t i = 0;
            while (i < own.size()) {
                while (i < own.size() && own[i] == ' ')
                    i++;
                const size_t e = own.find(' ', i);
                const std::string t = own.substr(i, e == std::string::npos ? std::string::npos : e - i);
                i = e == std::string::npos ? own.size() : e;
                if (t.empty() || !_strnicmp(t.c_str(), "-cooptest", 9) || !_stricmp(t.c_str(), "-zwindow") || !_strnicmp(t.c_str(), "-zres:", 6) ||
                    !_stricmp(t.c_str(), "-znosound") || !_stricmp(t.c_str(), "-znomusic"))
                    continue;
                pass += " " + t;
            }
        }
        const std::string dir = c + "\\system";
        const std::string exe = dir + "\\" + WpExeName();
        char cmd[2048];
        _snprintf(cmd, sizeof(cmd), "\"%s\" -cooptest:client:%d -zwindow -zres:960,540,32 -znosound -znomusic%s", exe.c_str(), CA.slot, pass.c_str());
        cmd[sizeof(cmd) - 1] = 0;
        // derselbe Desktop wie der Host (tools\cooptest_lauf.py: eigener Desktop "GothicCoopTest")
        char desk[256] = "";
        DWORD need = 0;
        GetUserObjectInformationA(GetThreadDesktop(GetCurrentThreadId()), UOI_NAME, desk, sizeof(desk), &need);
        std::string deskFull = std::string("WinSta0\\") + desk;
        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        // u50: Zuschauen (tools\cooptest_lauf.py --zeigen startet den Host auf dem Desktop des Nutzers) - dann als Fenster, sonst minimiert
        si.wShowWindow = !_stricmp(desk, "Default") ? SW_SHOWNOACTIVATE : SW_SHOWMINNOACTIVE;
        if (desk[0])
            si.lpDesktop = &deskFull[0];
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));
        if (!CreateProcessA(exe.c_str(), cmd, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, dir.c_str(), &si, &pi)) {
            why = "CreateProcess " + exe + " fehlgeschlagen (Fehler " + std::to_string(GetLastError()) + ")";
            return false;
        }
        HANDLE job = CreateJobObjectA(NULL, NULL);
        if (job) {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION li;
            ZeroMemory(&li, sizeof(li));
            li.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            SetInformationJobObject(job, JobObjectExtendedLimitInformation, &li, sizeof(li));
            AssignProcessToJobObject(job, pi.hProcess);
        }
        ResumeThread(pi.hThread);
        CloseHandle(pi.hThread);
        CA.clientProc = pi.hProcess;
        CA.clientJob = job;
        CA.clientStartMs = GetCurrentMs();
        CoopAutoLog("Testinstanz (Client) gestartet: Prozess %lu, Desktop %s, Slot %d (%d Dateien kopiert) - %s", (unsigned long)pi.dwProcessId,
            desk[0] ? desk : "?", CA.slot, copied, cmd);
        return true;
    }

    // Host: Meldungsboxen der Testinstanz bestaetigen (Test u49: SystemPack "First start!" beim ersten Start der Exe-Kopie)
    static void CoopAutoDialogs() {
        static long long next = 0;
        static std::set<HWND> handled;
        const long long now = GetCurrentMs();
        if (CA.role != 1 || !CA.clientProc || now < next)
            return;
        next = now + 1000;
        WpConfirmDialogs(GetThreadDesktop(GetCurrentThreadId()), GetProcessId(CA.clientProc), handled, "in der Testinstanz", CA.clientStartMs);
    }

    static bool CoopAutoPartnerHere() {
        return !PlayerNpcs.empty() || !CoopWorldsElsewhere().empty();
    }

    // UnionHooks.cpp _Game_Init (Hauptspiel): Reste verstecken - Plugin-Dateien des Testordners (und seines Weltordners) duerfen
    // fuer Unions Suche im Spielordner nie sichtbar sein (P83); die Testinstanz versteckt ihre eigenen, sobald sie geladen sind
    void CoopAutoTestInit() {
        const int role = CoopAutoRole();
        if (role == 2) {
            WpHidePlugins(CoopGameRoot(), "Testinstanz gestartet");
            CoopAutoLog("Testinstanz (Client) gestartet, Slot %d, Spielordner %s", CA.slot, CoopGameRoot().c_str());
        }
        else {
            const std::string t = CoopAutoRoot();
            if (GetFileAttributesA(t.c_str()) != INVALID_FILE_ATTRIBUTES) {
                WpHidePlugins(t, "Spielstart (Testordner)");
                const size_t s = t.find_last_of("\\/");
                WpHidePlugins(t + "\\" + t.substr(s + 1) + "_CoopWelt", "Spielstart (Weltordner des Testordners)");
            }
            if (role == 1)   // die Datei des vorigen Laufs hat das Log-Archiv beim Start mitgenommen
                CoopAutoLog("Host gestartet, Slot %d, %s", CA.slot, COOP_FASSUNG);
        }
        if (role)
            CA.startMs = GetCurrentMs();
    }

    // Plugin.cpp Game_Entry: die Testinstanz laeuft neben dem Hauptspiel - Gothics Einzelinstanz-Sperre aufheben (wie Weltprozess)
    void CoopAutoTestEarly() {
        if (CoopAutoRole() == 2)
            WpSkipInstanceLock("in der Testinstanz");
    }

    // UnionHooks.cpp _Game_MenuLoop (Hauptspiel): Slot laden; der Host startet vorher den Client
    void CoopAutoTestMenuPulse() {
        if (CoopAutoRole() == 0)
            return;
        CoopAutoDialogs();
        static bool placed = false;   // u50d: Zuschauen - beide Fenster lagen bei (0,0) uebereinander; Titel nennt die Rolle
        if (!placed && hWndApp) {
            placed = true;
            char desk[256] = "";
            DWORD need = 0;
            GetUserObjectInformationA(GetThreadDesktop(GetCurrentThreadId()), UOI_NAME, desk, sizeof(desk), &need);
            RECT r;
            if (CA.role == 2 && !_stricmp(desk, "Default") && GetWindowRect(hWndApp, &r)) {
                const int x = GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left);
                SetWindowPos(hWndApp, NULL, x > r.right ? x : r.right, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
            }
            SetWindowTextA(hWndApp, CA.role == 1 ? "Host - Coop-Testlauf" : "Testclient - Coop-Testlauf");
        }
        if (player)
            return;
        const long long now = GetCurrentMs();
        if (CA.phase == CA_OFF) {
            CoopAutoSetPhase(CA_MENU);
            return;
        }
        if (CA.phase == CA_MENU && now > CA.phaseMs + 3000) {
            if (CA.role == 1 && !CA.menuStarted) {
                CA.menuStarted = true;
                std::string why;
                const bool started = CoopAutoStartClient(why);
                if (!started) {
                    CoopAutoLog("FEHLER: Testinstanz nicht gestartet - %s", why.c_str());
                    CoopAutoExit("Testinstanz nicht gestartet");
                    return;
                }
            }
            zCMenu* menu = zCMenu::GetActive();
            if (!menu || !gameMan || !gameMan->menu_load_savegame)
                return;
            if (gameMan->savegameManager)
                gameMan->savegameManager->Reinit();   // Client: der Slot kam kurz vor dem Start
            gameMan->menu_load_savegame->m_selSlot = CA.slot;
            CoopAutoSetPhase(CA_LOADING);
            CoopAutoLog("lade Slot %d (Menue %s)", CA.slot, menu->name.ToChar());
            menu->ForceSelAction(SEL_ACTION_CLOSE, zSTRING(CoopProfileIsOthello() ? "OTHELLO_SAVEGAME_LOAD" : "SAVEGAME_LOAD"), NULL);
            return;
        }
        if (CA.phase == CA_LOADING && now > CA.phaseMs + 180000)
            CoopAutoExit("Slot nach 3 min nicht geladen");
    }

    // Plugin.cpp Game_Loop: der eigentliche Ablauf
    void CoopAutoTestPulse() {
        if (CoopAutoRole() == 0)
            return;
        CoopAutoDialogs();
        if (!player || IsLoadingLevel)
            return;
        const long long now = GetCurrentMs();
        const int limitMin = 45;
        const bool clientWatched = CA.phase < CA_DONE;
        if (now > CA.startMs + limitMin * 60000LL) {
            CoopAutoLog("FEHLER: Zeitlimit %d min (Phase %d)", limitMin, CA.phase);
            CoopAutoExit("Zeitlimit");
            return;
        }
        if (CA.role == 1 && CA.clientProc && clientWatched) {
            DWORD code = STILL_ACTIVE;
            if (GetExitCodeProcess(CA.clientProc, &code) && code != STILL_ACTIVE && CA.phase != CA_WAIT_PARTNER_RUN) {
                {
                    CoopAutoLog("FEHLER: Testinstanz (Client) beendet, Code 0x%08X (Phase %d)", (unsigned)code, CA.phase);
                    CoopAutoExit("Client weg");
                    return;
                }
            }
        }
        const bool partner = CoopAutoPartnerHere();
        if (!partner || CoopWorldLoads != CA.loadsSeen) {   // Partner weg oder eigener Ladevorgang: Ruhe neu abwarten
            CA.pairedSinceMs = partner ? now : 0;
            CA.loadsSeen = CoopWorldLoads;
        }
        else if (!CA.pairedSinceMs)
            CA.pairedSinceMs = now;
        const bool settled = partner && CA.pairedSinceMs && now > CA.pairedSinceMs + 25000;

        switch (CA.phase) {
        case CA_MENU:
        case CA_LOADING:
            CoopAutoSetPhase(CA_LOADED);
            CoopAutoLog("Slot geladen: %s", CoopWorldName().c_str());
            break;
        case CA_LOADED:
            if (now < CA.phaseMs + 4000)
                break;
            if (CA.role == 1) {
                CoopKeyHost();
                CoopAutoLog("Host gestartet (Port %d)", ConnectionPort);
                CoopAutoSetPhase(CA_HOSTING);
            }
            else {
                CoopSteamForgetLast();   // nie ueber Steam - der Host laeuft auf diesem PC
                CoopKeyClient();
                CoopAutoLog("verbinde mit %s:%d", CoopServerIp.c_str(), ConnectionPort);
                CoopAutoSetPhase(CA_CONNECTING);
            }
            break;
        case CA_HOSTING:
            if (settled) {
                CoopAutoLog("Partner da (%s) - starte den Welten-Testlauf", PlayerNpcs.empty() ? "andere Welt" : "Figur in meiner Welt");
                CoopAutoSetPhase(CA_START_RUN);
            }
            else if (now > CA.phaseMs + 15 * 60000) {
                CoopAutoLog("FEHLER: kein Mitspieler nach 15 min");
                CoopAutoExit("kein Mitspieler");
            }
            break;
        case CA_CONNECTING:
            if (ClientThread && CoopClientState == 2) {
                CA.connectedMs = now;
                CoopAutoLog("verbunden");
                CoopAutoSetPhase(CA_PAIRED);
            }
            else if (!ClientThread && now > CA.phaseMs + 15000) {   // fehlgeschlagen und aufgeraeumt (der Host bootet evtl. noch)
                if (++CA.tries > 60) {
                    CoopAutoLog("FEHLER: keine Verbindung nach 60 Versuchen");
                    CoopAutoExit("keine Verbindung");
                    break;
                }
                CA.phaseMs = now;
                CoopSteamForgetLast();
                CoopKeyClient();
                if (CA.tries % 4 == 0)
                    CoopAutoLog("verbinde erneut (Versuch %d)", CA.tries + 1);
            }
            break;
        case CA_PAIRED:
            if (CTPartnerQueryMs > CA.connectedMs) {
                CoopAutoLog("Testlauf des Hosts laeuft (%s) - warte auf sein Ende", CTPartnerRun.c_str());
                CoopAutoSetPhase(CA_WAIT_HOST_RUN);
            }
            else if (now > CA.phaseMs + 20 * 60000) {
                CoopAutoLog("FEHLER: Testlauf des Hosts kam nicht");
                CoopAutoExit("kein Lauf des Hosts");
            }
            break;
        case CA_WAIT_HOST_RUN:
            if (CTPartnerEndMs > CA.connectedMs && now > CTPartnerEndMs + 15000 && settled) {
                CoopAutoLog("Host-Lauf zu Ende (%s) - starte den eigenen Welten-Testlauf", CTPartnerEndText.c_str());
                CoopAutoSetPhase(CA_START_RUN);
            }
            break;
        case CA_START_RUN:
            if (CT.on) {
                CoopAutoLog("Welten-Testlauf %s laeuft", CT.tag.c_str());
                CoopAutoSetPhase(CA_RUNNING);
                break;
            }
            if (now > CA.phaseMs + 2000 + 15000 * CA.tries) {
                if (++CA.tries > 8) {
                    CoopAutoLog("FEHLER: Testlauf startet nicht");
                    CoopAutoExit("Testlauf startet nicht");
                    break;
                }
                    CoopSelfTestToggle(true);
            }
            break;
        case CA_RUNNING:
            if (!CT.on) {
                char b[200];
                _snprintf(b, sizeof(b), "Lauf %s: %d OK, %d FEHLER, %d uebersprungen, %d bekannte Luecke", CT.tag.c_str(), CT.ok, CT.fail,
                    CT.skip, CT.gap);
                b[sizeof(b) - 1] = 0;
                CA.result = b;
                CoopAutoLog("ERGEBNIS %s", b);
                for (auto& f : CT.fails)
                    CoopAutoLog("  FEHLER: %s", f.c_str());
                CA.runEndMs = now;
                if (CA.role == 1)
                    CoopAutoSetPhase(CA_WAIT_PARTNER_RUN);
                else
                    CoopAutoSetPhase(CA_DONE);
            }
            break;
        case CA_WAIT_PARTNER_RUN:   // nur Host
            if (CTPartnerEndMs > CA.runEndMs) {
                CoopAutoLog("Client-Lauf zu Ende: %s", CTPartnerEndText.c_str());
                CoopAutoSetPhase(CA_DONE);
            }
            else if (CA.clientProc) {
                DWORD code = STILL_ACTIVE;
                if (GetExitCodeProcess(CA.clientProc, &code) && code != STILL_ACTIVE) {
                    CoopAutoLog("FEHLER: Testinstanz (Client) vor dem Ende ihres Laufs beendet, Code 0x%08X", (unsigned)code);
                    CoopAutoSetPhase(CA_DONE);
                }
            }
            if (CA.phase == CA_WAIT_PARTNER_RUN && now > CA.phaseMs + 25 * 60000) {
                CoopAutoLog("FEHLER: Lauf des Clients nach 25 min nicht zu Ende");
                CoopAutoSetPhase(CA_DONE);
            }
            break;
        case CA_DONE:
            // Client zuerst (sein Ende hat der Host schon), der Host danach - er beendet den Client notfalls mit
            if (now > CA.phaseMs + (CA.role == 1 ? 20000 : 8000))
                CoopAutoExit("fertig");
            break;
        }
    }

#else   // Weltprozess bzw. Testlauf in dieser Engine nicht freigegeben
    void CoopAutoTestInit() {}
    void CoopAutoTestEarly() {}
    void CoopAutoTestMenuPulse() {}
    void CoopAutoTestPulse() {}
#endif
}
