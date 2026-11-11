// Supported with union (c) 2020 Union team
// Union SOURCE file

#include <tlhelp32.h>   // u251 (P301): Module des eigenen Prozesses

namespace GOTHIC_ENGINE {

    // --- Maus nicht einsperren auf einem eigenen Desktop (u251, P301) --------------------------------------------------------------
    //
    // Nutzer 10.11.2026: "ich kann manchmal meine maus nicht ausserhalb einer box oben links bewegen warum ist das so gerade ist es
    // wieder so" - GetClipCursor zeigte (0,0)-(768,432), das Fenster einer Gothic-Instanz mit 960x540 bei 125 % Skalierung; waehrend des
    // Xeres-Testlaufs musste ein Helfer die Maus 1571-mal wieder freigeben. Gothic-Instanzen auf einem eigenen Windows-Desktop (Coop-
    // Testlauf "GothicCoopTest_...", Weltprozess "GothicCoopWelt" - auch beim normalen Coop-Spiel) halten ihr Fenster dort fuer aktiv;
    // GRawInput (System\dinput.dll, z. B. in Xeres) und GD3D11 (g2a*.dll) sperren die Maus dann mit ClipCursor in dieses Fenster - und
    // die Sperre gilt fuer die echte Maus des Spielers auf seinem Desktop. Laeuft GT nicht auf dem Desktop "Default", leitet es
    // ClipCursor in den Importtabellen aller Module des Prozesses auf eine Funktion um, die nur loest, nie sperrt. Auf dem Desktop des
    // Spielers aendert sich nichts. Allgemein (jede Mod, beide Engines)
    // u252 (P301b): Messung im Xeres-Lauf u252 (GetClipCursor alle 100 ms auf dem Desktop des Spielers): trotz P301 in 440 von 7860 Proben
    // eingesperrt, nur beim Starten und Laden der Instanzen. In der 32-Bit-user32.dll ist ClipCursor nur ein Sprung ueber deren eigene
    // Importtabelle auf win32u!NtUserClipCursor; Aufrufer, die ClipCursor ueber einen API-Set-Namen (ext-ms-win-ntuser-...) oder erst
    // spaeter (verzoegertes Laden, GetProcAddress) beziehen, entgingen der Umleitung. Jetzt werden in allen Importtabellen beide
    // Eintraege umgeleitet, egal unter welchem DLL-Namen - auch der in user32.dll selbst, ueber den jeder Aufruf von ClipCursor laeuft
    typedef BOOL(WINAPI* CoopClipCursorFn)(const RECT*);
    static CoopClipCursorFn CoopClipCursorEcht = NULL;
    static CoopClipCursorFn CoopNtClipEcht = NULL;   // win32u!NtUserClipCursor (ab Windows 10), sonst NULL
    static std::set<HMODULE> CoopMausGeprueft;
    static int CoopMausUmgeleitet = 0;

    static bool CoopEigenerDesktop() {
        static int eigen = -1;
        if (eigen < 0) {
            char name[256] = "";
            DWORD need = 0;
            GetUserObjectInformationA(GetThreadDesktop(GetCurrentThreadId()), UOI_NAME, name, sizeof(name), &need);
            name[sizeof(name) - 1] = 0;
            eigen = name[0] && _stricmp(name, "Default") != 0 ? 1 : 0;
        }
        return eigen == 1;
    }

    static BOOL WINAPI CoopClipCursorNurLoesen(const RECT* r) {
        if (r)
            return TRUE;   // P301: nicht einsperren - das Fenster liegt auf einem eigenen Desktop
        return CoopClipCursorEcht ? CoopClipCursorEcht(NULL) : TRUE;
    }

    static BOOL WINAPI CoopNtClipNurLoesen(const RECT* r) {   // u252 (P301b): derselbe Riegel hinter user32!ClipCursor
        if (r)
            return TRUE;
        return CoopNtClipEcht ? CoopNtClipEcht(NULL) : TRUE;
    }

