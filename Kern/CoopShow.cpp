// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- u171: Vorfuehrung fuer Aufnahmen ---------------------------------------------------------------------------------
    //
    // Nutzer (4.11.2026): "eine Aufnahme ... fuer den Anfang des Spiels mit 2 Testinstanzen ... bei der Aufnahme soll ausserdem das menue
    // gezeigt werden und die Einstellungen im Spiel angepasst etc." Im Testlauf mit -coopaufnahme (tools\cooptest_lauf.py
    // --aufnahme) gehen Host und Client sichtbar durchs Menue wie zwei Spieler. Jede Taste kommt in Gothics Tastenpuffer
    // (CoopMenuPushKey, wie die Maus in CoopMenuMouse.cpp) - das Menue behandelt sie im naechsten Bild wie einen Tastendruck, auch
    // in Untermenues, die Gothic verschachtelt in zCMenu::Run laufen laesst (Game_MenuLoop kommt aus dem innersten Menue hierher).
    //   Host:   Einstellungen > Spiel (Untertitel an) > COOP-Einstellungen ansehen; COOP > Figur (Kopf, Gesicht) > Spiel hosten,
    //           warten, bis der Mitspieler verbunden ist; Neues Spiel > COOP-Schwierigkeit > Spiel starten (beide beginnen).
    //   Client: Einstellungen > COOP-Einstellungen; COOP > Figur (Kopf, Haut) > Ueber IP beitreten > Verbinden - das neue Spiel
    //           startet der Host (CoopMenuLobby.cpp, "newgame").
    // Fehlt ein Eintrag oder kommt ein Menue nicht (andere Mod), wird der Abschnitt abgebrochen und ins Hauptmenue zurueckgegangen;
    // scheitert der COOP-Teil, macht der Testlauf wie in u170 weiter (CoopAutoTest.cpp). Allgemein G1/G2.

    bool CoopMenuPushKey(int key);   // CoopMenuMouse.cpp
    bool CoopLobbyPeersReadyNow();   // CoopMenuLobby.cpp

