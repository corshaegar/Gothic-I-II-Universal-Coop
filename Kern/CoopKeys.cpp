// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- u226: Coop-Tasten, Coop-Hilfe, Coop-Fenster im Spiel, Einfuehrung, Hinweise (Nutzer 09.11.2026) -------------------
    //
    // Nutzer: "Hilfe Fenster ... Uebersicht ueber die Coop Funktionen ... auch Tastenzuweisungen anzeigen. Zudem soll das
    // Coop-Inventar beliebige Tasten zugelassen werden, alle Tasten des Coops sollen in den Coop Settings anpassbar sein, am
    // besten auch eine Taste fuer die Teleport Rune. Das Coop Fenster soll ueber eine Taste erreichbar sein, aber auch in den
    // Settings. ... kurz vor einem Neuen Spiel eine Kurze Coop Einfuehrung mit den Settings ... wer weitere Hilfe braucht soll
    // im Spiel 'H' druecken, oder in die Coop Settings. Ueberlege wo noch weiter solche Luecken sein koennten".
    //  - P264 Tasten: alle Coop-Tasten in CoopKeyActions, je Gothic.ini [GOTHICCOOP] coopKey<Name> = "[SHIFT+][CTRL+][ALT+]KEY_<Taste>"
    //    bzw. leer (aus). Einstellungen > COOP > Coop-Tasten (MENU_OPT_COOP_KEYS): Zeile waehlen, Taste druecken - jede Taste, mit
    //    Shift/Strg/Alt; Entf oder Rueck schaltet aus. Belegt Gothic die Taste selbst (Gothic.ini [KEYS]), steht das dabei.
    //    Vorgaben aus den bisherigen Einstellungen (coopInvKey, camWindowKey, GothicCoopConfig.json startServerKey ...). Neu:
    //    Hilfe (H; belegt Gothic H - in Gothic II der Heiltrank -, dann F1: mit Shift tranke der Held trotzdem), Coop-Fenster
    //    (Shift+K), Teleport zum Mitspieler (Shift+T, mit der Rune im Inventar).
    //  - P265 Hilfe MENU_COOP_HELP: alle Tasten mit Belegung und das Wichtigste in Saetzen - Taste im Spiel, Einstellungen > COOP.
    //  - P266 Coop-Fenster MENU_COOP per Taste im Spiel und unter Einstellungen > COOP; im Spiel gelten dort Hosten, Beitreten und
    //    Trennen (bis u225 "im Spiel ignoriert"), die Statuszeilen laufen mit.
    //  - P267 Einfuehrung MENU_COOP_INTRO vor einem neuen Spiel (Host bzw. allein; abschaltbar mit coopIntro).
    //  - P268 Luecken: Hinweis auf Hilfe/Fenster/Inventar beim ersten Spielbild, beim Hosten Port und Firewall, liegt ein Mitspieler
    //    bewusstlos, steht die Wiederbeleben-Taste dabei; die Rune-Taste ohne Rune sagt, wo sie fehlt.
    // Hilfe- und Fenster-Taste oeffnen das Menue wie Gothic die Esc-Taste: der Spieltakt erkennt sie am Tastenzustand und legt einen
    // eigenen Code in Gothics Tastenpuffer; der Haken auf CGameManager::HandleEvent (kommt ausserhalb des Zeichnens - aus dem Spieltakt,
    // der in oCGame::Render laeuft, waere das Menue verschachtelt, vgl. P239) ruft MenuEnabled und Menu wie bei Esc (G2A
    // 0x0042AE77/0x0042AE8B); im Esc-Menue geht das gewuenschte Untermenue auf, nach dessen Zurueck schliessen alle Menues
    // (CoopMenuCloseRequest). Allgemein (G1/G2).

    void CoopTeleportKey();   // CoopTeleport.cpp (P264): Rune-Taste

