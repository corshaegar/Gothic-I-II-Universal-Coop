// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- v81: COOP-Eintrag zur Laufzeit in Othellos Menues einhaengen --------------------------------------------------------
    //
    // Bis v80 ersetzte COOP_MENU.D Othellos MENU_MAIN und MENU_OPTIONS als ganze Kopie - das war Othello-Skript in unserer Datei,
    // und eine Aenderung Othellos an diesen Menues waere ueberschrieben worden. Nutzer (21.10.2026): "COOP-Eintrag zur Laufzeit in
    // Othellos Hauptmenue und Einstellungsmenue einhaengen, COOP_MENU.D nur noch mit unseren eigenen Menues". Jetzt bleiben
    // Othellos Menues, wie sie sind; GT setzt nach dem Aufbau aus dem Skript seine Eintraege in die Eintragsliste:
    // - MENU_MAIN: MENUITEM_COOPMAIN_COOP vor "Neues Spiel". Othello: NEWGAME, SAVEGAME_LOAD, OPTIONS, INTRO, CREDITS, EXIT auf
    //   MENU_MAIN_Y 2500 + MENU_MAIN_DY 550 * 1..6 - sie ruecken um 550 nach unten (nur, wenn sie genau dort stehen; ein erneut
    //   benutzter, schon verschobener Eintrag bleibt). Dazu MENUITEM_COOPMAIN_VERSION am Ende (Fassungszeile, v58).
    // - MENU_OPTIONS: MENUITEM_OPT_COOP an die Stelle von MENUITEM_PERF (MENU_OPT_START_Y 1600 + MENU_OPT_DY 600 * 6, Wunsch des
    //   Nutzers seit v55: "Performance/Qualitaet" durch COOP ersetzen), MENUITEM_PERF_CHOICE faellt aus der Liste.
    // Engine G1 (Speicherabbild geprueft): zCMenu::SetByScript 0x004CE720 parst die Instanz (zCParser::CreateInstance 0x006E9B60
    // in die Felder ab +0x04, itemID[150] als zSTRING ab +0x58) und legt je nicht leerem itemID[i] mit zCMenuItem::Create
    // 0x004D1970 einen Eintrag am Ende von m_listItems (+0xCB8 {Feld, Platz, Anzahl}) an; gezeichnet und positioniert wird erst in
    // ScreenInit (m_parPosY +0x2D4). Othello hat den Eingang von SetByScript durch einen Sprung nach OTHELLO_ABI+0x9CC60 ersetzt
    // (ruft das Original und haengt sich dabei kurz an 0x004CE7E5) - der Platz in der zCMenu-vtable 0x007D2ACC (+0x40) ist
    // unveraendert 0x004CE720. Aufgerufen wird er von zCMenu::Create 0x004CDFC0 (MENU_MAIN aus CGameManager::Menu) und von
    // zCMenu::HandleSelAction 0x004CF951 (SEL_ACTION_STARTMENU, z. B. MENU_OPTIONS). GT biegt den Platz um und arbeitet nach dem
    // bisherigen Inhalt. Die Eintragsliste waechst nur ueber die Engine (zCArray<zCMenuItem*>::AllocDelta 0x004D14A0) - sonst
    // gaebe ~zCMenu spaeter Speicher frei, den GTs Laufzeit angelegt hat. Weg allgemein G1, Namen und Positionen Othello 4.4.2.1.

