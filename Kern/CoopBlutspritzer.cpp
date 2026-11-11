// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- Blutspritzer-Schleife (u249, P300; u252: Blutspritzer mit echtem Schaden, P302) -----------------------------------------
    //
    // Nutzer 10.11.2026 zu Xeres' Rueckkehr: "wenn blutspritzer an war und man mit bogen abgeschossen wurde ist das spiel abgestuerzt".
    // LeGo ab 2.8 - in Xeres ueber das Union-Toolkit (Data\Toolkit.vdf, LeGo 2.9.0), das Xeres' LeGo 2.5.0 ersetzt: neuer Code wird
    // angehaengt, an den alten Funktionsanfang kommt ein Sprung dorthin (kein _OLD). _Bloodsplats_Loop setzt jedes Bild die Wahrnehmung
    // PERC_ASSESSDAMAGE des Helden auf _B_HeroDamage, merkt die vorige Funktion in _B_HeroDamagePercFunc, und _B_HeroDamage ruft sie
    // nach dem Blutspritzer auf. Xeres' eigene Schleife setzte dort B_AssessDamage_Hero, und das ruft bei "Blutspritzer: an" (Vorgabe)
    // nach dem eigenen Schaden selbst _B_HeroDamage auf - mit dem Toolkit rufen sich beide gegenseitig auf. Bei einem Angreifer im
    // Fernkampf nimmt B_AssessDamage_Hero jedes Mal wieder den Weg mit Schaden und Blutspritzer (B_MagicHurtNpc setzt aivar[99] auf das
    // Leben, die naechste Ebene sieht "kein neuer Schaden", Fernkampf -> B_CalculateDamage): endlose Verschachtelung, Absturz.
    // u249/u250 machten den Aufruf zu einem Sprung auf den naechsten Befehl. u252 (Pruefung 10.11. abends, Nutzer: "laeuft wirklich ohne
    // Probleme"): Dann zeigte nur noch LeGos Wahrnehmung einen Blutspritzer - vor dem Schaden der Mod, also fuer 0 (Fernkampf) oder 1
    // Lebenspunkt (Nahkampf, die Engine zieht 1 ab, die Mod gibt ihn zurueck und rechnet selbst): Blutspritzer praktisch weg. Jetzt bekommt
    // der alte Anfang von _B_HeroDamage (LeGo 2.5: currDam = Hero_LastHP - Leben, Blutspritzer, HERO_HURT - reicht nichts weiter) seinen
    // ersten Befehl zurueck (SETINST HERO statt des Sprungs). Der Aufruf der Mod landet wieder dort: Blutspritzer mit dem echten Schaden,
    // nichts ruft zurueck. Passt das Muster nicht (anderes LeGo, anderer Ersatz), wie bisher ein Sprung auf den naechsten Befehl. Einen so
    // entschaerften Aufruf (Skript-Paket Xeres-Fixes 1.1.0, GT u249-u251) setzt GT wieder ein, wenn der alte Anfang da ist. Allgemein: jede
    // Mod, deren Schadens-Wahrnehmung des Helden _B_HeroDamage selbst aufruft, mit einem LeGo ab 2.8; dasselbe tut das Skript-Paket ab
    // 1.2.0 - liegt beides vor, findet GT nichts mehr und meldet das
    int CoopBlutFixed = 0;   // behandelte Aufrufe seit dem Start (Testlauf)
    static int CoopBlutFid = -1;   // zuletzt gepruefte Wahrnehmung (Symbolnummer)

    static int CoopBlutLen(int op) {   // Laenge eines Daedalus-Befehls (wie P293)
        if (op == 61 || op == 62 || op == 64 || op == 65 || op == 67 || op == 75 || op == 76 || op == 80)
            return 5;
        return op == 245 ? 6 : 1;
    }

    // u250: Ziel eines Aufrufs. Das Union-Toolkit ersetzt eine Funktion, indem es neuen Code anhaengt und an den alten Anfang einen
    // Sprung dorthin schreibt (im Spiel nachgesehen: _B_HERODAMAGE Inhalt 5877985, am alten Anfang 97468 "JMP 5877985", kein _OLD;
    // B_AssessDamage_Hero ruft weiter "CALL 97468" auf) - u249 verglich nur das unmittelbare Ziel und fand den Aufruf nicht
    static int CoopBlutZiel(const unsigned char* code, int size, int ziel) {
        for (int k = 0; k < 4 && ziel >= 0 && ziel + 5 <= size && code[ziel] == 75; k++)
            ziel = *(const int*)(code + ziel + 1);
        return ziel;
    }

    // u252: Bereich [anf, ende) einer Skriptfunktion - bis zum Anfang der naechsten; false = keine
    static bool CoopBlutBereich(int fid, int& anf, int& ende, std::string& name) {
        const int cnt = parser->symtab.table.GetNum();
        if (fid <= 0 || fid >= cnt)
            return false;
        zCPar_Symbol* f = parser->symtab.table[fid];
        if (!f || f->type != zPAR_TYPE_FUNC || (f->flags & zPAR_FLAG_EXTERNAL))
            return false;
        name = f->name.ToChar();
        const int size = parser->stack.stacksize;
        anf = f->single_intdata;
        if (!parser->stack.stack || anf < 0 || anf >= size)
            return false;
        ende = size;
        for (int i = 0; i < cnt; i++) {
            zCPar_Symbol* s = parser->symtab.table[i];
            if (!s || (s->flags & (zPAR_FLAG_EXTERNAL | zPAR_FLAG_CLASSVAR)))
                continue;
            if (s->type != zPAR_TYPE_FUNC && s->type != zPAR_TYPE_PROTOTYPE && s->type != zPAR_TYPE_INSTANCE)
                continue;
            if (s->single_intdata > anf && s->single_intdata < ende)
                ende = s->single_intdata;
        }
        return true;
    }

    // Funktionen, die weiterreichen: _B_HeroDamage und - ist es per MergeMode ersetzt - das Original
    static void CoopBlutZiele(std::set<int>& ziele) {
        for (const char* z : { "_B_HERODAMAGE", "_B_HERODAMAGE_OLD" })
            if (zCPar_Symbol* s = parser->GetSymbol(zSTRING(z)))
                if (s->type == zPAR_TYPE_FUNC && !(s->flags & zPAR_FLAG_EXTERNAL))
                    ziele.insert(s->single_intdata);
    }

    // u252: Symbolnummern fuer das Muster des alten Anfangs (HERO, C_NPC.ATTRIBUTE, HERO_LASTHP); false = fehlt eins. Einmal je Parser -
    // die erste Fassung suchte sie bei jedem Aufruf, und die Suche ueber den ganzen Skriptcode (CoopBlutAltSuchen) hielt ein Bild 5,3 s an
    static bool CoopBlutSymbole(int& held, int& attr, int& last) {
        static zCParser* fuer = NULL;
        static int h = -1, a = -1, l = -1;
        if (fuer != parser) {
            fuer = parser;
            h = parser ? parser->GetIndex(zSTRING("HERO")) : -1;
            a = parser ? parser->GetIndex(zSTRING("C_NPC.ATTRIBUTE")) : -1;
            l = parser ? parser->GetIndex(zSTRING("HERO_LASTHP")) : -1;
        }
        held = h;
        attr = a;
        last = l;
        return held >= 0 && attr >= 0 && last >= 0;
    }

    // u252: Steht bei at der alte Anfang von _B_HeroDamage? Ab +5 "PUSHVAR C_NPC.ATTRIBUTE, PUSHVAR HERO_LASTHP, SUB" (currDam =
    // Hero_LastHP - hero.attribute[ATR_HITPOINTS]); davor der Sprung des Toolkits (sprung) oder schon wieder "SETINST HERO"
    static bool CoopBlutAlt(const unsigned char* code, int size, int at, bool sprung) {
        int held, attr, last;
        if (at < 0 || at + 16 > size || !CoopBlutSymbole(held, attr, last))
            return false;
        if (sprung ? code[at] != 75 : (code[at] != 80 || *(const int*)(code + at + 1) != held))
            return false;
        return code[at + 5] == 65 && *(const int*)(code + at + 6) == attr && code[at + 10] == 65 && *(const int*)(code + at + 11) == last &&
            code[at + 15] == 1;
    }

    // u252: alter Anfang im ganzen Skriptcode (Sprung des Toolkits auf eines der Ziele oder schon wiederhergestellt; nicht die Ziele
    // selbst - LeGo 2.9 beginnt _B_HeroDamage genauso); -1 = keiner oder nicht eindeutig
    static int CoopBlutAltSuchen(const unsigned char* code, int size, const std::set<int>& ziele) {
        static int merk = -1;
        if (merk >= 0 && !ziele.count(merk) && (CoopBlutAlt(code, size, merk, true) || CoopBlutAlt(code, size, merk, false)))
            return merk;
        int gefunden = -1, n = 0;
        for (int at = 0; at + 16 <= size; at++) {
            if ((code[at] != 75 && code[at] != 80) || code[at + 5] != 65 || code[at + 10] != 65 || code[at + 15] != 1)
                continue;   // billige Vorpruefung: PUSHVAR, PUSHVAR, SUB dahinter
            if (ziele.count(at))
                continue;
            if (code[at] == 75 ? (ziele.count(*(const int*)(code + at + 1)) && CoopBlutAlt(code, size, at, true)) : CoopBlutAlt(code, size, at, false)) {
                gefunden = at;
                n++;
            }
        }
        merk = n == 1 ? gefunden : -1;
        return merk;
    }

    static void CoopBlutAltZurueck(unsigned char* code, int at) {   // ersten Befehl des alten Anfangs zurueck: SETINST HERO
        int held, attr, last;
        if (!CoopBlutSymbole(held, attr, last))
            return;
        code[at] = 80;
        *(int*)(code + at + 1) = held;
    }

    struct CoopBlutStand {
        int schleife = 0;      // Aufrufe, die weiterreichen (ohne Ende, wenn LeGo hierher verkettet)
        int wieder = 0;        // davon: alter Anfang wiederhergestellt, der Aufruf bleibt
        int entschaerft = 0;   // davon: Muster unbekannt - Sprung auf den naechsten Befehl
        int alt = 0;           // frueher entschaerfte Aufrufe (Sprung auf den naechsten Befehl hinter einer Bedingung)
        int eingesetzt = 0;    // davon wieder eingesetzt (alter Anfang da)
    };

    // Aufrufe von _B_HeroDamage in der Funktion fid; aendern: behandeln wie oben beschrieben
    static bool CoopBlutScan(int fid, bool aendern, std::string& name, CoopBlutStand& st) {
        int anf = 0, ende = 0;
        if (!CoopBlutBereich(fid, anf, ende, name))
            return false;
        unsigned char* code = parser->stack.stack;
        const int size = parser->stack.stacksize;
        std::set<int> ziele;
        CoopBlutZiele(ziele);
        if (ziele.empty())
            return false;
        int prevQ = -1, prevOp = -1;
        for (int q = anf; q < ende;) {
            const int op = code[q];
            const int len = CoopBlutLen(op);
            if (q + len > size)
                break;
            const int arg = len == 5 ? *(const int*)(code + q + 1) : 0;
            if (op == 61 && (ziele.count(arg) || ziele.count(CoopBlutZiel(code, size, arg)))) {
                st.schleife++;
                if (aendern) {
                    if (!ziele.count(arg) && CoopBlutAlt(code, size, arg, true)) {
                        CoopBlutAltZurueck(code, arg);
                        st.wieder++;
                    }
                    else {
                        code[q] = 75;   // JMP auf den naechsten Befehl
                        *(int*)(code + q + 1) = q + 5;
                        st.entschaerft++;
                    }
                }
            }
            else if (op == 75 && arg == q + 5 && prevOp == 76 && *(const int*)(code + prevQ + 1) == q + 5) {
                // "if (...) { _B_HeroDamage(); }" mit entschaerftem Aufruf (JMPF und JMP auf denselben naechsten Befehl)
                const int alt = CoopBlutAltSuchen(code, size, ziele);
                if (alt >= 0) {
                    st.alt++;
                    if (aendern) {
                        if (code[alt] == 75)
                            CoopBlutAltZurueck(code, alt);
                        code[q] = 61;
                        *(int*)(code + q + 1) = alt;
                        st.eingesetzt++;
                    }
                }
            }
            prevQ = q;
            prevOp = op;
            q += len;
        }
        return true;
    }

    // jede Sekunde im Spiel (Plugin.cpp): billig - eine Zahl lesen, gesucht wird nur, wenn LeGo eine andere Wahrnehmung verkettet
    void CoopBlutFix(const char* wann) {
        if (!parser)
            return;
        static zCPar_Symbol* pf = NULL;
        static bool gesucht = false;
        if (!gesucht) {
            gesucht = true;
            pf = parser->GetSymbol(zSTRING("_B_HERODAMAGEPERCFUNC"));
        }
        if (!pf)
            return;   // LeGo bis 2.7 verkettet nicht - die Schleife gibt es dort nicht
        const int fid = pf->single_intdata;
        if (fid <= 0 || fid == CoopBlutFid)
            return;
        CoopBlutFid = fid;
        std::string name;
        CoopBlutStand st;
        if (!CoopBlutScan(fid, true, name, st))
            return;
        static int zeilen = 0;
        if (zeilen++ >= 10)
            return;
        CoopBlutFixed += st.wieder + st.entschaerft + st.eingesetzt;
        if (st.wieder)
            OthelloCompatLog("Blutspritzer-Schleife (P300, %s): %s ruft _B_HeroDamage selbst auf, und LeGo verkettet die Wahrnehmung des Helden "
                "dorthin - beide riefen sich ohne Ende auf (Absturz bei Fernkampftreffern); alter Anfang von _B_HeroDamage (LeGo bis 2.7) "
                "wiederhergestellt: der Aufruf zeigt den Blutspritzer mit dem echten Schaden und reicht nicht weiter (P302)", wann, name.c_str());
        if (st.entschaerft)
            OthelloCompatLog("Blutspritzer-Schleife (P300, %s): %s ruft _B_HeroDamage selbst auf, und LeGo verkettet die Wahrnehmung des Helden "
                "dorthin; alter Anfang nicht im erwarteten Muster - %d Aufruf(e) entschaerft (Sprung auf den naechsten Befehl, den Blutspritzer "
                "zeigt nur LeGos Wahrnehmung)", wann, name.c_str(), st.entschaerft);
        if (st.eingesetzt)
            OthelloCompatLog("Blutspritzer-Schleife (P300, %s): in %s war der Aufruf von _B_HeroDamage entschaerft (Skript-Paket Xeres-Fixes "
                "1.1.0 oder GT u249-u251) - wieder eingesetzt, er geht an den alten Anfang: Blutspritzer mit dem echten Schaden (P302)",
                wann, name.c_str());
        if (!st.wieder && !st.entschaerft && !st.eingesetzt)
            OthelloCompatLog("Blutspritzer-Schleife (P300, %s): LeGo verkettet die Wahrnehmung des Helden nach %s - ruft _B_HeroDamage nicht "
                "(mehr) so auf (schon behandelt, z. B. vom Skript-Paket Xeres-Fixes)", wann, name.c_str());
    }

    // Coop-Testlauf: Aufrufe, die weiterreichen, in der verketteten Wahrnehmung jetzt (soll 0); -1 = LeGo verkettet (noch) nicht
    int CoopBlutTestOffen(std::string& name) {
        if (!parser)
            return -1;
        zCPar_Symbol* pf = parser->GetSymbol(zSTRING("_B_HERODAMAGEPERCFUNC"));
        if (!pf)
            return -1;
        CoopBlutStand st;
        return CoopBlutScan(pf->single_intdata, false, name, st) ? st.schleife : -1;
    }


} // namespace GOTHIC_ENGINE