    static int CoopMausIat(HMODULE m) {
        BYTE* base = (BYTE*)m;
        IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
        if (!IsReadableMemory(base, sizeof(IMAGE_DOS_HEADER)) || dos->e_magic != IMAGE_DOS_SIGNATURE)
            return 0;
        IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
        if (!IsReadableMemory(nt, sizeof(IMAGE_NT_HEADERS)) || nt->Signature != IMAGE_NT_SIGNATURE)
            return 0;
        IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
        if (!dir.VirtualAddress || !dir.Size)
            return 0;
        int n = 0;
        for (IMAGE_IMPORT_DESCRIPTOR* imp = (IMAGE_IMPORT_DESCRIPTOR*)(base + dir.VirtualAddress); imp->Name; imp++) {
            // u252 (P301b): jeder DLL-Name (user32.dll, API-Sets, win32u.dll) - verglichen wird das Ziel des Eintrags
            for (IMAGE_THUNK_DATA* t = (IMAGE_THUNK_DATA*)(base + imp->FirstThunk); t->u1.Function; t++) {
                ULONG_PTR neu = 0;
                if ((ULONG_PTR)t->u1.Function == (ULONG_PTR)CoopClipCursorEcht)
                    neu = (ULONG_PTR)&CoopClipCursorNurLoesen;
                else if (CoopNtClipEcht && (ULONG_PTR)t->u1.Function == (ULONG_PTR)CoopNtClipEcht)
                    neu = (ULONG_PTR)&CoopNtClipNurLoesen;
                if (!neu)
                    continue;
                DWORD alt = 0;
                if (!VirtualProtect(&t->u1.Function, sizeof(t->u1.Function), PAGE_READWRITE, &alt))
                    continue;
                t->u1.Function = neu;
                VirtualProtect(&t->u1.Function, sizeof(t->u1.Function), alt, &alt);
                n++;
            }
        }
        return n;
    }

    // Start, jedes Menue- und Spielbild, Weltprozess: hoechstens alle 2 s die Module durchsehen (GD3D11 laedt spaeter als GRawInput)
    void CoopMausKaefigAus(const char* wann) {
        if (!CoopEigenerDesktop())
            return;
        static DWORD naechste = 0;
        const DWORD jetzt = GetTickCount();
        if (naechste && (int)(jetzt - naechste) < 0)
            return;
        naechste = jetzt + 2000;
        if (!CoopClipCursorEcht) {
            CoopClipCursorEcht = (CoopClipCursorFn)GetProcAddress(GetModuleHandleA("user32.dll"), "ClipCursor");
            if (HMODULE w = GetModuleHandleA("win32u.dll"))
                CoopNtClipEcht = (CoopClipCursorFn)GetProcAddress(w, "NtUserClipCursor");
        }
        if (!CoopClipCursorEcht)
            return;
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
        if (snap == INVALID_HANDLE_VALUE)
            return;
        MODULEENTRY32 me;
        me.dwSize = sizeof(me);
        std::string welche;
        int n = 0;
        for (BOOL ok = Module32First(snap, &me); ok; ok = Module32Next(snap, &me)) {
            if (!CoopMausGeprueft.insert(me.hModule).second)
                continue;
            const int k = CoopMausIat(me.hModule);
            if (k > 0) {
                n += k;
                welche += std::string(welche.empty() ? "" : ", ") + me.szModule;
            }
        }
        CloseHandle(snap);
        if (!n)
            return;
        CoopMausUmgeleitet += n;
        CoopClipCursorEcht(NULL);   // eine schon gesetzte Sperre loesen
        static int zeilen = 0;
        if (zeilen++ < 10)
            OthelloCompatLog("Mauskaefig (P301, %s): eigener Desktop - ClipCursor in %s umgeleitet, die Maus des Spielers wird nicht mehr "
                "in dieses Fenster gesperrt", wann, welche.c_str());
    }

} // namespace GOTHIC_ENGINE