#if (ENGINE == Engine_G2A || ENGINE == Engine_G1) && COOP_BUILD(Save) && COOP_BUILD(SelfTest)   // wie der Testlauf

    enum CoopShowOp {
        SH_WAIT,        // a = ms
        SH_PART,        // Abschnitt: arg = Name, a = 1 Pflicht (scheitert er, endet die Vorfuehrung mit -1)
        SH_MENU,        // warten, bis das oberste Menue so heisst (arg, mit | getrennt), a = Frist in ms
        SH_SEL,         // Eintrag arg markieren - Pfeil runter/hoch, bis m_mainSel stimmt
        SH_KEY,         // Taste a, n-mal
        SH_CHOICE_TO,   // Eintrag arg markieren, dann rechts, bis das Wahlfeld dahinter den Wert a zeigt
        SH_PEER,        // Host: warten, bis der Mitspieler verbunden und abgeglichen ist, a = Frist in ms
        SH_MAIN,        // Escape bis zum Hauptmenue
        SH_LAST,        // RETURN - damit beginnt das Spiel bzw. die Verbindung; Vorfuehrung fertig (arg = Text fuers Log)
        SH_NGPICK,      // u172: nach "Neues Spiel" - steht ein eigenes Menue der Mod davor (Othello: Schwierigkeit, CLOSE
                        // "OTHELLO_NEW_..."), dort den Eintrag mit arg im Aktionstext waehlen (sonst den markierten); bei MENU_COOP_NG weiter
        SH_LOOK         // u176: Aussehen wie der NPC arg (wie das Eingabefeld "Aussehen wie NPC", CoopLook.cpp)
    };

    std::string CoopLookTakeNpc(const char* name);   // CoopLook.cpp (u176)

    struct CoopShowStep {
        CoopShowOp op;
        const char* arg;
        int a;
        int n;
    };

    static const CoopShowStep ShowHost[] = {
        { SH_WAIT, NULL, 2500, 0 },
        { SH_PART, "Einstellungen", 0, 0 },
        { SH_SEL, "MENUITEM_MAIN_OPTIONS", 0, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_OPTIONS", 5000, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_SEL, "MENUITEM_OPT_GAME", 0, 0 },
        { SH_WAIT, NULL, 400, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_OPT_GAME", 5000, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_CHOICE_TO, "MENUITEM_GAME_SUB_TITLES", 1, 0 },
        { SH_WAIT, NULL, 900, 0 },
        { SH_SEL, "MENUITEM_GAME_LOOKAROUND_INVERSE", 0, 0 },
        { SH_WAIT, NULL, 900, 0 },
        { SH_KEY, NULL, KEY_ESCAPE, 1 },
        { SH_MENU, "MENU_OPTIONS", 5000, 0 },
        { SH_WAIT, NULL, 700, 0 },
        { SH_SEL, "MENUITEM_OPT_COOP", 0, 0 },
        { SH_WAIT, NULL, 400, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_OPT_COOP", 5000, 0 },
        { SH_WAIT, NULL, 1500, 0 },
        { SH_SEL, "MENUITEM_OPTCOOP_DIFF", 0, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_SEL, "MENUITEM_OPTCOOP_SYNC", 0, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_KEY, NULL, KEY_ESCAPE, 1 },
        { SH_MENU, "MENU_OPTIONS", 5000, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_MAIN, NULL, 0, 0 },
        { SH_WAIT, NULL, 800, 0 },
        { SH_PART, "COOP", 1, 0 },
        { SH_SEL, "MENUITEM_COOPMAIN_COOP", 0, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_COOP_LOOK", 6000, 0 },
        { SH_WAIT, NULL, 1500, 0 },
        { SH_SEL, "MENUITEM_COOP_LOOKNPC_INP", 0, 0 },   // u176: Aussehen wie Gorn (Nutzer: "vielleicht Gorn und Diego")
        { SH_WAIT, NULL, 900, 0 },
        { SH_LOOK, "Gorn", 0, 0 },
        { SH_WAIT, NULL, 2500, 0 },
        { SH_SEL, "MENUITEM_COOPLOOK_NEXT", 0, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_COOP", 6000, 0 },
        { SH_WAIT, NULL, 1500, 0 },
        { SH_SEL, "MENUITEM_COOP_HOST", 0, 0 },
        { SH_WAIT, NULL, 600, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_WAIT, NULL, 1500, 0 },
        { SH_PEER, NULL, 240000, 0 },
        { SH_WAIT, NULL, 2500, 0 },
        { SH_SEL, "MENUITEM_COOP_NEWGAME", 0, 0 },
        { SH_WAIT, NULL, 600, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_NGPICK, "NORMAL", 8000, 0 },
        { SH_MENU, "MENU_COOP_NG", 8000, 0 },
        { SH_WAIT, NULL, 1500, 0 },
        { SH_SEL, "MENUITEM_COOPDIFF_XPPARTNER", 0, 0 },
        { SH_WAIT, NULL, 1000, 0 },
        { SH_SEL, "MENUITEM_COOPNG_START", 0, 0 },
        { SH_WAIT, NULL, 1000, 0 },
        { SH_LAST, "Spiel starten - beide beginnen", 0, 0 },
    };

    static const CoopShowStep ShowClient[] = {
        { SH_WAIT, NULL, 9000, 0 },
        { SH_PART, "Einstellungen", 0, 0 },
        { SH_SEL, "MENUITEM_MAIN_OPTIONS", 0, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_OPTIONS", 5000, 0 },
        { SH_WAIT, NULL, 1000, 0 },
        { SH_SEL, "MENUITEM_OPT_COOP", 0, 0 },
        { SH_WAIT, NULL, 400, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_OPT_COOP", 5000, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_SEL, "MENUITEM_OPTCOOP_CAM", 0, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_KEY, NULL, KEY_ESCAPE, 1 },
        { SH_MENU, "MENU_OPTIONS", 5000, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_MAIN, NULL, 0, 0 },
        { SH_WAIT, NULL, 800, 0 },
        { SH_PART, "COOP", 1, 0 },
        { SH_SEL, "MENUITEM_COOPMAIN_COOP", 0, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_COOP_LOOK", 6000, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_SEL, "MENUITEM_COOP_LOOKNPC_INP", 0, 0 },   // u176: Aussehen wie Diego
        { SH_WAIT, NULL, 900, 0 },
        { SH_LOOK, "Diego", 0, 0 },
        { SH_WAIT, NULL, 2500, 0 },
        { SH_SEL, "MENUITEM_COOPLOOK_NEXT", 0, 0 },
        { SH_WAIT, NULL, 500, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_COOP", 6000, 0 },
        { SH_WAIT, NULL, 1200, 0 },
        { SH_SEL, "MENUITEM_COOP_IPJOIN", 0, 0 },
        { SH_WAIT, NULL, 600, 0 },
        { SH_KEY, NULL, KEY_RETURN, 1 },
        { SH_MENU, "MENU_COOP_IP", 6000, 0 },
        { SH_WAIT, NULL, 2000, 0 },
        { SH_SEL, "MENUITEM_COOP_CLIENT", 0, 0 },
        { SH_WAIT, NULL, 800, 0 },
        { SH_LAST, "Verbinden", 0, 0 },
    };

    static struct {
        const CoopShowStep* steps = NULL;
        int count = 0;
        int idx = 0;
        long long nextMs = 0, stepMs = 0;
        int presses = 0, moves = 0;
        bool fail = false, required = false, on = false, direct = false;
        int result = 0;   // 0 laeuft, 1 fertig, -1 abgebrochen
        std::string part;
    } Show;

    static void ShowLog(const char* fmt, ...) {
        char buf[600];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Vorfuehrung: %s", DiagSeconds(), DiagRole(), buf);
    }

    // Name mit | getrennter Liste vergleichen
    static bool ShowNameIn(const zSTRING& name, const char* list) {
        const char* p = list;
        while (p && *p) {
            const char* e = strchr(p, '|');
            const size_t n = e ? (size_t)(e - p) : strlen(p);
            if ((size_t)name.Length() == n && !_strnicmp(name.ToChar(), p, n))
                return true;
            p = e ? e + 1 : NULL;
        }
        return false;
    }

    static int ShowFind(zCMenu* menu, const char* ids) {
        for (int i = 0; i < menu->m_listItems.GetNum(); i++) {
            zCMenuItem* it = menu->m_listItems[i];
            if (it && ShowNameIn(it->id, ids))
                return i;
        }
        return -1;
    }

    static void ShowNext(long long now, int pauseMs) {
        Show.idx++;
        Show.presses = Show.moves = 0;
        Show.stepMs = 0;
        Show.nextMs = now + pauseMs;
    }

    static int ShowFinish(int result, const char* why) {
        Show.on = false;
        Show.result = result;
        ShowLog("%s (%s)", result > 0 ? "fertig" : "abgebrochen", why);
        return result;
    }

    static int ShowFail(zCMenu* menu, const CoopShowStep& s, const char* why, long long now) {
        ShowLog("Abschnitt %s: %s (%s, Menue %s) - %s", Show.part.c_str(), why, s.arg ? s.arg : "-", menu ? menu->name.ToChar() : "?",
            Show.required ? "zurueck ins Hauptmenue, dann wie ohne Vorfuehrung" : "zurueck ins Hauptmenue, weiter mit dem naechsten Abschnitt");
        Show.fail = true;
        // bis zum naechsten SH_MAIN springen (er folgt jedem freiwilligen Abschnitt); fehlt er (Pflichtabschnitt am Ende), geht
        // CoopShowPulse selbst mit Escape ins Hauptmenue zurueck
        while (Show.idx < Show.count && Show.steps[Show.idx].op != SH_MAIN)
            Show.idx++;
        Show.presses = Show.moves = 0;
        Show.stepMs = 0;
        Show.nextMs = now + 600;
        return 0;
    }

    // Eintrag direkt markieren wie zCMenu::NextOption (SetIsActive, HandleLeaveItem vtable +8, m_mainSel, HandleEnterItem +4) - wenn
    // die Pfeiltasten ihn nicht erreichen (Probe 04.11. 17:20: "Neues Spiel" im COOP-Menue nach 30 Schritten nicht markiert)
    static void ShowSelectDirect(zCMenu* menu, int idx) {
        const int n = menu->m_listItems.GetNum();
        if (idx < 0 || idx >= n || idx == menu->m_mainSel || !menu->m_listItems[idx])
            return;
        typedef int(__thiscall* ItemFn)(zCMenu*, zCMenuItem*);
        const int old = menu->m_mainSel;
        if (old >= 0 && old < n && menu->m_listItems[old]) {
            zCMenuItem* was = menu->m_listItems[old];
            was->SetIsActive(0);
            ((ItemFn)(*(DWORD**)menu)[0x08 / 4])(menu, was);
        }
        menu->m_mainSel = idx;
        zCMenuItem* now = menu->m_listItems[idx];
        ((ItemFn)(*(DWORD**)menu)[0x04 / 4])(menu, now);
        if (menu->cursorEn)
            now->SetIsActive(1);
    }

    // Enter/Escape ohne Tastenpuffer: zCMenu::ForceSelAction setzt m_exitState (GO bzw. BACK) und die Aktion, zCMenu::Run fuehrt sie
    // nach dem Bild in HandleAction aus - wie ein Druck auf RETURN beim markierten Eintrag. Probe 04.11. 17:31: nach "Spiel hosten"
    // kamen Tasten aus dem Puffer im Menue nicht mehr an (Pfeile, RETURN, Escape), ForceSelAction wirkte. Nur, wenn das Menue gerade
    // keine Aktion ausfuehrt (m_exitState 0 - sonst setzt Run ihn danach zurueck, CoopTitle.cpp)
    static bool ShowActivate(zCMenu* menu) {
        if ((int)menu->m_exitState != 0 || menu->m_mainSel < 0 || menu->m_mainSel >= menu->m_listItems.GetNum())
            return false;
        zCMenuItem* it = menu->m_listItems[menu->m_mainSel];
        if (!it || it->m_parOnSelAction[0] == SEL_ACTION_UNDEF)
            return false;
        menu->ForceSelAction((zTMenuItemSelAction)it->m_parOnSelAction[0], it->m_parOnSelAction_S[0], it);
        return true;
    }

    static bool ShowBack(zCMenu* menu) {
        if ((int)menu->m_exitState != 0)
            return false;
        menu->ForceSelAction(SEL_ACTION_BACK, zSTRING(""), NULL);
        return true;
    }

    // Markierung einen Schritt Richtung Eintrag bewegen (Pfeiltaste wie ein Spieler, nach 12 Schritten direkt); true = steht drauf
    static bool ShowMoveTo(zCMenu* menu, int want, long long now) {
        if (menu->m_mainSel == want)
            return true;
        Show.moves++;
        if (Show.moves > 12 || Show.direct) {
            if (!Show.direct)
                ShowLog("Eintrag %d per Pfeil nicht erreicht (steht auf %d) - ab jetzt direkt markiert", want, menu->m_mainSel);
            Show.direct = true;   // Tasten kommen nicht mehr an (nach "Spiel hosten") - nicht bei jedem Eintrag 12 Schritte warten
            ShowSelectDirect(menu, want);
            Show.nextMs = now + 300;
            return menu->m_mainSel == want;
        }
        CoopMenuPushKey(want > menu->m_mainSel ? KEY_DOWN : KEY_UP);
        Show.nextMs = now + 300;
        return false;
    }

    bool CoopShowActive() {
        return Show.on;
    }

    // u175: Aufnahme Gothic 1 (04.11. 18:24 und 19:02-19:20): beide Fenster schwarz - Gothic 1 zeichnet ohne Fokus nicht weiter (das
    // Fenster im Vordergrund zeigte das Spiel, die Logik lief weiter; Gothic II zeichnet weiter). In der Aufnahme meldet GT nach jedem
    // Fokusverlust gleich wieder "aktiv" (WM_ACTIVATEAPP / WM_ACTIVATE an Gothics Fensterfunktion). GD3D11 prueft dabei selbst, ob das
    // Fenster vorn ist (UpdateFocus: GetForegroundWindow), und klemmt den Mauszeiger nicht ein
    static WNDPROC ShowPrevProc = NULL;
    static int ShowFocusFakes = 0;

    static LRESULT CALLBACK ShowWndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
        const LRESULT r = CallWindowProcA(ShowPrevProc, h, m, w, l);
        if ((m == WM_ACTIVATEAPP && !w) || (m == WM_ACTIVATE && LOWORD(w) == WA_INACTIVE) || m == WM_KILLFOCUS) {
            CallWindowProcA(ShowPrevProc, h, WM_ACTIVATEAPP, TRUE, 0);
            CallWindowProcA(ShowPrevProc, h, WM_ACTIVATE, WA_ACTIVE, 0);
            if (++ShowFocusFakes <= 5)
                ShowLog("Fokus verloren (Nachricht %04X) - Gothic bekommt gleich wieder aktiv gemeldet (Aufnahme)", m);
        }
        return r;
    }

    // u180 (Nutzer 5.11.: Aufnahme "außerdem mit sound" - "am besten den sound nicht die ganze zeit bei beiden abspielen, vorallem bei
    // dialogen"): aufnahme.exe nimmt den Ton beider Spiele getrennt ab und laesst nur eines hoeren - es muss wissen, wer gerade spricht
    // oder kaempft. Je Prozess zwei benannte Ereignisse, gesetzt, solange der eigene Held im Dialog ist (wie CoopCanSaveNow) bzw. eine
    // Waffe gezogen hat: Local\GothicCoopTonDialog_<PID>, Local\GothicCoopTonKampf_<PID>
    static void ShowTonPulse() {
        static HANDLE dlg = NULL, kampf = NULL;
        static bool tried = false;
        static int lastDlg = -1, lastKampf = -1, logs = 0;
        if (!tried) {
            tried = true;
            char n[64];
            sprintf_s(n, "Local\\GothicCoopTonDialog_%lu", GetCurrentProcessId());
            dlg = CreateEventA(NULL, TRUE, FALSE, n);
            sprintf_s(n, "Local\\GothicCoopTonKampf_%lu", GetCurrentProcessId());
            kampf = CreateEventA(NULL, TRUE, FALSE, n);
            ShowLog("Ton: Ereignisse fuer aufnahme.exe %s (Dialog/Kampf, Prozess %lu)", dlg && kampf ? "angelegt" : "NICHT angelegt",
                GetCurrentProcessId());
        }
        if (!dlg || !kampf)
            return;
        const bool imSpiel = player && ogame && !IsLoadingLevel;
        const int d = imSpiel && (IsPlayerTalkingWithAnybody() || !oCInformationManager::GetInformationManager().IsDone) ? 1 : 0;
        const int k = imSpiel && player->GetWeaponMode() != NPC_WEAPON_NONE ? 1 : 0;
        if (d != lastDlg) {
            d ? SetEvent(dlg) : ResetEvent(dlg);
            lastDlg = d;
            if (++logs <= 30)
                ShowLog("Ton: Dialog %s", d ? "beginnt" : "vorbei");
        }
        if (k != lastKampf) {
            k ? SetEvent(kampf) : ResetEvent(kampf);
            lastKampf = k;
            if (++logs <= 30)
                ShowLog("Ton: Waffe %s", k ? "gezogen" : "weg");
        }
    }

    // Menue- und Spieltakt der Aufnahme (CoopAutoTest.cpp): einmal, sobald das Fenster da ist; u180 jedes Mal ShowTonPulse
    void CoopShowKeepActive() {
        if (!CoopRecordMode())
            return;
        ShowTonPulse();
        if (ShowPrevProc || !hWndApp)
            return;
        ShowPrevProc = (WNDPROC)SetWindowLongPtrA(hWndApp, GWLP_WNDPROC, (LONG_PTR)&ShowWndProc);
        if (!ShowPrevProc) {
            ShowLog("Fensterfunktion nicht ersetzt (Fehler %lu) - ohne Fokus zeichnet Gothic 1 evtl. nicht", GetLastError());
            return;
        }
        CallWindowProcA(ShowPrevProc, hWndApp, WM_ACTIVATEAPP, TRUE, 0);
        CallWindowProcA(ShowPrevProc, hWndApp, WM_ACTIVATE, WA_ACTIVE, 0);
        ShowLog("Fenster bleibt fuer Gothic aktiv (Aufnahme - Gothic 1 zeichnet sonst ohne Fokus nicht)");
    }

    // Host nach einer abgebrochenen Vorfuehrung (CoopAutoTest.cpp, CA_LOADING): steht die COOP-Schwierigkeit offen (MENU_COOP_NG -
    // "Neues Spiel" mit verbundenem Mitspieler), "Spiel starten" waehlen. Rueckgabe true = RETURN gegeben
    bool CoopShowStartNg() {
        zCMenu* menu = zCMenu::GetActive();
        if (!menu || !ShowNameIn(menu->name, "MENU_COOP_NG"))
            return false;
        const int at = ShowFind(menu, "MENUITEM_COOPNG_START");
        if (at < 0)
            return false;
        ShowSelectDirect(menu, at);
        return ShowActivate(menu);
    }

    // role 1 Host, 2 Client
    void CoopShowBegin(int role) {
        Show = {};
        Show.steps = role == 1 ? ShowHost : ShowClient;
        Show.count = role == 1 ? (int)(sizeof(ShowHost) / sizeof(ShowHost[0])) : (int)(sizeof(ShowClient) / sizeof(ShowClient[0]));
        Show.on = true;
        Show.nextMs = GetCurrentMs();
        ShowLog("beginnt (%s, %d Schritte)", role == 1 ? "Host" : "Client", Show.count);
    }

    // Game_MenuLoop (ueber CoopAutoTestMenuPulse): Rueckgabe 0 laeuft, 1 fertig (Spiel bzw. Verbindung beginnt), -1 abgebrochen
    int CoopShowPulse() {
        if (!Show.on)
            return Show.result;
        const long long now = GetCurrentMs();
        if (now < Show.nextMs)
            return 0;
        zCMenu* menu = zCMenu::GetActive();
        if (!menu) {
            Show.nextMs = now + 200;
            return 0;
        }
        if (Show.idx >= Show.count) {
            if (!Show.fail)
                return ShowFinish(1, "Ende der Liste");
            // Fehler ohne folgendes SH_MAIN: zurueck ins Hauptmenue
            if (ShowNameIn(menu->name, "MENU_MAIN"))
                return ShowFinish(-1, "Abschnitt gescheitert");
            if (++Show.presses > 8)
                return ShowFinish(-1, "Hauptmenue nicht erreicht");
            ShowBack(menu);
            Show.nextMs = now + 600;
            return 0;
        }
        const CoopShowStep& s = Show.steps[Show.idx];
        if (Show.fail && s.op != SH_MAIN) {
            Show.idx++;
            return 0;
        }
        switch (s.op) {
        case SH_WAIT:
            ShowNext(now, s.a);
            return 0;
        case SH_PART:
            Show.fail = false;
            Show.required = s.a != 0;
            Show.part = s.arg ? s.arg : "?";
            ShowLog("Abschnitt %s (Menue %s)", Show.part.c_str(), menu->name.ToChar());
            ShowNext(now, 0);
            return 0;
        case SH_MENU:
            if (ShowNameIn(menu->name, s.arg)) {
                ShowNext(now, 100);
                return 0;
            }
            if (!Show.stepMs)
                Show.stepMs = now;
            if (now > Show.stepMs + s.a)
                return ShowFail(menu, s, "Menue kam nicht", now);
            Show.nextMs = now + 100;
            return 0;
        case SH_SEL: {
            const int want = ShowFind(menu, s.arg);
            if (want < 0)
                return ShowFail(menu, s, "Eintrag fehlt", now);
            if (Show.moves > 30)
                return ShowFail(menu, s, "Eintrag nicht erreicht", now);
            if (ShowMoveTo(menu, want, now))
                ShowNext(now, 250);
            return 0;
        }
        case SH_KEY:
            if (Show.presses >= s.n) {
                ShowNext(now, 100);
                return 0;
            }
            if (s.a == KEY_RETURN || s.a == KEY_ESCAPE) {   // Enter/Escape ueber ForceSelAction (siehe ShowActivate)
                if (!(s.a == KEY_RETURN ? ShowActivate(menu) : ShowBack(menu))) {
                    if (!Show.stepMs)
                        Show.stepMs = now;
                    if (now > Show.stepMs + 5000)
                        return ShowFail(menu, s, s.a == KEY_RETURN ? "Eintrag ohne Aktion bzw. Menue beschaeftigt" : "Menue beschaeftigt", now);
                    Show.nextMs = now + 100;
                    return 0;
                }
            }
            else
                CoopMenuPushKey(s.a);
            Show.presses++;
            Show.nextMs = now + 450;
            return 0;
        case SH_CHOICE_TO: {
            const int at = ShowFind(menu, s.arg);
            if (at < 0)
                return ShowFail(menu, s, "Eintrag fehlt", now);
            if (Show.moves > 30)
                return ShowFail(menu, s, "Eintrag nicht erreicht", now);
            if (!ShowMoveTo(menu, at, now))
                return 0;
            zCMenuItem* next = at + 1 < menu->m_listItems.GetNum() ? menu->m_listItems[at + 1] : NULL;
            if (!next || next->m_parType != MENU_ITEM_CHOICEBOX) {
                ShowLog("%s: kein Wahlfeld dahinter - nur markiert", s.arg);
                ShowNext(now, 300);
                return 0;
            }
            const int cur = ((zCMenuItemChoice*)next)->option;
            if (cur == s.a || Show.presses >= 6) {
                ShowLog("%s: Wert %d%s", s.arg, cur, Show.presses ? "" : " (stand schon so)");
                ShowNext(now, 300);
                return 0;
            }
            CoopMenuPushKey(KEY_RIGHT);
            Show.presses++;
            Show.nextMs = now + 600;
            return 0;
        }
        case SH_PEER:
            if (CoopLobbyPeersReadyNow()) {
                ShowLog("Mitspieler verbunden und abgeglichen nach %lld s", Show.stepMs ? (now - Show.stepMs) / 1000 : 0);
                ShowNext(now, 0);
                return 0;
            }
            if (!Show.stepMs) {
                Show.stepMs = now;
                ShowLog("warte auf den Mitspieler (Menue %s)", menu->name.ToChar());
            }
            if (now > Show.stepMs + s.a)
                return ShowFail(menu, s, "kein Mitspieler", now);
            Show.nextMs = now + 250;
            return 0;
        case SH_MAIN:
            if (ShowNameIn(menu->name, "MENU_MAIN")) {
                if (Show.fail && Show.required)
                    return ShowFinish(-1, "Pflichtabschnitt gescheitert");
                Show.fail = false;
                ShowNext(now, 400);
                return 0;
            }
            if (++Show.presses > 8)
                return ShowFinish(-1, "Hauptmenue nicht erreicht");
            ShowBack(menu);
            Show.nextMs = now + 600;
            return 0;
        case SH_NGPICK: {
            if (ShowNameIn(menu->name, "MENU_COOP_NG")) {
                ShowNext(now, 100);
                return 0;
            }
            if (!Show.stepMs)
                Show.stepMs = now;
            if (ShowNameIn(menu->name, "MENU_COOP")) {   // noch im COOP-Menue - das naechste Menue kommt gleich
                if (now > Show.stepMs + s.a)
                    return ShowFail(menu, s, "nach Neues Spiel kam kein Menue", now);
                Show.nextMs = now + 100;
                return 0;
            }
            // eigenes Menue der Mod (Othello: MENU_NG mit der Schwierigkeit) - Eintrag suchen, dessen Aktion arg enthaelt
            int pick = -1;
            for (int i = 0; i < menu->m_listItems.GetNum() && pick < 0; i++) {
                zCMenuItem* it = menu->m_listItems[i];
                if (!it)
                    continue;
                std::string act = it->m_parOnSelAction_S[0].ToChar();
                for (auto& c : act)
                    c = (char)toupper((unsigned char)c);
                if (s.arg && act.find(s.arg) != std::string::npos)
                    pick = i;
            }
            if (Show.presses == 0) {
                ShowLog("Menue %s vor der COOP-Schwierigkeit - waehle %s", menu->name.ToChar(), pick >= 0 ? menu->m_listItems[pick]->id.ToChar() : "den markierten Eintrag");
                if (pick >= 0)
                    ShowSelectDirect(menu, pick);
                Show.presses = 1;
                Show.nextMs = now + 1500;   // die Wahl sichtbar lassen
                return 0;
            }
            if (!ShowActivate(menu)) {
                if (now > Show.stepMs + s.a + 5000)
                    return ShowFail(menu, s, "Auswahl der Mod ohne Aktion", now);
                Show.nextMs = now + 100;
                return 0;
            }
            ShowNext(now, 300);
            return 0;
        }
        case SH_LOOK: {
            const std::string note = CoopLookTakeNpc(s.arg);
            ShowLog("Aussehen wie %s: %s", s.arg ? s.arg : "?", note.empty() ? "(keine Rueckmeldung)" : note.c_str());
            if (!note.empty())
                menu->PrintInformation(zSTRING(note.c_str()));   // Hinweiszeile des Menues wie nach dem Eingabefeld
            ShowNext(now, 300);
            return 0;
        }
        case SH_LAST:
            if (!ShowActivate(menu)) {
                if (!Show.stepMs)
                    Show.stepMs = now;
                if (now > Show.stepMs + 5000)
                    return ShowFail(menu, s, "Eintrag ohne Aktion bzw. Menue beschaeftigt", now);
                Show.nextMs = now + 100;
                return 0;
            }
            return ShowFinish(1, s.arg ? s.arg : "Ende");
        }
        Show.idx++;
        return 0;
    }

#else
    bool CoopShowActive() { return false; }
    void CoopShowBegin(int) {}
    int CoopShowPulse() { return -1; }
    bool CoopShowStartNg() { return false; }
    void CoopShowKeepActive() {}   // u175
#endif
}