#if COOP_BUILD(MenuInject)   // u3: bis u2 nur G1 - Freigabe je Engine in CoopCore.cpp

    typedef void(__thiscall* CoopSetByScriptFn)(zCMenu*, const zSTRING&);
    static CoopSetByScriptFn CoopOrigSetByScript = NULL;

    static void InjectLog(const char* fmt, ...) {
        char buf[600];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Menue-Einhaengen: %s", DiagSeconds(), DiagRole(), buf);
    }

    // zCArray<zCMenuItem*>: Feld, Platz, Anzahl
    struct InjectArray {
        zCMenuItem** items;
        int alloc;
        int num;
    };

    static InjectArray& InjectList(zCMenu* menu) {
        return *(InjectArray*)&menu->m_listItems;
    }

    static int InjectFind(zCMenu* menu, const char* id) {
        InjectArray& a = InjectList(menu);
        for (int i = 0; i < a.num; i++)
            if (a.items[i] && LobbyUpper(a.items[i]->id) == id)
                return i;
        return -1;
    }

    // Eintrag aus unserem Menue-Skript - nur, wenn die Instanz dort steht (ohne sie legte zCMenuItem::Create einen leeren an)
    static zCMenuItem* InjectCreate(const char* id) {
        zCParser* par = zCMenu::GetParser();
        if (!par || !par->GetSymbol(zSTRING(id)))
            return NULL;
        return zCMenuItem::Create(zSTRING(id));
    }

    static bool InjectInsert(zCMenu* menu, int pos, zCMenuItem* item) {
        InjectArray& a = InjectList(menu);
        if (pos < 0 || pos > a.num)
            return false;
        if (a.alloc < a.num + 1)
            ((void(__thiscall*)(void*, int))CoopAddr::zCArray_zCMenuItem_AllocDelta)(&menu->m_listItems, 16);   // zCArray<zCMenuItem*>::AllocDelta
        if (a.alloc < a.num + 1 || !a.items)
            return false;
        for (int i = a.num; i > pos; i--)
            a.items[i] = a.items[i - 1];
        a.items[pos] = item;
        a.num++;
        // itemID (Skriptnamen) gleich halten - Othellos Listen haben keine Luecken, Index = Listenplatz
        for (int i = 149; i > pos; i--)
            menu->itemID[i] = menu->itemID[i - 1];
        menu->itemID[pos] = item->id;
        return true;
    }

    static void InjectRemove(zCMenu* menu, int pos) {
        InjectArray& a = InjectList(menu);
        if (pos < 0 || pos >= a.num)
            return;
        for (int i = pos; i < a.num - 1; i++)
            a.items[i] = a.items[i + 1];
        a.num--;
        for (int i = pos; i < 149; i++)
            menu->itemID[i] = menu->itemID[i + 1];
        menu->itemID[149] = zSTRING("");
    }

    static const int INJECT_MAIN_Y = 2500, INJECT_MAIN_DY = 550;
    static const int INJECT_OPT_Y = 1600, INJECT_OPT_DY = 600;

    // u21: Zeilen von MENU_MAIN beim ersten Aufbau nach dem Einhaengen (je Eintrag, siehe MainShowForState)
    static std::map<std::string, int> MainRowY;

    // u22 (Test u21, Nutzer: "Es hat das eine Mal in ESC die Coop-Sachen gefehlt"; Log: 37 s nach dem Laden "Hauptmenue anders als
    // erwartet (MENUITEM_MAIN_OPTIONS da, y 4150 statt 4700) - COOP nicht eingehaengt"): Gothic baute MENU_MAIN nach dem Laden neu
    // (neues zCMenu, SetByScript), die Eintraege sind aber dieselben Objekte (zCMenuItem::Create nimmt vorhandene) - sie standen
    // noch auf den Zeilen, auf die MainShowForState sie im Startmenue zusammengerueckt hatte. InjectMain erkannte das Menue nicht und
    // haengte weder COOP noch "Spielstand waehlen"/"Hauptmenue" ein. Jetzt bekommen bekannte Eintraege vor dem Einhaengen ihre
    // gemerkte Zeile zurueck (= Lage nach dem ersten Einhaengen, die InjectMain als "schon verschoben" kennt). Fehler aus u21,
    // allgemein (Vanilla G1/G2).
    static int MainRestoreRows(zCMenu* menu) {
        InjectArray& a = InjectList(menu);
        int n = 0;
        for (int i = 0; i < a.num; i++) {
            zCMenuItem* it = a.items[i];
            if (!it)
                continue;
            auto f = MainRowY.find(LobbyUpper(it->id));
            if (f != MainRowY.end() && it->m_parPosY != f->second) {
                it->m_parPosY = f->second;
                n++;
            }
        }
        return n;
    }
    static const char* const InjectMainItems[] = { "MENUITEM_MAIN_NEWGAME", "MENUITEM_MAIN_SAVEGAME_LOAD", "MENUITEM_MAIN_OPTIONS",
        "MENUITEM_MAIN_INTRO", "MENUITEM_MAIN_CREDITS", "MENUITEM_MAIN_EXIT" };
    // u5: Gothic 1 selbst (MDK Menu_Main.d) - ein Menue fuer Start und Spiel: NEWGAME .. EXIT auf MENU_MAIN_Y + DY * 0..7, dazu
    // SAVEGAME_SAVE und RESUME (nur im Spiel). COOP kommt auch hier vor "Neues Spiel" (im Skript nur ausserhalb des Spiels).
    static const char* const InjectMainItemsVanilla[] = { "MENUITEM_MAIN_NEWGAME", "MENUITEM_MAIN_SAVEGAME_LOAD",
        "MENUITEM_MAIN_SAVEGAME_SAVE", "MENUITEM_MAIN_RESUME", "MENUITEM_MAIN_OPTIONS", "MENUITEM_MAIN_INTRO", "MENUITEM_MAIN_CREDITS",
        "MENUITEM_MAIN_EXIT" };

    // u19: Gothic 1/2 ohne Mod - das Esc-Menue ist MENU_MAIN selbst (Othello hat dafuer MENU_MAIN_INGAME, InjectIngame unten). Bis u18
    // kamen "Hauptmenue" und "Spielstand waehlen" dort nie an. Jetzt beide nur im Spiel (IT_ONLY_IN_GAME): "Spielstand waehlen" auf
    // der Zeile von COOP (die gilt nur ausserhalb), "Hauptmenue" unter "Beenden". Listenfolge = Tastenfolge: LOAD direkt nach COOP,
    // TITLE direkt nach EXIT. Allgemein (Vanilla).
    // u25 (Nutzer: "Hauptmenue ueber Gothic beenden"): TITLE jetzt direkt VOR EXIT in der Liste; die Zeilen setzt MainShowForState
    // seit u25 nach der Listenfolge, die Lage (exitY + DY) zaehlt nur noch als Zeile des Rasters.
    static void InjectIngameVanilla(zCMenu* menu) {
        if (InjectFind(menu, "MENUITEM_COOPINGAME_TITLE") >= 0 || InjectFind(menu, "MENUITEM_COOPINGAME_LOAD") >= 0)
            return;
        InjectArray& a = InjectList(menu);
        const int coop = InjectFind(menu, "MENUITEM_COOPMAIN_COOP");
        const int exit = InjectFind(menu, "MENUITEM_MAIN_EXIT");
        if (coop < 0 || exit < 0)
            return;
        const int coopY = a.items[coop]->m_parPosY, exitY = a.items[exit]->m_parPosY;
        zCMenuItem* load = InjectCreate("MENUITEM_COOPINGAME_LOAD");
        if (load) {
            load->m_parItemFlags = (load->m_parItemFlags & ~IT_ONLY_OUT_GAME) | IT_ONLY_IN_GAME;
            load->m_parPosY = coopY;
            if (InjectInsert(menu, coop + 1, load)) {
                if (menu->defaultInGame > coop)
                    menu->defaultInGame++;
                if (menu->defaultOutGame > coop)
                    menu->defaultOutGame++;
            }
            else
                load = NULL;
        }
        zCMenuItem* title = InjectCreate("MENUITEM_COOPINGAME_TITLE");
        const int exit2 = InjectFind(menu, "MENUITEM_MAIN_EXIT");
        if (title && exit2 >= 0 && exitY + INJECT_MAIN_DY <= 8192 - 300) {
            title->m_parItemFlags = (title->m_parItemFlags & ~IT_ONLY_OUT_GAME) | IT_ONLY_IN_GAME;
            title->m_parPosY = exitY + INJECT_MAIN_DY;
            if (InjectInsert(menu, exit2, title)) {   // u25: vor "Beenden"
                if (menu->defaultInGame >= exit2)
                    menu->defaultInGame++;
                if (menu->defaultOutGame >= exit2)
                    menu->defaultOutGame++;
            }
            else
                title = NULL;
        }
        else
            title = NULL;
        static int logged = 0;
        if (logged++ < 5)
            InjectLog("Esc-Men\xFC (Gothics MENU_MAIN): \"Spielstand w\xE4hlen\" %s (Zeile von COOP, y %d), \"Hauptmen\xFC\" %s (vor \"Beenden\") - "
                "jetzt %d Eintr\xE4ge", load ? "eingeh\xE4ngt" : "FEHLT", coopY, title ? "eingeh\xE4ngt" : "FEHLT", a.num);
    }

    static void InjectMain(zCMenu* menu) {
        if (InjectFind(menu, "MENUITEM_COOPMAIN_COOP") >= 0)
            return;   // schon da (erneut aufgebaut, oder ein altes COOP_MENU.D mit eigenem MENU_MAIN)
        const bool othello = CoopProfileIsOthello();
        const char* const* names = othello ? InjectMainItems : InjectMainItemsVanilla;
        const int count = othello ? 6 : 8;
        const int row0 = othello ? 1 : 0;   // Zeile von "Neues Spiel": Othello MENU_MAIN_Y + DY * 1, Gothic 1 + DY * 0
        InjectArray& a = InjectList(menu);
        const int restored = othello ? 0 : MainRestoreRows(menu);   // u22
        if (restored)
            InjectLog("Hauptmen\xFC neu aufgebaut: %d Eintr\xE4ge zur\xFC" "ck auf ihre gemerkte Zeile (vom Zusammenr\xFC" "cken)", restored);
        const int first = InjectFind(menu, names[0]);
        if (first < 0) {
            InjectLog("Hauptmen\xFC ohne %s (%d Eintr\xE4ge) - COOP nicht eingeh\xE4ngt", names[0], a.num);
            return;
        }
        zCMenuItem* items[8];
        for (int k = 0; k < count; k++) {
            const int i = InjectFind(menu, names[k]);
            items[k] = i >= 0 ? a.items[i] : NULL;
            const int y = items[k] ? items[k]->m_parPosY : 0;
            const int orig = INJECT_MAIN_Y + INJECT_MAIN_DY * (k + row0);
            if (!items[k] || (y != orig && y != orig + INJECT_MAIN_DY)) {
                InjectLog("Hauptmen\xFC anders als erwartet (%s %s, y %d statt %d) - COOP nicht eingeh\xE4ngt", names[k],
                    items[k] ? "da" : "fehlt", y, orig);
                return;
            }
        }
        zCMenuItem* coop = InjectCreate("MENUITEM_COOPMAIN_COOP");
        if (!coop) {
            InjectLog("MENUITEM_COOPMAIN_COOP fehlt im Men\xFC-Skript (COOP_MENU.D nicht geladen?) - Hauptmen\xFC unver\xE4ndert");
            return;
        }
        if (!InjectInsert(menu, first, coop)) {
            InjectLog("Hauptmen\xFC: Eintrag lie\xDF sich nicht einf\xFCgen (%d/%d) - unver\xE4ndert", a.num, a.alloc);
            return;
        }
        int moved = 0;
        for (int k = 0; k < count; k++) {
            const int orig = INJECT_MAIN_Y + INJECT_MAIN_DY * (k + row0);
            if (items[k]->m_parPosY == orig) {
                items[k]->m_parPosY = orig + INJECT_MAIN_DY;
                moved++;
            }
        }
        // Gothic 1: Vorauswahl ist ein Listenplatz - im Spiel weiter "Spiel speichern" (defaultInGame), beim Start COOP auf dem
        // alten Platz von "Neues Spiel" (wie unter Othello)
        if (!othello && menu->defaultInGame > first)
            menu->defaultInGame++;
        bool version = InjectFind(menu, "MENUITEM_COOPMAIN_VERSION") >= 0;
        if (!version) {
            zCMenuItem* v = InjectCreate("MENUITEM_COOPMAIN_VERSION");
            version = v && InjectInsert(menu, a.num, v);
        }
        if (!othello && COOP_READY_Title)   // u19: Esc-Menue ohne Mod (siehe oben)
            InjectIngameVanilla(menu);
        static int logged = 0;
        if (logged++ < 5)
            InjectLog("Hauptmen\xFC: COOP vor \"Neues Spiel\" eingeh\xE4ngt (Platz %d), %d Eintr\xE4ge %s um %d nach unten, "
                "Versionszeile %s - jetzt %d Eintr\xE4ge", first, moved, othello ? "Othellos" : "von Gothic", INJECT_MAIN_DY,
                version ? "da" : "FEHLT", a.num);
    }

    static void InjectOptions(zCMenu* menu) {
        if (InjectFind(menu, "MENUITEM_OPT_COOP") >= 0)
            return;
        InjectArray& a = InjectList(menu);
        const int perf = InjectFind(menu, "MENUITEM_PERF");
        // u8: Gothic 2 hat einen Eintrag mehr (MENUITEM_OPT_EXT) und MENU_OPT_START_Y 2000 - PERF steht dort auf 2000 + 600 * 6 = 5600,
        // in Gothic 1 und Othello auf 5200. COOP uebernimmt die Lage von PERF (unter Othello/G1 dieselbe wie im Skript).
        const int want = COOP_ENGINE_SWITCH(INJECT_OPT_Y + INJECT_OPT_DY * 6, INJECT_OPT_Y + INJECT_OPT_DY * 6, 2000 + INJECT_OPT_DY * 6,
            2000 + INJECT_OPT_DY * 6);
        if (perf < 0 || a.items[perf]->m_parPosY != want) {
            InjectLog("Einstellungen anders als erwartet (MENUITEM_PERF %s, y %d statt %d) - COOP nicht eingeh\xE4ngt",
                perf < 0 ? "fehlt" : "da", perf < 0 ? 0 : a.items[perf]->m_parPosY, want);
            return;
        }
        zCMenuItem* coop = InjectCreate("MENUITEM_OPT_COOP");
        if (!coop) {
            InjectLog("MENUITEM_OPT_COOP fehlt im Men\xFC-Skript (COOP_MENU.D nicht geladen?) - Einstellungen unver\xE4ndert");
            return;
        }
        coop->m_parPosY = want;
        a.items[perf] = coop;
        menu->itemID[perf] = coop->id;
        const int choice = InjectFind(menu, "MENUITEM_PERF_CHOICE");
        if (choice >= 0)
            InjectRemove(menu, choice);
        static int logged = 0;
        if (logged++ < 5)
            InjectLog("Einstellungen: COOP statt \"Performance/Qualit\xE4t\" (Platz %d)%s - jetzt %d Eintr\xE4ge", perf,
                choice >= 0 ? ", Auswahlfeld entfernt" : "", a.num);
    }

    // v82: Esc-Menue im Spiel (Othello MENU_MAIN_INGAME: RESUME, LOAD_INGAME, SAVE, OPTIONS_INGAME, EXIT_INGAME) - "Hauptmenue"
    // (MENUITEM_COOPINGAME_TITLE, CoopTitle.cpp) an die Stelle von "Gothic beenden", das um 550 nach unten rueckt.
    // v83: v82 erwartete EXIT_INGAME auf 2500 + 550 * 5 = 5250; Othello 4.4.2.1 setzt es auf 5525 (Log 21.10.2026, 23:41: "y 5525
    // statt 5250") - der Eintrag wurde nie eingehaengt, Nutzer: "hab kein hauptmenü gesehen". Jetzt gilt die Stelle, an der
    // EXIT_INGAME beim ersten Aufbau steht; ein schon verschobener (wieder benutzter) Eintrag steht 550 tiefer und bleibt.
    // v91 (Nutzer: "wie im Verbindungsmenue gibt's ja dieses Spielstand waehlen. Das brauchen wir noch im Spiel bei Esc"): dazu
    // "Spielstand waehlen" (MENUITEM_COOPINGAME_LOAD -> MENU_COOP_LOAD) hinter "Spiel laden". Lagen aus Othellos erstem Aufbau
    // (Eintraege werden wiederverwendet und behalten unsere Lage): Eintraege zwischen "Spiel laden" und "Gothic beenden" +550,
    // "Hauptmenue" an Othellos Stelle von "Gothic beenden" +550, "Gothic beenden" +1100 (Othello 4.4.2.1: 5525 -> 6625).
    static std::map<std::string, int> IngameOrigY;

    static int IngameOrig(zCMenuItem* it) {
        const std::string id = LobbyUpper(it->id);
        auto f = IngameOrigY.find(id);
        if (f != IngameOrigY.end())
            return f->second;
        IngameOrigY[id] = it->m_parPosY;
        return it->m_parPosY;
    }

    static void InjectIngame(zCMenu* menu) {
        InjectArray& a = InjectList(menu);
        int exit = InjectFind(menu, "MENUITEM_MAIN_EXIT_INGAME");
        int load = InjectFind(menu, "MENUITEM_MAIN_SAVEGAME_LOAD_INGAME");
        if (exit < 0 || load < 0) {
            static int warned = 0;
            if (warned++ < 5)
                InjectLog("Esc-Men\xFC anders als erwartet (EXIT_INGAME %s, SAVEGAME_LOAD_INGAME %s) - nichts eingeh\xE4ngt",
                    exit < 0 ? "fehlt" : "da", load < 0 ? "fehlt" : "da");
            return;
        }
        for (int i = 0; i < a.num; i++)   // Othellos Lagen beim ersten Aufbau merken (unsere Eintraege kommen danach)
            if (a.items[i] && LobbyUpper(a.items[i]->id).compare(0, 19, "MENUITEM_COOPINGAME") != 0)
                IngameOrig(a.items[i]);
        const int exitY = IngameOrig(a.items[exit]);
        const int loadY = IngameOrig(a.items[load]);
        if (exitY < INJECT_MAIN_Y || exitY > 8192 - 4 * INJECT_MAIN_DY || loadY >= exitY) {
            static int warned = 0;
            if (warned++ < 5)
                InjectLog("Esc-Men\xFC anders als erwartet (Spiel laden y %d, Gothic beenden y %d) - nichts eingeh\xE4ngt", loadY, exitY);
            return;
        }
        zCMenuItem* title = NULL;
        zCMenuItem* coopLoad = NULL;
        int t = InjectFind(menu, "MENUITEM_COOPINGAME_TITLE");
        if (t < 0) {
            title = InjectCreate("MENUITEM_COOPINGAME_TITLE");
            if (title && !InjectInsert(menu, exit, title))
                title = NULL;
        }
        else
            title = a.items[t];
        load = InjectFind(menu, "MENUITEM_MAIN_SAVEGAME_LOAD_INGAME");
        int l = InjectFind(menu, "MENUITEM_COOPINGAME_LOAD");
        if (l < 0) {
            coopLoad = InjectCreate("MENUITEM_COOPINGAME_LOAD");
            if (coopLoad && !InjectInsert(menu, load + 1, coopLoad))
                coopLoad = NULL;
            // Vorauswahl ist ein Listenplatz (defaultingame/defaultoutgame) - beim selben Eintrag lassen
            if (coopLoad) {
                if (menu->defaultInGame > load)
                    menu->defaultInGame++;
                if (menu->defaultOutGame > load)
                    menu->defaultOutGame++;
            }
        }
        else
            coopLoad = a.items[l];
        const int dy = INJECT_MAIN_DY;
        const int shift = coopLoad ? dy : 0;
        for (int i = 0; i < a.num; i++) {
            zCMenuItem* it = a.items[i];
            if (!it)
                continue;
            const std::string id = LobbyUpper(it->id);
            if (it == title)
                it->m_parPosY = exitY + shift;   // im Skript MENU_MAIN_Y + DY * 5 - zaehlt nicht
            else if (it == coopLoad)
                it->m_parPosY = loadY + dy;
            else if (id == "MENUITEM_MAIN_EXIT_INGAME")
                it->m_parPosY = exitY + shift + (title ? dy : 0);
            else {
                const int o = IngameOrig(it);
                it->m_parPosY = (o > loadY && o < exitY) ? o + shift : o;
            }
        }
        static int logged = 0;
        if (logged++ < 5)
            InjectLog("Esc-Men\xFC: \"Spielstand w\xE4hlen\" %s, \"Hauptmen\xFC\" %s (Spiel laden y %d, Gothic beenden jetzt y %d) - jetzt %d Eintr\xE4ge",
                coopLoad ? "eingeh\xE4ngt" : "FEHLT (COOP_MENU.D zu alt?)", title ? "eingeh\xE4ngt" : "FEHLT", loadY,
                a.items[InjectFind(menu, "MENUITEM_MAIN_EXIT_INGAME")]->m_parPosY, a.num);
    }

    // u21 (Test u20, Nutzer: "im Hauptmenue waren die Eintraege ausgegraut zu sehen, die bei ESC zu sehen sind im Spiel, und
    // andersrum"): Gothic graut Eintraege mit IT_ONLY_IN_GAME ausserhalb und IT_ONLY_OUT_GAME im Spiel nur aus
    // (zCMenuItem::GetIsDisabled, G2A 0x004E1DF0, G1 0x004D4E30) und zeichnet sie weiter - "Spielstand waehlen" lag so ausgegraut
    // ueber COOP (gleiche Zeile, u19), "Hauptmenue" und Gothics "Spiel speichern"/"Weiterspielen" standen ausgegraut im Startmenue.
    // Engine (G2A wie G1, Speicherabbild): zCMenuItem::m_bVisible (SDK, G2A +0x3A8, G1 +0x378) = 0 -> InsertInWin (G2A 0x004DFA90)
    // haengt den Eintrag nicht ein, GetIsSelectable (G2A 0x004E1D60) = 0 (Tasten und GTs Maus springen darueber); Gothic setzt das
    // Feld nur beim Anlegen (Init, Flag 0x4000 -> SetIsVisible(0)). GT hakt zCMenu::ScreenInit (vtable +0x30, baut das Menue bei
    // jedem Oeffnen auf) ein und setzt davor in MENU_MAIN ohne Othello (Othello hat ein eigenes Esc-Menue) m_bVisible nach
    // zCMenu::inGameMenu. Die sichtbaren Zeilen ruecken lueckenlos zusammen (Raster MENU_MAIN_Y 2500 + 550 * n, Lagen vom ersten
    // Aufbau nach dem Einhaengen): im Startmenue COOP, Neues Spiel, Laden, Einstellungen, Intro, Credits, Beenden; im Spiel wie
    // bisher. Allgemein (Vanilla G1/G2).
    // u25 (Nutzer: "ESC-Menue ueberarbeiten: 'Gothic II starten' entfernen, 'Intro abspielen' entfernen, Hauptmenue ueber Gothic
    // beenden und Weiterspielen entfernen"): im Spiel zusaetzlich ausgeblendet MENUITEM_MAIN_NEWGAME, _INTRO, _RESUME (Esc fuehrt
    // weiter zurueck ins Spiel); im Startmenue bleiben "Neues Spiel" und "Intro". Die Zeilen folgen jetzt der Listenfolge (=
    // Tastenfolge, "Hauptmenue" steht seit u25 vor "Beenden"): jeder sichtbare Rasterzeilen-Eintrag bekommt die naechste Zeile.
    // Im Spiel: Spielstand waehlen, Spiel laden, Spiel speichern, Einstellungen, Credits, Hauptmenue, Beenden.
    typedef void(__thiscall* CoopScreenInitFn)(zCMenu*);
    static CoopScreenInitFn CoopOrigScreenInit = NULL;
    static const char* const MainHideInGame[] = { "MENUITEM_MAIN_NEWGAME", "MENUITEM_MAIN_INTRO", "MENUITEM_MAIN_RESUME" };

    static void MainShowForState(zCMenu* menu) {
        InjectArray& a = InjectList(menu);
        const bool inGame = zCMenu::inGameMenu != 0;
        int hidden = 0, rows = 0, moved = 0;
        for (int i = 0; i < a.num; i++) {
            zCMenuItem* it = a.items[i];
            if (!it)
                continue;
            const std::string id = LobbyUpper(it->id);
            if (!MainRowY.count(id))
                MainRowY[id] = it->m_parPosY;
            const int flags = it->m_parItemFlags;
            bool hideInGame = false;   // u25
            for (const char* h : MainHideInGame)
                hideInGame = hideInGame || id == h;
            if ((flags & (IT_ONLY_IN_GAME | IT_ONLY_OUT_GAME)) || hideInGame) {
                bool vis = (flags & IT_ONLY_IN_GAME) ? inGame : (flags & IT_ONLY_OUT_GAME) ? !inGame : true;
                if (hideInGame && inGame)
                    vis = false;
                it->m_bVisible = vis ? 1 : 0;
                if (!vis)
                    hidden++;
            }
            const int y = MainRowY[id] - INJECT_MAIN_Y;
            if (y < 0 || y % INJECT_MAIN_DY != 0 || !it->m_bVisible)
                continue;   // nicht im Raster (Ueberschrift, Fassungszeile) oder ausgeblendet
            const int want = INJECT_MAIN_Y + INJECT_MAIN_DY * rows++;
            if (it->m_parPosY != want) {
                it->m_parPosY = want;
                moved++;
            }
        }
        static int logged[2] = { 0, 0 };
        if (logged[inGame]++ < 3)
            InjectLog("Hauptmen\xFC %s: %d Eintr\xE4ge ausgeblendet, %d Zeilen sichtbar (Listenfolge), %d verschoben", inGame ? "im Spiel" : "beim Start",
                hidden, rows, moved);
    }

    static void __fastcall CoopScreenInitHook(zCMenu* menu, void* edx) {
        try {
            if (menu && !CoopProfileIsOthello() && LobbyUpper(menu->name) == "MENU_MAIN")
                MainShowForState(menu);
        }
        catch (...) {
            InjectLog("Ausnahme beim Ausblenden in MENU_MAIN - Men\xFC bleibt wie von Gothic aufgebaut");
        }
        CoopOrigScreenInit(menu);
    }

    static void __fastcall CoopSetByScriptHook(zCMenu* menu, void* edx, const zSTRING& name) {
        CoopOrigSetByScript(menu, name);
        try {
            if (!menu)
                return;
            const std::string n = LobbyUpper(name);
            if (n == "MENU_MAIN")
                InjectMain(menu);
            else if (n == "MENU_OPTIONS")
                InjectOptions(menu);
            else if (n == "MENU_MAIN_INGAME")
                InjectIngame(menu);
        }
        catch (...) {
            InjectLog("Ausnahme beim Einh\xE4ngen in %s - Men\xFC bleibt, wie Othello es aufgebaut hat", name.ToChar());
        }
    }

    // Plugin.cpp, Game_Entry: vor dem ersten Menue
    void CoopMenuInjectInstall() {
        static bool done = false;
        if (done)
            return;
        done = true;
        DWORD* const vtable = (DWORD*)CoopAddr::vt_zCMenu();   // zCMenu-vtable (G1 0x007D2ACC): +0x00 HandleEvent, +0x3C HandleSelAction
        DWORD* const slot = vtable + 0x40 / 4;                  // SetByScript - Plaetze in allen Engines gleich (CoopAddr.h)
        if (!IsReadableMemory(vtable, 0x48) || vtable[0] != CoopAddr::zCMenu_HandleEvent || vtable[0x3C / 4] != CoopAddr::zCMenu_HandleSelAction) {
            InjectLog("zCMenu-vtable nicht erkannt (%08X) - COOP-Eintr\xE4ge werden nicht eingeh\xE4ngt",
                IsReadableMemory(vtable, 4) ? vtable[0] : 0);
            return;
        }
        const DWORD before = *slot;
        DWORD old = 0;
        if (!VirtualProtect(slot, 4, PAGE_READWRITE, &old)) {
            InjectLog("VirtualProtect f\xFCr den SetByScript-Platz fehlgeschlagen (%lu)", GetLastError());
            return;
        }
        CoopOrigSetByScript = (CoopSetByScriptFn)before;
        *slot = (DWORD)&CoopSetByScriptHook;
        VirtualProtect(slot, 4, old, &old);
        char mod[96];
        CoopModuleOf((const void*)before, mod, sizeof(mod));
        InjectLog("zCMenu::SetByScript (vtable %08X) eingehakt, bisher %08X (%s)%s", (unsigned)(UINT_PTR)slot, before, mod,
            before == CoopAddr::zCMenu_SetByScript ? "" : " - NICHT das Original, wird weitergereicht");
        // u21: ScreenInit (+0x30) - Eintraege nur im Spiel/nur ausserhalb ausblenden (siehe MainShowForState)
        DWORD* const slotInit = vtable + 0x30 / 4;
        const DWORD beforeInit = *slotInit;
        if (!IsReadableMemory((void*)beforeInit, 4) || !VirtualProtect(slotInit, 4, PAGE_READWRITE, &old)) {
            InjectLog("zCMenu::ScreenInit-Platz nicht beschreibbar - Eintr\xE4ge nur im Spiel bleiben ausgegraut sichtbar");
            return;
        }
        CoopOrigScreenInit = (CoopScreenInitFn)beforeInit;
        *slotInit = (DWORD)&CoopScreenInitHook;
        VirtualProtect(slotInit, 4, old, &old);
        CoopModuleOf((const void*)beforeInit, mod, sizeof(mod));
        InjectLog("zCMenu::ScreenInit (vtable %08X) eingehakt, bisher %08X (%s)%s", (unsigned)(UINT_PTR)slotInit, beforeInit, mod,
            beforeInit == CoopAddr::zCMenu_ScreenInit ? "" : " - NICHT das Original, wird weitergereicht");
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)
    void CoopMenuInjectInstall() {}
#endif
}
