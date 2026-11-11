// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- Zeiger-Pruefungen ueber 2 GB (u246, P293) -------------------------------------------------------------------------
    //
    // Nutzer 10.11.2026 zu Xeres' Rueckkehr: "Das Spiel hat ausserdem noch einige Probleme / Bugs. Ein davon wurde bereits behoben
    // ... Es ist ein Teleport-Bug gewesen. Diese Fixes die das Spiel betreffen, sollen sowohl als eigener Download bereitgestellt
    // werden, als auch teil von Xeres Coop werden". Gothic2.exe ist Large-Address-Aware: Liegt ein Objekt oberhalb von 2 GB, ist
    // sein Zeiger als Daedalus-int negativ. Skripte, die Zeiger mit "> 0" pruefen, halten ihn dann fuer NULL. Xeres 1.0.9 tut das
    // an 20 Stellen (tools\mod_zeiger.py, docs\mod-analyse\Xeres-zeiger.md): B_SetLevelChange (Weltwechsel ohne Ziel - Absturz oder
    // falsches Ziel; der Teleport-Fix vom 17.10. ersetzte nur diese Funktion), der Pass ins Minental, Tueren in Vinos Brennerei,
    // Loren in der Eismine (auch in DauerFunc_01), Mob_RemoveItems, MEM_RemoveVob/MEMINT_RemoveVob. Archolos: Player_Mob_Missing_Item.
    // Jetzt stellt der Coop solche Vergleiche nach dem Laden der Skripte im Bytecode um - ein Byte je Stelle, die Spiellogik bleibt:
    // "PUSHINT 0; PUSHVAR z; >" und "PUSHVAR z; PUSHINT 0; <" (z > 0, 0 < z) -> "!="; "... <=" bzw. "... >=" (z <= 0, 0 >= z) -> "==".
    // z ist ein Zeiger, wenn die Variable in derselben Funktion das Ergebnis einer Zeiger-Funktion bekommt (MEM_SearchVobByName,
    // MEM_InstToPtr, _@, Itm_GetPtr ...) oder ihr Name auf PTR endet. Dasselbe tut das Skript-Paket Xeres-Fixes (XERES_FIX_ZEIGER.D)
    // fuer Spieler ohne Coop; liegt beides vor, findet der zweite nichts mehr. Allgemein (jede Mod, beide Engines), jede Stelle im Log.
    int CoopZeigerFixed = 0;   // umgestellte Vergleiche seit dem Start (Testlauf)

    // u248 (P299): Probe fuer ein Skript-Paket, das dieselben Stellen selbst umstellt (Xeres-Fixes, XERES_FIX_ZEIGER.D bei Init_Global).
    // Im Coop stellt GT beim Start zuerst um, das Paket findet danach nichts - ob es allein wirkt, sah man nie. Mit -coopzeigerprobe zaehlt
    // GT beim Start nur; nach dem Laden (Init_Global ist durch) zaehlt er wieder: 0 offen = das Paket hat alle umgestellt. Was dann noch
    // offen ist, stellt GT wie sonst um - das Spiel bleibt geschuetzt
    static bool CoopZeigerProbe() {
        static int on = -1;
        if (on < 0)
            on = strstr(GetCommandLineA(), "-coopzeigerprobe") ? 1 : 0;
        return on == 1;
    }

    static bool CoopZeigerFunk(const char* n) {   // wie ZEIGER_FUNK in tools\mod_zeiger.py
        static const char* exakt[] = { "MEM_SEARCHVOBBYNAME", "MEM_SEARCHALLVOBSBYNAME", "MEM_INSTTOPTR", "MEM_INSTGETOFFSET",
            "MEM_GETINTADDRESS", "MEM_ALLOC", "MEM_GETSTRINGADDRESS", "_@", "_@S", "_@F" };
        for (const char* e : exakt)
            if (!strcmp(n, e))
                return true;
        const size_t l = strlen(n);
        for (const char* s : { "VOBBYNAME", "TOPTR", "GETPTR", "_PTR" }) {
            const size_t k = strlen(s);
            if (l >= k && !strcmp(n + l - k, s))
                return true;
        }
        return false;
    }

    // Name endet auf PTR, auch mit Ziffern danach (PTR, HAGENPTR, WALLPTR, PTR2) - nur der Teil nach dem Punkt (lokale Variable)
    static bool CoopZeigerName(const char* n) {
        const char* p = strrchr(n, '.');
        p = p ? p + 1 : n;
        size_t l = strlen(p);
        while (l > 0 && p[l - 1] >= '0' && p[l - 1] <= '9')
            l--;
        return l >= 3 && !strncmp(p + l - 3, "PTR", 3);
    }

    void CoopZeigerFix(const char* wann) {
        if (!parser)
            return;
        const bool nurZaehlen = CoopZeigerProbe() && !strcmp(wann, "Start");   // u248 (P299)
        unsigned char* code = parser->stack.stack;
        const int size = parser->stack.stacksize;
        const int cnt = parser->symtab.table.GetNum();
        if (!code || size <= 0 || cnt <= 0)
            return;
        std::vector<std::pair<int, int>> owners;   // Code-Anfang -> Symbol (Funktion, Prototyp, Instanz)
        std::map<int, int> funcAt;                  // Code-Anfang -> Funktionssymbol
        for (int i = 0; i < cnt; i++) {
            zCPar_Symbol* s = parser->symtab.table[i];
            if (!s || (s->flags & (zPAR_FLAG_EXTERNAL | zPAR_FLAG_CLASSVAR)))
                continue;
            if (s->type != zPAR_TYPE_FUNC && s->type != zPAR_TYPE_PROTOTYPE && s->type != zPAR_TYPE_INSTANCE)
                continue;
            const int st = s->single_intdata;
            if (st < 0 || st >= size)
                continue;
            owners.push_back(std::make_pair(st, i));
            if (s->type == zPAR_TYPE_FUNC)
                funcAt[st] = i;
        }
        if (owners.empty())
            return;
        std::sort(owners.begin(), owners.end());
        auto ownerOf = [&](int q) {
            auto it = std::upper_bound(owners.begin(), owners.end(), std::make_pair(q, 0x7FFFFFFF));
            return it == owners.begin() ? -1 : (it - 1)->second;
        };
        auto symName = [&](int i) -> const char* {
            zCPar_Symbol* s = (i >= 0 && i < cnt) ? parser->symtab.table[i] : NULL;
            return s ? s->name.ToChar() : "";
        };
        struct Befehl { int q; unsigned char op; int arg; };
        // Durchgang 1: Zeiger-Variablen je Funktion (CALL <Zeiger-Funktion>; PUSHVAR v; =)
        std::set<std::pair<int, int>> zeiger;
        Befehl b1 = { -1, 0, 0 }, b2 = { -1, 0, 0 };
        for (int q = 0; q < size;) {
            const unsigned char op = code[q];
            int arg = 0, len = 1;
            if (op == 61 || op == 62 || op == 64 || op == 65 || op == 67 || op == 75 || op == 76 || op == 80) {
                if (q + 5 > size)
                    break;
                arg = *(const int*)(code + q + 1);
                len = 5;
            }
            else if (op == 245) {
                if (q + 6 > size)
                    break;
                arg = *(const int*)(code + q + 1);
                len = 6;
            }
            if (op == 9 && b1.op == 65 && b2.op == 61 && b2.q + 5 == b1.q && b1.q + 5 == q) {
                auto f = funcAt.find(b2.arg);
                if (f != funcAt.end() && CoopZeigerFunk(symName(f->second)))
                    zeiger.insert(std::make_pair(ownerOf(q), b1.arg));
            }
            b2 = b1;
            b1 = { q, op, arg };
            q += len;
        }
        // Durchgang 2: Vergleiche umstellen
        std::map<int, int> jeFunktion;
        int n = 0;
        b1 = { -1, 0, 0 };
        b2 = { -1, 0, 0 };
        for (int q = 0; q < size;) {
            const unsigned char op = code[q];
            int arg = 0, len = 1;
            if (op == 61 || op == 62 || op == 64 || op == 65 || op == 67 || op == 75 || op == 76 || op == 80) {
                if (q + 5 > size)
                    break;
                arg = *(const int*)(code + q + 1);
                len = 5;
            }
            else if (op == 245) {
                if (q + 6 > size)
                    break;
                arg = *(const int*)(code + q + 1);
                len = 6;
            }
            // Daedalus legt erst den rechten, dann den linken Operanden: b2 = rechts, b1 = links
            if ((op == 8 || op == 15 || op == 7 || op == 18) && b2.q + 5 == b1.q && b1.q + 5 == q) {
                int var = -1;
                unsigned char neu = 0;
                if (b2.op == 64 && b2.arg == 0 && b1.op == 65 && (op == 8 || op == 15)) {          // z > 0, z <= 0
                    var = b1.arg;
                    neu = op == 8 ? 17 : 16;
                }
                else if (b1.op == 64 && b1.arg == 0 && b2.op == 65 && (op == 7 || op == 18)) {     // 0 < z, 0 >= z
                    var = b2.arg;
                    neu = op == 7 ? 17 : 16;
                }
                if (var >= 0) {
                    const int o = ownerOf(q);
                    if (zeiger.count(std::make_pair(o, var)) || CoopZeigerName(symName(var))) {
                        if (!nurZaehlen)
                            code[q] = neu;
                        n++;
                        jeFunktion[o]++;
                    }
                }
            }
            b2 = b1;
            b1 = { q, op, arg };
            q += len;
        }
        if (CoopZeigerProbe()) {   // u248 (P299)
            static int probeZeilen = 0;
            if (probeZeilen++ < 10) {
                if (nurZaehlen)
                    OthelloCompatLog("Zeiger-Probe (P299, %s, -coopzeigerprobe): %d Pruefungen \"> 0\"/\"<= 0\" auf Zeiger offen - GT stellt sie "
                        "jetzt nicht um (ein Skript-Paket soll es bei Init_Global tun), nach dem Laden zaehlt er wieder", wann, n);
                else if (!n)
                    OthelloCompatLog("Zeiger-Probe (P299, %s): 0 Pruefungen offen - das Skript-Paket hat alle umgestellt", wann);
                else
                    OthelloCompatLog("Zeiger-Probe (P299, %s): %d Pruefungen noch offen - kein Skript-Paket hat sie umgestellt, GT stellt sie "
                        "jetzt um", wann, n);
            }
            if (nurZaehlen)
                return;
        }
        if (!n)
            return;
        CoopZeigerFixed += n;
        static int zeilen = 0;
        std::string liste;
        for (auto& f : jeFunktion) {
            if (liste.size() > 600) {
                liste += ", ...";
                break;
            }
            liste += std::string(liste.empty() ? "" : ", ") + symName(f.first) + (f.second > 1 ? " x" + std::to_string(f.second) : "");
        }
        if (zeilen++ < 10)
            OthelloCompatLog("Zeiger-Fix (P293, %s): %d Pruefungen \"> 0\"/\"<= 0\" auf Zeiger umgestellt (ueber 2 GB sonst wie NULL) - %s",
                wann, n, liste.c_str());
    }

} // namespace GOTHIC_ENGINE