#if COOP_BUILD(Lobby)   // braucht die Menue-Haken aus CoopMenuLobby.cpp (G1, G2A)

    static void KeysLog(const char* fmt, ...) {
        static CoopDiagBudget budget = { 0, 0 };
        if (!CoopDiagAllow(budget, 60))
            return;
        char buf[700];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Coop-Tasten: %s", DiagSeconds(), DiagRole(), buf);
    }

    // --- Tastennamen (DirectInput-Codes = Scancodes) ---------------------------------------------------------------------
    struct CoopKeyName {
        int dik;
        const char* token;   // in Gothic.ini (wie GothicCoopConfig.json)
        const char* de;
        const char* en;
        bool layout;         // Anzeige nach der Tastaturbelegung (Z/Y, Umlaute)
    };

    static const CoopKeyName CoopKeyNames[] = {
        { 0x02, "KEY_1", "1", "1", true }, { 0x03, "KEY_2", "2", "2", true }, { 0x04, "KEY_3", "3", "3", true },
        { 0x05, "KEY_4", "4", "4", true }, { 0x06, "KEY_5", "5", "5", true }, { 0x07, "KEY_6", "6", "6", true },
        { 0x08, "KEY_7", "7", "7", true }, { 0x09, "KEY_8", "8", "8", true }, { 0x0A, "KEY_9", "9", "9", true },
        { 0x0B, "KEY_0", "0", "0", true }, { 0x0C, "KEY_MINUS", "-", "-", true }, { 0x0D, "KEY_EQUALS", "=", "=", true },
        { 0x0F, "KEY_TAB", "Tab", "Tab", false },
        { 0x10, "KEY_Q", "Q", "Q", true }, { 0x11, "KEY_W", "W", "W", true }, { 0x12, "KEY_E", "E", "E", true },
        { 0x13, "KEY_R", "R", "R", true }, { 0x14, "KEY_T", "T", "T", true }, { 0x15, "KEY_Y", "Y", "Y", true },
        { 0x16, "KEY_U", "U", "U", true }, { 0x17, "KEY_I", "I", "I", true }, { 0x18, "KEY_O", "O", "O", true },
        { 0x19, "KEY_P", "P", "P", true }, { 0x1A, "KEY_LBRACKET", "[", "[", true }, { 0x1B, "KEY_RBRACKET", "]", "]", true },
        { 0x1C, "KEY_RETURN", "Eingabe", "Enter", false },
        { 0x1E, "KEY_A", "A", "A", true }, { 0x1F, "KEY_S", "S", "S", true }, { 0x20, "KEY_D", "D", "D", true },
        { 0x21, "KEY_F", "F", "F", true }, { 0x22, "KEY_G", "G", "G", true }, { 0x23, "KEY_H", "H", "H", true },
        { 0x24, "KEY_J", "J", "J", true }, { 0x25, "KEY_K", "K", "K", true }, { 0x26, "KEY_L", "L", "L", true },
        { 0x27, "KEY_SEMICOLON", ";", ";", true }, { 0x28, "KEY_APOSTROPHE", "'", "'", true }, { 0x29, "KEY_GRAVE", "^", "`", true },
        { 0x2B, "KEY_BACKSLASH", "\\", "\\", true },
        { 0x2C, "KEY_Z", "Z", "Z", true }, { 0x2D, "KEY_X", "X", "X", true }, { 0x2E, "KEY_C", "C", "C", true },
        { 0x2F, "KEY_V", "V", "V", true }, { 0x30, "KEY_B", "B", "B", true }, { 0x31, "KEY_N", "N", "N", true },
        { 0x32, "KEY_M", "M", "M", true }, { 0x33, "KEY_COMMA", ",", ",", true }, { 0x34, "KEY_PERIOD", ".", ".", true },
        { 0x35, "KEY_SLASH", "/", "/", true }, { 0x37, "KEY_MULTIPLY", "Num *", "Num *", false },
        { 0x39, "KEY_SPACE", "Leertaste", "Space", false }, { 0x3A, "KEY_CAPITAL", "Feststell", "Caps Lock", false },
        { 0x3B, "KEY_F1", "F1", "F1", false }, { 0x3C, "KEY_F2", "F2", "F2", false }, { 0x3D, "KEY_F3", "F3", "F3", false },
        { 0x3E, "KEY_F4", "F4", "F4", false }, { 0x3F, "KEY_F5", "F5", "F5", false }, { 0x40, "KEY_F6", "F6", "F6", false },
        { 0x41, "KEY_F7", "F7", "F7", false }, { 0x42, "KEY_F8", "F8", "F8", false }, { 0x43, "KEY_F9", "F9", "F9", false },
        { 0x44, "KEY_F10", "F10", "F10", false }, { 0x45, "KEY_NUMLOCK", "Num", "Num Lock", false },
        { 0x46, "KEY_SCROLL", "Rollen", "Scroll Lock", false },
        { 0x47, "KEY_NUMPAD7", "Num 7", "Num 7", false }, { 0x48, "KEY_NUMPAD8", "Num 8", "Num 8", false },
        { 0x49, "KEY_NUMPAD9", "Num 9", "Num 9", false }, { 0x4A, "KEY_SUBTRACT", "Num -", "Num -", false },
        { 0x4B, "KEY_NUMPAD4", "Num 4", "Num 4", false }, { 0x4C, "KEY_NUMPAD5", "Num 5", "Num 5", false },
        { 0x4D, "KEY_NUMPAD6", "Num 6", "Num 6", false }, { 0x4E, "KEY_ADD", "Num +", "Num +", false },
        { 0x4F, "KEY_NUMPAD1", "Num 1", "Num 1", false }, { 0x50, "KEY_NUMPAD2", "Num 2", "Num 2", false },
        { 0x51, "KEY_NUMPAD3", "Num 3", "Num 3", false }, { 0x52, "KEY_NUMPAD0", "Num 0", "Num 0", false },
        { 0x53, "KEY_DECIMAL", "Num ,", "Num .", false }, { 0x56, "KEY_OEM_102", "<", "<", true },
        { 0x57, "KEY_F11", "F11", "F11", false }, { 0x58, "KEY_F12", "F12", "F12", false },
        { 0x9C, "KEY_NUMPADENTER", "Num Eingabe", "Num Enter", false }, { 0xB5, "KEY_DIVIDE", "Num /", "Num /", false },
        { 0xB7, "KEY_SYSRQ", "Druck", "Print", false }, { 0xC5, "KEY_PAUSE", "Pause", "Pause", false },
        { 0xC7, "KEY_HOME", "Pos1", "Home", false }, { 0xC8, "KEY_UPARROW", "Pfeil hoch", "Up", false },
        { 0xC9, "KEY_PRIOR", "Bild auf", "Page Up", false }, { 0xCB, "KEY_LEFTARROW", "Pfeil links", "Left", false },
        { 0xCD, "KEY_RIGHTARROW", "Pfeil rechts", "Right", false }, { 0xCF, "KEY_END", "Ende", "End", false },
        { 0xD0, "KEY_DOWNARROW", "Pfeil runter", "Down", false }, { 0xD1, "KEY_NEXT", "Bild ab", "Page Down", false },
        { 0xD2, "KEY_INSERT", "Einfg", "Ins", false }, { 0xD3, "KEY_DELETE", "Entf", "Del", false },
    };

    static const CoopKeyName* CoopKeyNameOf(int dik) {
        for (const CoopKeyName& n : CoopKeyNames)
            if (n.dik == dik)
                return &n;
        return NULL;
    }

    static bool CoopKeyIsModifier(int dik) {
        return dik == 0x1D || dik == 0x9D || dik == 0x2A || dik == 0x36 || dik == 0x38 || dik == 0xB8 || dik == 0xDB || dik == 0xDC;
    }

    static int CoopKeyVk(int dik) {
        return (int)MapVirtualKeyA(dik & 0x80 ? (0xE000u | (unsigned)(dik & 0x7F)) : (unsigned)dik, MAPVK_VSC_TO_VK_EX);
    }

    // Anzeigename: Buchstaben, Ziffern und Zeichen nach der Tastaturbelegung (deutsche Tastatur: Scancode 0x15 ist "Z")
    static std::string CoopKeyName1(int dik) {
        const CoopKeyName* n = CoopKeyNameOf(dik);
        if (n && n->layout) {
            const UINT vk = MapVirtualKeyA((UINT)dik, MAPVK_VSC_TO_VK);
            UINT ch = vk ? (MapVirtualKeyA(vk, MAPVK_VK_TO_CHAR) & 0x7FFF) : 0;
            if (ch >= 0x21 && ch < 0x100) {
                char b[2] = { (char)ch, 0 };
                CharUpperA(b);
                return b;
            }
        }
        if (n)
            return CoopLangEn ? n->en : n->de;
        char b[24];
        _snprintf(b, sizeof(b), CoopTr("Taste %02X", "Key %02X"), dik);
        b[sizeof(b) - 1] = 0;
        return b;
    }

    // --- Belegungen -------------------------------------------------------------------------------------------------------
    enum { CK_MOD_SHIFT = 1, CK_MOD_CTRL = 2, CK_MOD_ALT = 4 };

    struct CoopKeyBind {
        int dik = 0;
        int mods = 0;
    };

    struct CoopKeyActionDef {
        const char* id;    // Befehl COOP_KEY_<id>, Menue-Eintraege MENUITEM_COOPKEYS_LBL_/VAL_<id>
        const char* ini;   // Gothic.ini [GOTHICCOOP]
        const char* de;
        const char* en;
    };

    static const CoopKeyActionDef CoopKeyActions[CK_COUNT] = {
        { "HELP", "coopKeyHelp", "Coop-Hilfe", "Coop help" },
        { "MENU", "coopKeyMenu", "Coop-Fenster", "Coop window" },
        { "INV", "coopKeyInv", "Coop-Inventar", "Coop inventory" },
        { "RUNE", "coopKeyRune", "Teleport-Rune in die Hand", "Teleport rune to hand" },   // u238: nimmt die Rune in die Hand (bis u237: sofort teleportieren)
        { "CAM", "coopKeyCam", "Zuschau-Fenster", "Watch window" },
        { "HOST", "coopKeyHost", "Hosten", "Host" },
        { "JOIN", "coopKeyJoin", "Beitreten / Pause als Client", "Join / pause as client" },
        { "REVIVE", "coopKeyRevive", "Mitspieler wiederbeleben", "Revive partner" },
        { "REINIT", "coopKeyReinit", "Mitspieler-Figuren neu", "Respawn partner figures" },
        { "SYNC", "coopKeySync", "Welt neu abgleichen", "Resync the world" },
    };

    static CoopKeyBind CoopKeyBinds[CK_COUNT];
    static bool CoopKeyDown[CK_COUNT];
    static long long CoopKeysReadMs = 0;
    static bool CoopKeysLogged = false;

    static std::string CoopKeyToken(const CoopKeyBind& b) {
        if (!b.dik)
            return std::string();
        const CoopKeyName* n = CoopKeyNameOf(b.dik);
        char raw[16];
        _snprintf(raw, sizeof(raw), "0x%02X", b.dik);
        raw[sizeof(raw) - 1] = 0;
        return std::string(b.mods & CK_MOD_SHIFT ? "SHIFT+" : "") + (b.mods & CK_MOD_CTRL ? "CTRL+" : "") + (b.mods & CK_MOD_ALT ? "ALT+" : "") +
            (n ? n->token : raw);
    }

    static CoopKeyBind CoopKeyParse(const std::string& text) {
        CoopKeyBind b;
        std::string t = text, part;
        for (char& c : t)
            c = (char)toupper((unsigned char)c);
        size_t p = 0;
        while (p <= t.size()) {
            const size_t q = t.find('+', p);
            part = t.substr(p, q == std::string::npos ? std::string::npos : q - p);
            p = q == std::string::npos ? t.size() + 1 : q + 1;
            while (!part.empty() && part[0] == ' ')
                part.erase(0, 1);
            while (!part.empty() && part[part.size() - 1] == ' ')
                part.erase(part.size() - 1);
            if (part == "SHIFT") b.mods |= CK_MOD_SHIFT;
            else if (part == "CTRL" || part == "STRG") b.mods |= CK_MOD_CTRL;
            else if (part == "ALT") b.mods |= CK_MOD_ALT;
            else if (!part.empty()) {
                for (const CoopKeyName& n : CoopKeyNames)
                    if (part == n.token)
                        b.dik = n.dik;
                if (!b.dik && part.compare(0, 2, "0X") == 0)
                    b.dik = (int)strtol(part.c_str() + 2, NULL, 16) & 0xFF;
                if (!b.dik && part.compare(0, 4, "KEY_") == 0)
                    b.dik = GetEmulationKeyCode(string(part.c_str())) & 0xFF;
            }
        }
        if (!b.dik)
            b.mods = 0;
        return b;
    }

    static std::string CoopKeyShow(const CoopKeyBind& b) {
        if (!b.dik)
            return CoopTr("aus", "off");
        return std::string(b.mods & CK_MOD_SHIFT ? "Shift+" : "") + (b.mods & CK_MOD_CTRL ? CoopTr("Strg+", "Ctrl+") : "") +
            (b.mods & CK_MOD_ALT ? "Alt+" : "") + CoopKeyName1(b.dik);
    }

    // Gothics eigene Belegung (Gothic.ini [KEYS], je Aktion zwei Codes aus 4 Hex-Ziffern: niederwertiges Byte zuerst)
    struct CoopGothicKey { const char* ini; const char* de; const char* en; };
    static const CoopGothicKey CoopGothicKeys[] = {
        { "keyEnd", "Spielmen\xFC", "Game menu" }, { "keyHeal", "Heiltrank", "Healing potion" }, { "keyPotion", "Manatrank", "Mana potion" },
        { "keyLockTarget", "Ziel festhalten", "Lock target" }, { "keyParade", "Parade", "Parry" }, { "keyActionRight", "Aktion rechts", "Action right" },
        { "keyActionLeft", "Aktion links", "Action left" }, { "keyUp", "Vorw\xE4rts", "Forward" }, { "keyDown", "R\xFC" "ckw\xE4rts", "Backward" },
        { "keyLeft", "Links drehen", "Turn left" }, { "keyRight", "Rechts drehen", "Turn right" }, { "keyStrafeLeft", "Seitw\xE4rts links", "Strafe left" },
        { "keyStrafeRight", "Seitw\xE4rts rechts", "Strafe right" }, { "keyAction", "Aktion", "Action" }, { "keySlow", "Gehen", "Walk" },
        { "keySMove", "Sonderbewegung", "Special move" }, { "keyWeapon", "Waffe", "Weapon" }, { "keySneak", "Schleichen", "Sneak" },
        { "keyLook", "Umsehen", "Look around" }, { "keyLookFP", "Ich-Sicht", "First person" }, { "keyInventory", "Inventar", "Inventory" },
        { "keyShowStatus", "Charakter", "Character" }, { "keyShowLog", "Tagebuch", "Log" }, { "keyShowMap", "Karte", "Map" },
    };
    static std::map<int, int> CoopGothicUse;   // Code -> Eintrag in CoopGothicKeys
    static long long CoopGothicReadMs = 0;

    static void CoopGothicKeysRead() {
        const long long now = GetCurrentMs();
        if (CoopGothicReadMs && now < CoopGothicReadMs + 5000)
            return;
        CoopGothicReadMs = now;
        CoopGothicUse.clear();
        if (!zoptions)
            return;
        for (int i = 0; i < (int)(sizeof(CoopGothicKeys) / sizeof(CoopGothicKeys[0])); i++) {
            const std::string v = zoptions->ReadString(zSTRING("KEYS"), CoopGothicKeys[i].ini, "").ToChar();
            for (size_t k = 0; k + 4 <= v.size(); k += 4) {
                const int lo = (int)strtol(v.substr(k, 2).c_str(), NULL, 16), hi = (int)strtol(v.substr(k + 2, 2).c_str(), NULL, 16);
                const int code = lo | (hi << 8);
                if (code > 0 && code < 0x100 && !CoopGothicUse.count(code))
                    CoopGothicUse[code] = i;
            }
        }
    }

    // belegt Gothic diese Taste? -> Name der Aktion, sonst NULL
    static const char* CoopGothicUses(int dik) {
        CoopGothicKeysRead();
        auto it = CoopGothicUse.find(dik);
        if (it == CoopGothicUse.end())
            return NULL;
        return CoopLangEn ? CoopGothicKeys[it->second].en : CoopGothicKeys[it->second].de;
    }

    // Vorgabe je Aktion - aus den bisherigen Einstellungen, damit niemand seine Tasten verliert
    static CoopKeyBind CoopKeyDefault(int a) {
        CoopKeyBind b;
        const zSTRING sec("GOTHICCOOP");
        switch (a) {
        case CK_HELP:
            b.dik = 0x23;   // H - in Gothic II der Heiltrank: dann F1 (mit Shift tranke der Held trotzdem)
            if (CoopGothicUses(0x23))
                b.dik = 0x3B;
            break;
        case CK_MENU: b.dik = 0x25; b.mods = CK_MOD_SHIFT; break;   // Shift+K
        case CK_INV: {   // u101: coopInvKey 0-7 = aus|I|J|K|U|G|T|V
            static const int inv[8] = { 0, 0x17, 0x24, 0x25, 0x16, 0x22, 0x14, 0x2F };
            const int k = zoptions && zoptions->EntryExists(sec, "coopInvKey") ? zoptions->ReadInt(sec, "coopInvKey", 1) : 1;
            b.dik = inv[k >= 0 && k < 8 ? k : 1];
            break;
        }
        case CK_RUNE: b.dik = 0x14; b.mods = CK_MOD_SHIFT; break;   // Shift+T
        case CK_CAM: {   // v66: camWindowKey 0-8 = Shift+F5|F5|F11|Shift+Einfg|Einfg|Shift+Pos1|Pos1|Shift+Ende|Ende
            static const int cam[9][2] = { { 0x3F, 1 }, { 0x3F, 0 }, { 0x57, 0 }, { 0xD2, 1 }, { 0xD2, 0 }, { 0xC7, 1 }, { 0xC7, 0 }, { 0xCF, 1 }, { 0xCF, 0 } };
            const int k = zoptions && zoptions->EntryExists(sec, "camWindowKey") ? zoptions->ReadInt(sec, "camWindowKey", 0) : 0;
            const int i = k >= 0 && k < 9 ? k : 0;
            b.dik = cam[i][0];
            b.mods = cam[i][1] ? CK_MOD_SHIFT : 0;
            break;
        }
        case CK_HOST: b.dik = ReadConfigKey("startServerKey", "KEY_F1") & 0xFF; b.mods = CK_MOD_SHIFT; break;
        case CK_JOIN: b.dik = ReadConfigKey("startConnectionKey", "KEY_F2") & 0xFF; b.mods = CK_MOD_SHIFT; break;
        case CK_REVIVE: b.dik = ReadConfigKey("revivePlayerKey", "KEY_F4") & 0xFF; b.mods = CK_MOD_SHIFT; break;
        case CK_REINIT: b.dik = ReadConfigKey("reinitPlayersKey", "KEY_F3") & 0xFF; b.mods = CK_MOD_SHIFT; break;
        case CK_SYNC: b.dik = ReadConfigKey("coopSyncKey", "KEY_F6") & 0xFF; break;
        default: break;
        }
        return b;
    }

    static void CoopKeyStore(int a, const CoopKeyBind& b) {
        CoopKeyBinds[a] = b;
        if (zoptions)
            zoptions->WriteString(zSTRING("GOTHICCOOP"), CoopKeyActions[a].ini, zSTRING(CoopKeyToken(b).c_str()), 0);
    }

    // je 2 s aus Gothic.ini (Gothic schreibt sie nicht selbst - nur GT; so gilt auch eine Hand-Aenderung der Datei nach dem Start)
    static void CoopKeysRead(bool force = false) {
        const long long now = GetCurrentMs();
        if (!force && CoopKeysReadMs && now < CoopKeysReadMs + 2000)
            return;
        CoopKeysReadMs = now;
        if (!zoptions)
            return;
        const zSTRING sec("GOTHICCOOP");
        int written = 0;
        for (int a = 0; a < CK_COUNT; a++) {
            if (!zoptions->EntryExists(sec, CoopKeyActions[a].ini)) {
                CoopKeyStore(a, CoopKeyDefault(a));
                written++;
                continue;
            }
            CoopKeyBinds[a] = CoopKeyParse(zoptions->ReadString(sec, CoopKeyActions[a].ini, "").ToChar());
        }
        if (!CoopKeysLogged || written) {
            CoopKeysLogged = true;
            std::string all;
            for (int a = 0; a < CK_COUNT; a++)
                all += std::string(a ? ", " : "") + CoopKeyActions[a].de + " " + CoopKeyShow(CoopKeyBinds[a]);
            KeysLog("%s%s (P264)", written ? "Vorgaben geschrieben - " : "", all.c_str());
        }
    }

    // CoopLook.cpp, CoopSettingsDefaults: Vorgaben schreiben, damit Menue und Hilfe sie schon im Hauptmenue zeigen
    void CoopKeysDefaults() {
        CoopKeysRead(true);
        const zSTRING sec("GOTHICCOOP");
        if (zoptions && !zoptions->EntryExists(sec, "coopIntro"))
            zoptions->WriteInt(sec, "coopIntro", 0, 0);   // 0 = vor jedem neuen Spiel zeigen (Auswahl Ja|Nein)
    }

    std::string CoopKeyText(int a) {
        if (a < 0 || a >= CK_COUNT)
            return std::string();
        CoopKeysRead();
        return CoopKeyShow(CoopKeyBinds[a]);
    }

    int CoopKeyDik(int a) {
        if (a < 0 || a >= CK_COUNT)
            return 0;
        CoopKeysRead();
        return CoopKeyBinds[a].dik;
    }

    static bool CoopKeyForeground() {
        HWND fg = GetForegroundWindow();
        DWORD pid = 0;
        if (fg)
            GetWindowThreadProcessId(fg, &pid);
        return pid == GetCurrentProcessId();
    }

    static bool CoopKeyRaw(int dik) {
        if (zinput && zinput->KeyPressed(dik))
            return true;
        const int vk = CoopKeyVk(dik);   // Enter u. a. kommen ueber KeyPressed nicht immer an (v20, CoopKeyEdge)
        return vk && CoopKeyForeground() && (GetAsyncKeyState(vk) & 0x8000);
    }

    static int CoopKeyModsNow() {
        int m = 0;
        if (CoopKeyRaw(0x2A) || CoopKeyRaw(0x36)) m |= CK_MOD_SHIFT;
        if (CoopKeyRaw(0x1D) || CoopKeyRaw(0x9D)) m |= CK_MOD_CTRL;
        if (CoopKeyRaw(0x38) || CoopKeyRaw(0xB8)) m |= CK_MOD_ALT;
        return m;
    }

    // Spieltakt: einmal je Druck (Flanke), Zusatztasten genau wie eingestellt (Shift+F1 nicht bei F1, I nicht bei Shift+I), nicht
    // in Menues. Die Taste wird verbraucht (KeyToggled), damit ein spaeterer Abfrager des Merkers sie nicht noch einmal sieht.
    bool CoopKeyHit(int a) {
        if (a < 0 || a >= CK_COUNT)
            return false;
        CoopKeysRead();
        const CoopKeyBind& b = CoopKeyBinds[a];
        if (!b.dik || !zinput) {
            CoopKeyDown[a] = false;
            return false;
        }
        const bool down = CoopKeyRaw(b.dik);
        const bool edge = down && !CoopKeyDown[a];
        CoopKeyDown[a] = down;
        if (!edge || zCMenu::GetActive())
            return false;
        if (CoopKeyModsNow() != b.mods)
            return false;
        zinput->KeyToggled(b.dik);
        return true;
    }

    // --- Menue: Belegung aufnehmen (MENU_OPT_COOP_KEYS, Zuschau-Fenster in MENU_OPT_COOP_CAM) -----------------------------
    static int KeysCapture = -1;            // Aktion, deren Taste gerade aufgenommen wird
    static bool KeysCaptureArmed = false;   // erst aufnehmen, wenn die Taste losgelassen ist, die den Eintrag gewaehlt hat
    static long long KeysCaptureMs = 0;
    static std::string KeysNote, KeysNote2;   // zwei Hinweiszeilen - GD3D11 zeichnet die Menueschrift groesser, ~50 Zeichen je Zeile
    static long long KeysNoteUntil = 0;
    static long long KeysTextMs = 0;
    static zCMenu* KeysTextMenu = NULL;

    static void KeysSetNote(const std::string& t, const std::string& t2 = std::string(), long long ms = 8000) {
        KeysNote = t;
        KeysNote2 = t2;
        KeysNoteUntil = GetCurrentMs() + ms;
        KeysTextMs = 0;
    }

    static int KeysActionById(const std::string& id) {
        for (int a = 0; a < CK_COUNT; a++)
            if (id == CoopKeyActions[a].id)
                return a;
        return -1;
    }

    static std::string KeysValueText(int a) {
        if (a == KeysCapture)
            return CoopTr("Taste dr\xFC" "cken ...", "Press a key ...");
        const CoopKeyBind& b = CoopKeyBinds[a];
        std::string t = CoopKeyShow(b);
        const char* g = b.dik ? CoopGothicUses(b.dik) : NULL;
        if (g)
            t += std::string(CoopTr("  (Gothic: ", "  (Gothic: ")) + g + ")";
        return t;
    }

    // Befehle COOP_KEY_* und COOP_INTRO_OK aus CoopMenuLobby.cpp (LobbyHandleSelAction) - true: behandelt
    static bool IntroOk = false;
    static bool IntroRunning = false;

    bool CoopKeysSelAction(const std::string& arg, zCMenu* menu, int& result) {
        if (arg == "COOP_INTRO_OK") {   // P267: "Weiter zum Spiel" - die Einfuehrung schliessen wie ZURUECK (wie COOP_NG_START)
            if (menu && IntroRunning) {
                IntroOk = true;
                *(int*)&menu->m_exitState = 1;
                result = 1;
            }
            return true;
        }
        if (arg.compare(0, 9, "COOP_KEY_") != 0)
            return false;
        const std::string id = arg.substr(9);
        if (id == "RESET") {
            for (int a = 0; a < CK_COUNT; a++)
                CoopKeyStore(a, CoopKeyDefault(a));
            KeysCapture = -1;
            KeysSetNote(CoopTr("Alle Coop-Tasten auf die Vorgaben gesetzt.", "All coop keys reset to defaults."));
            KeysLog("alle Tasten auf die Vorgaben gesetzt");
            return true;
        }
        const int a = KeysActionById(id);
        if (a < 0)
            return false;
        CoopKeysRead(true);
        KeysCapture = a;
        KeysCaptureArmed = false;
        KeysCaptureMs = GetCurrentMs();
        KeysSetNote(std::string(CoopTr("Neue Taste: ", "New key: ")) + (CoopLangEn ? CoopKeyActions[a].en : CoopKeyActions[a].de),
            CoopTr("auch mit Shift/Strg/Alt - Entf: aus, Esc: abbrechen", "also with Shift/Ctrl/Alt - Del: off, Esc: cancel"), 30000);
        return true;
    }

    // jedes Menuebild waehrend der Aufnahme: erste gedrueckte Taste (ohne Zusatztasten allein) gilt
    static void KeysCapturePulse() {
        if (KeysCapture < 0)
            return;
        const long long now = GetCurrentMs();
        zCMenu* m = zCMenu::GetActive();
        if (!m || now > KeysCaptureMs + 30000) {
            KeysCapture = -1;
            KeysSetNote(CoopTr("Keine Taste gedr\xFC" "ckt - unver\xE4ndert.", "No key pressed - unchanged."));
            return;
        }
        int hit = 0;
        for (int dik = 1; dik < 0xEE; dik++) {
            if (CoopKeyIsModifier(dik) || !CoopKeyRaw(dik))
                continue;
            hit = dik;
            break;
        }
        if (!KeysCaptureArmed) {   // Enter bzw. Klick, der den Eintrag gewaehlt hat, erst loslassen
            if (!hit && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
                KeysCaptureArmed = true;
            return;
        }
        if (!hit || hit == 0x1C || hit == 0x9C)   // Enter waehlt im Menue - nicht als Taste
            return;
        const int a = KeysCapture;
        KeysCapture = -1;
        const char* name = CoopLangEn ? CoopKeyActions[a].en : CoopKeyActions[a].de;
        if (hit == 0x01) {
            KeysSetNote(CoopTr("Abgebrochen - unver\xE4ndert.", "Cancelled - unchanged."));
            return;
        }
        CoopKeyBind b;
        if (hit != 0xD3 && hit != 0x0E) {
            b.dik = hit;
            b.mods = CoopKeyModsNow();
        }
        CoopKeyStore(a, b);
        const char* g = b.dik ? CoopGothicUses(b.dik) : NULL;
        std::string warn;
        for (int o = 0; o < CK_COUNT; o++)
            if (o != a && b.dik && CoopKeyBinds[o].dik == b.dik && CoopKeyBinds[o].mods == b.mods)
                warn += std::string(warn.empty() ? "" : ", ") + CoopTr("auch ", "also ") + (CoopLangEn ? CoopKeyActions[o].en : CoopKeyActions[o].de);
        if (g)
            warn += std::string(warn.empty() ? "" : ", ") + "Gothic: " + g;
        KeysSetNote(std::string(name) + ": " + CoopKeyShow(b), warn.empty() ? std::string() : std::string(CoopTr("Achtung: ", "Note: ")) + warn);
        KeysLog("%s = %s (%s)%s", CoopKeyActions[a].de, CoopKeyShow(b).c_str(), CoopKeyToken(b).c_str(), g ? " - belegt auch Gothic" : "");
    }

    // --- Texte: Hilfe, Einfuehrung ----------------------------------------------------------------------------------------
    static std::string K(int a) { return CoopKeyShow(CoopKeyBinds[a]); }

    static void CoopHelpLines(std::vector<std::string>& l) {
        static const int order[CK_COUNT] = { CK_HELP, CK_MENU, CK_INV, CK_RUNE, CK_CAM, CK_REVIVE, CK_JOIN, CK_HOST, CK_SYNC, CK_REINIT };
        // je Zeile hoechstens ~50 Zeichen: GD3D11 zeichnet die Menueschrift groesser (Test 09.11., 1280x960: ~54 sichtbar)
        l.push_back(CoopTr("Coop-Tasten (\xE4ndern: Einstellungen > COOP):", "Coop keys (change: Settings > COOP):"));
        for (int i = 0; i < CK_COUNT; i++) {
            const int a = order[i];
            l.push_back("   " + K(a) + "   -   " + (CoopLangEn ? CoopKeyActions[a].en : CoopKeyActions[a].de));
        }
        l.push_back(std::string());
        l.push_back(CoopTr("Mitspieler-Anzeige: Einstellungen > COOP.", "Partner display: Settings > COOP."));
        l.push_back(std::string(CoopTr("Bewusstlos? Hingehen, ", "Knocked out? Walk over, ")) + K(CK_REVIVE) + CoopTr(", 10 s stehen.", ", stand 10 s."));
        l.push_back(CoopTr("Speichern und Laden: der Host f\xFC" "r beide.", "Saving and loading: the host for both."));
        l.push_back(std::string(CoopTr("Als Client h\xE4lt ", "As client, ")) + K(CK_JOIN) + CoopTr(" den Abgleich an.", " pauses the sync."));
        l.push_back(std::string(CoopTr("Probleme: ", "Problems: ")) + K(CK_SYNC) + CoopTr(" Welt abgleichen,", " resync world,"));
        l.push_back(CoopTr("   \xE4ltere St\xE4nde: Esc > Spielstand w\xE4hlen.", "   older saves: Esc > Choose save."));   // u237: statt der F9-Liste
    }

    static void CoopIntroLines(std::vector<std::string>& l) {   // 14 Zeilen, je hoechstens ~50 Zeichen (wie CoopHelpLines)
        l.push_back(CoopTr("Gemeinsam spielen: einer hostet, einer tritt bei -", "Play together: one hosts, the other joins -"));
        l.push_back(CoopTr("im Hauptmen\xFC unter COOP, \xFC" "ber Steam oder IP.", "COOP in the main menu, via Steam or IP."));
        l.push_back(CoopTr("Neues Spiel und Laden startet der Host,", "The host starts new games and loads,"));
        l.push_back(CoopTr("der Mitspieler kommt automatisch mit.", "your partner follows automatically."));
        l.push_back(CoopTr("Quests, Tagebuch, Truhen und Spielst\xE4nde werden", "Quests, diary, chests and saves are synced -"));
        l.push_back(CoopTr("abgeglichen - der Host gibt vor.", "the host decides."));
        l.push_back(CoopTr("Die Teleport-Rune bringt dich zum Mitspieler,", "The teleport rune takes you to your partner,"));
        l.push_back(CoopTr("das Coop-Inventar teilt ihr euch.", "you share the coop inventory."));
        l.push_back(std::string(CoopTr("Bewusstlos? Der Mitspieler belebt dich (", "Knocked out? Your partner revives you (")) + K(CK_REVIVE) + ").");
        l.push_back(std::string());
        l.push_back(std::string(CoopTr("Tasten: ", "Keys: ")) + K(CK_HELP) + CoopTr(" Hilfe, ", " help, ") + K(CK_MENU) + CoopTr(" Coop-Fenster,", " coop window,"));
        l.push_back(K(CK_INV) + CoopTr(" Coop-Inventar, ", " coop inventory, ") + K(CK_RUNE) + CoopTr(" Teleport-Rune.", " teleport rune."));
        l.push_back(std::string());
        l.push_back(std::string(CoopTr("Mehr Hilfe: ", "More help: ")) + K(CK_HELP) + CoopTr(" im Spiel, Einstellungen > COOP.", " in game, Settings > COOP."));
    }

    // --- Menue-Kreise: Einstellungen > COOP > Coop-Fenster > COOP-Einstellungen ... --------------------------------------
    // Ist das Ziel eines STARTMENU schon offen (weiter unten im Stapel), geht es dorthin zurueck, statt dasselbe Menue verschachtelt
    // ein zweites Mal zu oeffnen (dieselben Eintraege zweimal offen). Nur eigene Menues (Name mit COOP).
    static std::string KeysBackTo;

    bool CoopMenuCycleGuard(zCMenu* menu, const std::string& target, int& result) {
        if (!menu || target.find("COOP") == std::string::npos)
            return false;
        bool open = false;
        for (int i = 0; i < zCMenu::activeList.GetNum(); i++) {
            zCMenu* o = zCMenu::activeList[i];
            if (o && o != menu && LobbyUpper(o->name) == target)
                open = true;
        }
        if (!open)
            return false;
        KeysBackTo = target;
        *(int*)&menu->m_exitState = 1;   // wie ZURUECK (HandleSelAction-Fall 1, wie COOP_NG_START)
        result = 1;
        KeysLog("%s ist schon offen - zur\xFC" "ck dorthin statt verschachtelt", target.c_str());
        return true;
    }

    // jedes Menuebild (UnionHooks.cpp, Game_MenuLoop): Texte der eigenen Menues, Aufnahme, Untermenue nach der Taste
    static std::string KeysMenuPending;   // Untermenue, das im Esc-Menue aufgehen soll (Hilfe-/Fenster-Taste)
    static bool KeysMenuByKey = false;    // nach dessen Zurueck alle Menues schliessen

    void CoopKeysMenuPulse() {
        CoopKeysRead();
        KeysCapturePulse();
        zCMenu* m = zCMenu::GetActive();
        if (!m)
            return;
        if (!KeysBackTo.empty()) {   // Menue-Kreis: weiter zurueck, bis das Ziel oben ist
            if (LobbyUpper(m->name) == KeysBackTo || zCMenu::activeList.GetNum() <= 1)
                KeysBackTo.clear();
            else if ((int)m->m_exitState == 0)
                m->ForceSelAction(SEL_ACTION_BACK, zSTRING(""), NULL);
            return;
        }
        // P266: Untermenue, das die Taste angefordert hat - im ersten Bild des Esc-Menues, verschachtelt wie ein Eintrag
        if (!KeysMenuPending.empty() && (int)m->m_exitState == 0) {
            const std::string sub = KeysMenuPending;
            KeysMenuPending.clear();
            zCParser* mp = zCMenu::GetParser();
            if (mp && mp->GetSymbol(zSTRING(sub.c_str()))) {
                KeysLog("%s im Esc-Men\xFC ge\xF6" "ffnet (Taste)", sub.c_str());
                Ivk_zCMenu_HandleSelAction_Coop(m, SEL_ACTION_STARTMENU, zSTRING(sub.c_str()), NULL);
                if (KeysMenuByKey) {
                    KeysMenuByKey = false;
                    CoopMenuCloseRequest();   // zurueck ins Spiel statt ins Esc-Menue
                }
                return;
            }
            KeysLog("%s fehlt im Men\xFC-Skript (COOP_MENU.D vor u226?) - nur das Esc-Men\xFC", sub.c_str());
            KeysMenuByKey = false;
        }
        const std::string name = LobbyUpper(m->name);
        const long long now = GetCurrentMs();
        if (m == KeysTextMenu && now < KeysTextMs + 250 && KeysCapture < 0)
            return;
        KeysTextMenu = m;
        KeysTextMs = now;
        char id[64];
        if (name == "MENU_OPT_COOP_KEYS") {
            for (int a = 0; a < CK_COUNT; a++) {
                _snprintf(id, sizeof(id), "MENUITEM_COOPKEYS_LBL_%s", CoopKeyActions[a].id);
                id[sizeof(id) - 1] = 0;
                LobbySetItemText(LobbyFindItem(m, id), CoopLangEn ? CoopKeyActions[a].en : CoopKeyActions[a].de);
                _snprintf(id, sizeof(id), "MENUITEM_COOPKEYS_VAL_%s", CoopKeyActions[a].id);
                id[sizeof(id) - 1] = 0;
                LobbySetItemText(LobbyFindItem(m, id), KeysValueText(a));
            }
            const bool note = !KeysNote.empty() && now < KeysNoteUntil;
            LobbySetItemText(LobbyFindItem(m, "MENUITEM_COOPKEYS_NOTE"), note ? KeysNote
                : std::string(CoopTr("Zeile w\xE4hlen, dann die neue Taste dr\xFC" "cken.", "Pick a row, then press the new key.")));
            LobbySetItemText(LobbyFindItem(m, "MENUITEM_COOPKEYS_NOTE2"), note ? KeysNote2
                : std::string(CoopTr("Entf: aus. Gothics eigene Tasten in Klammern.", "Del: off. Gothic's own keys in brackets.")));
            return;
        }
        if (name == "MENU_OPT_COOP_CAM") {   // Taste des Zuschau-Fensters - dieselbe Aufnahme
            LobbySetItemText(LobbyFindItem(m, "MENUITEM_COOP_CAMKEY_VAL"), KeysValueText(CK_CAM));
            return;
        }
        if (name == "MENU_COOP_HELP" || name == "MENU_COOP_INTRO") {
            std::vector<std::string> l;
            const bool help = name == "MENU_COOP_HELP";
            if (help)
                CoopHelpLines(l);
            else
                CoopIntroLines(l);
            const int lines = help ? 20 : 14;
            for (int i = 0; i < lines; i++) {
                _snprintf(id, sizeof(id), help ? "MENUITEM_COOPHELP_L%02d" : "MENUITEM_COOPINTRO_L%02d", i);
                id[sizeof(id) - 1] = 0;
                LobbySetItemText(LobbyFindItem(m, id), i < (int)l.size() ? l[i] : std::string());
            }
            return;
        }
        if (player && (name == "MENU_COOP" || name == "MENU_COOP_IP"))   // P266: Coop-Fenster im Spiel - Statuszeilen wie im Hauptmenue
            LobbyShowStatus();
    }

    // --- P266: Befehle des Coop-Fensters im Spiel (bis u225 ignoriert) ---------------------------------------------------
    bool CoopKeysIngameCommand(const std::string& cmd, zCMenu* menu) {
        if (cmd == "COOP_HOST") {
            CoopKeyHost();
            LobbyNote(ServerThread ? CoopTr("Host l\xE4uft.", "Host running.") : CoopTr("Host startet ...", "Starting host ..."), menu);
            return true;
        }
        if (cmd == "COOP_CLIENT") {
            if (ClientThread && CoopClientState == 2) {
                LobbyNote(CoopTr("Schon mit dem Host verbunden.", "Already connected to the host."), menu);
                return true;
            }
            CoopSettingsRefresh(true);   // IP/Port/Name aus dem Fenster
            CoopKeyClient();
            LobbyNote(CoopTr("Verbinde ...", "Connecting ..."), menu);
            return true;
        }
        if (cmd == "COOP_STOP") {
            CoopPendingRole = 0;
            if (ClientThread)
                CoopClientStop = 1;
            if (ServerThread)
                CoopServerStop = 1;
            LobbyNote(CoopTr("Verbindung getrennt.", "Disconnected."), menu);
            return true;
        }
        LobbyNote(CoopTr("Das geht nur im Hauptmen\xFC (Esc > Hauptmen\xFC).", "Only in the main menu (Esc > Main menu)."), menu);
        return true;
    }

    // --- P267: Einfuehrung vor einem neuen Spiel --------------------------------------------------------------------------
    static long long IntroPassedMs = 0;
    bool CoopShowActive();   // CoopShow.cpp: Vorfuehrung fuer Aufnahmen (zeigt die Einfuehrung und bestaetigt sie, SH_INTRO)

    // aus LobbyHandleSelAction bei SEL_ACTION_CLOSE: true = kein neues Spiel (Einfuehrung mit Zurueck verlassen)
    bool CoopIntroGate(zCMenu* menu, const std::string& arg) {
        const bool newGame = arg == "NEW_GAME" || arg.compare(0, 12, "OTHELLO_NEW_") == 0;
        if (!newGame || !menu || IntroRunning)
            return false;
        if (IntroPassedMs && GetCurrentMs() < IntroPassedMs + 300000)
            return false;   // eben bestaetigt (z. B. danach COOP-Schwierigkeit abgebrochen und neu gewaehlt) - nicht noch einmal
        if (ClientThread && !ServerThread)
            return false;   // der Mitspieler startet nie selbst (LobbyHandleSelAction sagt es ihm)
        if (strstr(GetCommandLineA(), "-cooptest:") && !CoopShowActive())
            return false;   // automatischer Testlauf bzw. Testlauf (CoopAutoTest.cpp startet ueber NEW_GAME) - ausser in der Vorfuehrung
        if (zoptions && zoptions->ReadInt(zSTRING("GOTHICCOOP"), "coopIntro", 0) != 0)
            return false;   // "Vor jedem neuen Spiel zeigen: Nein"
        zCParser* mp = zCMenu::GetParser();
        if (!mp || !mp->GetSymbol(zSTRING("MENU_COOP_INTRO")))
            return false;
        IntroRunning = true;
        IntroOk = false;
        KeysLog("Einf\xFChrung vor dem neuen Spiel (%s)", arg.c_str());
        Ivk_zCMenu_HandleSelAction_Coop(menu, SEL_ACTION_STARTMENU, zSTRING("MENU_COOP_INTRO"), NULL);
        IntroRunning = false;
        if (!IntroOk) {
            KeysLog("Einf\xFChrung mit Zur\xFC" "ck verlassen - kein neues Spiel");
            return true;
        }
        IntroPassedMs = GetCurrentMs();
        return false;
    }

    // --- P265/P266: Hilfe- und Fenster-Taste wie Esc (Haken auf CGameManager::HandleEvent) ---------------------------------
    // u226 (Test 09.11.): Shift+K kam bei CGameManager::HandleEvent an, F1 nie - Gothic II verarbeitet F1-F4 schon in
    // oCGame::HandleEvent (G2A 0x006FC66F: Taste - 0x3B, Sprungtabelle), zCInputCallback::DoEvents gibt eine verarbeitete Taste
    // nicht weiter. Darum erkennt der Spieltakt Hilfe- und Fenster-Taste wie jede Coop-Taste am Tastenzustand (CoopKeyHit) und legt
    // einen eigenen Code in Gothics Tastenpuffer (wie die Maus, CoopMenuMouse.cpp); den faengt der Haken ab und oeffnet das Menue
    // wie Esc. So geht jede Taste, auch eine, die Gothic selbst nutzt.
    static const int KEYS_SYNTH = 0x1FE;   // eigener Code im Tastenpuffer: ueber den DirectInput-Codes, unter Gothics Mauscodes (0x20C ...)
    static int KeysWant = -1;               // CK_HELP bzw. CK_MENU - wartet auf den eigenen Code
    static long long KeysWantMs = 0;
    bool CoopMenuPushKey(int key);          // CoopMenuMouse.cpp

    static void KeysOpenMenu(int which) {
        if (!player || !ogame || IsLoadingLevel || zCMenu::GetActive() || !gameMan || CoopWpIsWorld() || WorldEditMode)
            return;
        if (IsPlayerTalkingWithAnybody()) {
            ChatLog(CoopTr("Coop: erst das Gespr\xE4" "ch beenden.", "Coop: finish the conversation first."));
            return;
        }
        int arg = 0;
        const int ok = ((int(__thiscall*)(CGameManager*, int&))CoopAddr::CGameManager_MenuEnabled)(gameMan, arg);
        if (!ok) {
            KeysLog("%s: Gothic l\xE4sst gerade kein Men\xFC zu", CoopKeyActions[which].de);
            return;
        }
        KeysMenuPending = which == CK_HELP ? "MENU_COOP_HELP" : "MENU_COOP";
        KeysMenuByKey = true;
        KeysLog("%s (%s) - \xF6" "ffne %s", CoopKeyActions[which].de, CoopKeyShow(CoopKeyBinds[which]).c_str(), KeysMenuPending.c_str());
        ((void(__thiscall*)(CGameManager*, int))CoopAddr::CGameManager_Menu)(gameMan, arg);   // Esc-Menue; darin das Untermenue
        KeysMenuPending.clear();
        KeysMenuByKey = false;
    }

    // Spieltakt (Plugin.cpp, KeysPressedChecks)
    void CoopKeysMenuKeyPulse() {
        const long long now = GetCurrentMs();
        for (int a : { CK_HELP, CK_MENU }) {
            if (!CoopKeyHit(a))
                continue;
            KeysWant = a;
            KeysWantMs = now;
            if (!CoopMenuPushKey(KEYS_SYNTH)) {
                KeysLog("%s: Tastenpuffer nicht erreichbar (Maus-Modul aus) - kein Men\xFC", CoopKeyActions[a].de);
                KeysWant = -1;
            }
        }
        if (KeysWant >= 0 && now > KeysWantMs + 2000) {
            KeysLog("%s: eigener Tastencode kam nicht bei Gothics Men\xFC-Taste an", CoopKeyActions[KeysWant].de);
            KeysWant = -1;
        }
    }

    int __fastcall CGameManager_HandleEvent_Coop(CGameManager* _this, void* vtable, int key);
    CInvoke<int(__thiscall*)(CGameManager*, int)> Ivk_CGameManager_HandleEvent_Coop(CoopAddr::CGameManager_HandleEvent, &CGameManager_HandleEvent_Coop,
        COOP_IVK(Lobby));
    int __fastcall CGameManager_HandleEvent_Coop(CGameManager* _this, void* vtable, int key) {
        try {
            if (key == KEYS_SYNTH) {
                const int which = KeysWant;
                KeysWant = -1;
                if (which >= 0)
                    KeysOpenMenu(which);
                return 1;
            }
        }
        catch (...) {
            CoopErrorLog("Coop-Tasten", "Zugriffsfehler (SEH) oder unbekannte Ausnahme", "HandleEvent");
        }
        return Ivk_CGameManager_HandleEvent_Coop(_this, key);
    }

    // --- P268: Hinweise im Spiel (Spieltakt) ------------------------------------------------------------------------------
    static bool HintFirstShown = false;
    static long long HintFirstAtMs = 0;
    static bool HintHostShown = false;
    static std::set<oCNpc*> HintDownShown;
    static long long HintDownScanMs = 0;

    void CoopKeysGamePulse() {
        // u238 (Nutzer 09.11.: "sehe ich noch die Coop einstellungen und hotkeys"): in automatischen Laeufen keine Tasten-Hinweise
        if (!player || IsLoadingLevel || CoopWpIsWorld() || CoopAutoLauf())
            return;
        const long long now = GetCurrentMs();
        CoopKeysRead();
        if (!HintFirstShown) {   // einmal je Sitzung, kurz nach dem ersten Spielbild
            if (!HintFirstAtMs)
                HintFirstAtMs = now + 8000;
            else if (now > HintFirstAtMs) {
                HintFirstShown = true;
                ChatLog(string((std::string(CoopTr("Coop: Hilfe ", "Coop: help ")) + K(CK_HELP) + CoopTr(", Coop-Fenster ", ", coop window ") + K(CK_MENU) +
                    CoopTr(", Coop-Inventar ", ", coop inventory ") + K(CK_INV) + CoopTr(" - alle Tasten unter Einstellungen > COOP.", " - all keys under Settings > COOP.")).c_str()),
                    zCOLOR(200, 230, 255, 255));
                KeysLog("Hinweis auf Hilfe/Fenster/Inventar gezeigt (P268)");
            }
        }
        if (ServerThread && !HintHostShown) {   // Hosten: Port und Firewall
            HintHostShown = true;
            char b[300];
            _snprintf(b, sizeof(b), CoopTr("Coop: Host l\xE4uft auf Port %d - Freunde treten \xFC" "ber Steam oder deine IP bei. Fragt Windows nach der Firewall: Zugriff zulassen.",
                "Coop: host running on port %d - friends join via Steam or your IP. If Windows asks about the firewall: allow access."), ConnectionPort);
            b[sizeof(b) - 1] = 0;
            ChatLog(string(b), zCOLOR(200, 230, 255, 255));
        }
        if (!ServerThread)
            HintHostShown = false;
        if (now > HintDownScanMs + 1000) {   // Mitspieler bewusstlos: wie wiederbeleben
            HintDownScanMs = now;
            for (auto& p : PlayerNpcs) {
                oCNpc* n = p.first;
                if (!n || n == player)
                    continue;
                const bool down = n->GetAttribute(NPC_ATR_HITPOINTS) == 0 && !n->IsDead();
                if (!down) {
                    HintDownShown.erase(n);
                    continue;
                }
                if (HintDownShown.count(n))
                    continue;
                HintDownShown.insert(n);
                ChatLog(string((std::string(p.second.ToChar()) + CoopTr(" ist bewusstlos - hingehen, ", " is knocked out - walk over, press ") + K(CK_REVIVE) +
                    CoopTr(" dr\xFC" "cken und 10 s stillstehen.", " and stand still for 10 s.")).c_str()), zCOLOR(255, 220, 140, 255));
            }
            if (HintDownShown.size() > 16)
                HintDownShown.clear();
        }
    }

    // Plugin.cpp, LoadBegin: Zeiger der alten Welt verwerfen
    void CoopKeysWorldGone() {
        HintDownShown.clear();
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)
    bool CoopKeyHit(int) { return false; }
    std::string CoopKeyText(int) { return std::string(); }
    int CoopKeyDik(int) { return 0; }
    void CoopKeysDefaults() {}
    bool CoopKeysSelAction(const std::string&, zCMenu*, int&) { return false; }
    void CoopKeysMenuPulse() {}
    bool CoopKeysIngameCommand(const std::string&, zCMenu*) { return false; }
    bool CoopIntroGate(zCMenu*, const std::string&) { return false; }
    bool CoopMenuCycleGuard(zCMenu*, const std::string&, int&) { return false; }
    void CoopKeysGamePulse() {}
    void CoopKeysMenuKeyPulse() {}
    void CoopKeysWorldGone() {}
#endif
}
