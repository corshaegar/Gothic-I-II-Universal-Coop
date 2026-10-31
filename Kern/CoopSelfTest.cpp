// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- Coop-Testlauf: 100 Abgleich-Pruefungen (v107, ersetzt den Lauf v37-v48) --------------------------------------------
    //
    // Nutzer (23.10.2026): "Als naechstes erstelle einen Test, der 100 verschiedene Mechaniken/Hooks auf Synchronisation testet,
    // auf Shift+F11 wie wir davor gemacht haben, die alten Tests koennen weg."
    //
    // Shift + CoopSimKey (Standard F11) startet, nochmal Shift+F11 bricht ab. Je Schritt wie bisher (Paket 46 SELFTEST):
    //   1. der Partner meldet seinen Stand (Frage "pre"), 2. dieser PC handelt wie im Spiel (Skripte/Externals wie in Dialogen,
    //   Engine-Funktionen wie beim Spielen), 3. nach einer Wartezeit meldet der Partner, was er jetzt sieht ("post"), 4. dieser PC
    //   bewertet: OK / FEHLER / -- (uebersprungen, nicht bewertbar) / BEKANNT (bekannte Luecke).
    // Sieben Bereiche:
    //   1 Verbindung und Grundstand (13): Rundlauf, Fassung, Welt, Zeit, Regeln, Schwierigkeit, Engine-Aenderungen, Durchlauf,
    //     Speicherbereitschaft, Netz-Warteschlange, Absturzschutz, Log-Versand.
    //   2 Eigene Figur beim Partner (31): jedes Paket von LocalNpc -> RemoteNpc (Lage, Blickrichtung, Name und Aussehen ueber
    //     INIT_NPC, Leben, Waffenmodus, Nah-/Fernkampfwaffe, Ruestung, Talente, Schutz, Overlays, Animation) und was beim eigenen
    //     Helden bleiben muss (Staerke, Erfahrung ohne Skript).
    //   3 Story und Tagebuch (18): Dialog-Weg (DlgSendDiff) und Story-Takt getrennt, Skript-Variable COOP_TEST_VAR (COOP_ITEMS.D
    //     ab v107), Heldenvariable, Ruf (nur Othello), Gilden, Kapitel und Vergleiche ganzer Bestaende.
    //   4 Test-NPCs und Welt-Ereignisse (17): Wld_InsertNpc ueber den Host, Fuehrung, Lage, Blickrichtung, Leben, Treffer, Tod,
    //     EP-Regel, Haltung, Trigger/Untrigger, Wld_InsertItem ueber den Host (u27, bis u26 bekannte Luecke), Kampf-Uebergabe.
    //   5 NPCs um mich (7): Verfolgungspunkt, Liste, Leben, Waffenmodus, Blickrichtung, Animationen, tot/lebendig.
    //   6 Gegenstaende und Truhen (9): Truhe +-1/+-3, Inhalt, alle Truhen, Boden, Aufheben, Welt-Abgleich neu (nur Client).
    //   7 Coop-Funktionen (5): Teleport-Rune, ihr Aussehen, Mitspieler-Markierung, Zuschau-Fenster an/aus.
    //   8 Speichern und Laden (5, u30): gemeinsam speichern (der Partner speichert mit, sein Held dazu), gemeinsam laden, danach
    //     Held erhalten, Figuren da, gleiche Welt. Am Ende, weil das Laden die Welt neu aufbaut.
    // u29/u30 Bereich 1 dazu: Weltschluessel beim Partner, getrennte Welten erkannt / keine Figur / keine NPCs aus der anderen Welt
    // (nur bei getrennten Welten), wer die NPCs des Clients fuehrt. Ist der Partner beim Start in einer anderen Welt (seit u29 steht
    // seine Figur dann nicht hier - bis u30 hiess es "kein Mitspieler verbunden"), laufen nur Schritte, die keine gemeinsame Welt
    // brauchen (CTAnyWorldSteps), der Rest meldet "--". Bereiche ueber den Namen ihres ersten Schritts (bis u29 feste Nummern).
    // Vergleiche ganzer Bestaende (Skript-Variablen, Tagebuch, Gildentabelle, Tagesablauf/Haltung aller NPCs, tote NPCs, alle
    // Truhen, Gegenstaende am Boden): die Frage traegt 64 Pruefsummen (Fach = FNV-1a des Schluessels mod 64, Summe der
    // FNV-1a von "Schluessel=Wert"), der Partner schickt nur die Eintraege der abweichenden Faecher (hoechstens 800), dieser PC
    // nennt die Abweichungen im Log (hoechstens 8 je Art und Schritt).
    // Aenderungen am eigenen Helden nimmt der jeweilige Schritt danach zurueck, beim Abbruch CTRestoreAll. Bleibend: Tagebuch-Thema
    // "Coop-Test", Quest-EP, zwei tote Test-NPCs, Uhrzeit +30 min (Host), ein aufgehobener Gegenstand - nur auf einem
    // Test-Spielstand benutzen. Einmal am Host und einmal am Client starten (einige Schritte gibt es nur dort).
    // v108 (Test v107: Host 80 OK/8 FEHLER, Client 87 OK/5 FEHLER; beide Laeufe liefen gleichzeitig): kein Start, solange der
    // Partner testet; "noch nicht" wird bis 8 s nach der Handlung nachgefragt (CTF_ONCE/CTF_LONG); Kampf-Uebergabe gilt als
    // gefuehrt; Fernkampf-Schritt mit Munition und eigenem Modus; Geste "--", wenn sie beim eigenen Helden schon vorbei ist; fehlende
    // NPCs um mich mit Namen im Log; Truhe bis 30 m, Gegenstand bis 20 m (COOP-FIXES D12d). Gefundene Fehler: H16c, I7, A21, G4c, C22.
    // v109 (Test v108: Host 90 OK/3 FEHLER, Client 92 OK/5 FEHLER, nacheinander): Test-Molerat friedlich bis zum Treffer (er toetete
    // den Host-Helden), EP-Grenze aus dem Schwierigkeitsgrad, Fleischwanze mit toedlichem Schlag (8 HP bei "Leben der Gegner"
    // 150 %), "abgeglichen" nur unter den gepaarten NPCs, Truhe bis 50 m (COOP-FIXES D12e). Gefundene Fehler: D13c, C18d.
    // v110 (Test v109: Host 97 OK/1 FEHLER, Client 95 OK/1 FEHLER): der einzige Fehler war ein echter - Bloodwyns Tagesablauf
    // (COOP-FIXES D19). Am Test nichts geaendert.
    // u39 (P82; Test u38: Host im Minental, Partner in Khorinis - 70 von 110 Schritten "--"; Nutzer: "die ganzen Tests wurden
    // uebersprungen, das sollte aber nicht so sein"): ist der Partner beim Start in einer anderen Welt, holt ihn Schritt 3 her (Paket
    // SELFTEST "go": er wechselt wie beim gemeinsamen Weltwechsel und tritt neben mich), dann laufen alle Schritte. Neuer Bereich 9
    // "Getrennte Welten und Weltprozess": Partner in eine andere Welt (zurueck in seine bzw. die zweite Hauptwelt), Trennung, Weltprozess
    // bereit und im Zuschau-Fenster (P80), gemeinsam speichern/laden in verschiedenen Welten mit Weltstand-Tausch (P81: CRC der
    // Weltdateien in Slot und Saves\current auf beiden PCs gleich), Teleport ueber Welten zu mir (N2, dabei kein zweiter Held aus der
    // getauschten Weltdatei), Ausgangslage. CTF_PARTNERWAIT: bis 150 s nachfragen, auch ohne Antwort (Partner laedt).
    // u40 (Test u39: Host 116 OK/1 FEHLER, Client 114 OK/1 FEHLER, Bereich 9 auf beiden 17/17; Nutzer: "Wir haben jedoch immer noch
    // nicht die Synchronisation ueber Welten hinweg getestet"): in Bereich 9 laufen Uhrzeit, Heldenwerte und alle Story-Schritte aus
    // Bereich 3 noch einmal, waehrend der Partner in der anderen Welt ist ("ueber Welten", eigene Tagebuch-Kennung CTStoryTag) - 144
    // Schritte. Spawn-Schritte zaehlen und suchen den Test-NPC um den Wegpunkt statt um mich (D12h).
    // Die Datei steht seit v107 am Ende von Sources.h: sie liest den Zustand vieler Module direkt (Story-Abgleich, Schwierigkeit,
    // Spielstaende, Rune, Markierung, Kampf-Uebergabe, Absturzschutz). Alle benutzten Engine-Funktionen nutzt GT schon oder sie
    // sind laut Speicherabbild unveraendert (COOP-FIXES D12c).

#if COOP_BUILD(SelfTest)   // u3: bis u2 nur G1 - Freigabe je Engine in CoopCore.cpp

    json CoopAuditSummaryJson(bool refresh);    // CoopAudit.cpp
    void CoopAuditDumpToLog(const char* why);

    enum { CT_START, CT_PRE, CT_WAIT, CT_POST, CT_GAP };
    enum { CTF_HOST = 1, CTF_GAP = 2, CTF_CLIENT = 4, CTF_OTHELLO = 8 };   // nur Host, bekannte Luecke, nur Client, nur Othello
    // v108 (Test v107): ein Urteil "noch nicht" fragt den Partner bis 8 s nach der Handlung erneut (alle 0,8 s) - Abgleiche brauchen
    // je nach Weg 1-3 s (Story-Takt, Log-Versand, Glaetten). CTF_ONCE: nur einmal (Momentaufnahmen, "bleibt gleich"),
    // CTF_LONG: bis 20 s (Log-Versand nach grossen Bloecken).
    enum { CTF_ONCE = 16, CTF_LONG = 32 };
    // u30: nur, wenn der Partner in einer anderen Welt ist (Pruefungen der Trennung, P65)
    enum { CTF_SEPWORLD = 64 };
    // u30: Schritt laedt einen Spielstand - nachfragen erst 4 s nach dem eigenen Ladevorgang, bis 90 s, Antwort bis 20 s
    enum { CTF_LOAD = 128 };
    // u39: der Partner wechselt die Welt bzw. der Weltprozess laedt - bis 150 s alle 1,5 s nachfragen, auch wenn der Partner gerade
    // nicht antwortet (er laedt)
    enum { CTF_PARTNERWAIT = 256 };

    struct CoopTestDef {
        const char* name;
        const char* ref;
        int waitMs;
        int flags;
        bool (*prep)(std::string& why);                              // Ziel suchen; false = uebersprungen
        json (*query)();                                             // Frage an den Partner (vorher und nachher)
        bool (*act)(std::string& why);                               // Handlung; false = uebersprungen
        bool (*judge)(const json& pre, const json& post, std::string& d);
        void (*cleanup)();
    };

    struct CoopTestBlock {
        const char* firstStep;   // u30: Name des ersten Schritts (bis u29 feste Nummer - verrutschte mit jedem neuen Schritt)
        const char* title;
    };

    typedef std::map<std::string, std::string> CTKv;

    // u30: Held fuer den Vergleich vor dem Speichern / nach dem Laden
    struct CTHeroSnap {
        int lvl = -1, exp = 0, str = 0, dex = 0, hpMax = 0;
        std::string world;
        float pos[3] = { 0.f, 0.f, 0.f };   // u31: Lage (nach dem Laden wieder hier, auch in einer anderen Welt als der Partner)
    };

    struct CoopTestRun {
        bool on = false;
        std::string tag;
        size_t idx = 0;
        int phase = CT_START;
        long long deadline = 0;
        json pre, post;
        bool havePre = false, havePost = false;
        long long askMs = 0;
        int rttMs = -1;
        long long actMs = 0;          // v108: Zeitpunkt der Handlung (Nachfragen bis 8 bzw. 20 s danach)
        int retries = 0;
        float myHeadAsk = -1.f;       // v108: eigene Blickrichtung bei der Frage "post"
        std::string partner;          // Kennung des Partners (aus der Antwort)
        bool otherWorld = false;      // u30: Partner war beim Start in einer anderen Welt (nur weltunabhaengige Schritte)
                                      // u39: jetzt "Partner gerade in einer anderen Welt" - der Test holt ihn her und schickt ihn weg
        std::string otherText;        //      "Name: Welt" fuer Meldungen
        bool startedApart = false;    // u39: beim Start in verschiedenen Welten (am Ende schickt der Test ihn dorthin zurueck)
        bool worldsOnly = false;      // u44: Welten-Testlauf (Shift+F11; voller Lauf mit Strg) - Schritte 1-3 und Bereich 9
        bool checksOnly = false;      // u50: Pruefrunde (lange Laeufe) - nur Vergleiche, keine Handlungen
        bool guildsOnly = false;      // u86: Gilden-Testlauf (tools\cooptest_lauf.py --gilden) - Schritte 1-4, Regeln, Gilden je Held
        // u30: Bereich "Speichern und Laden"
        int saveSlot = -1;            // Slot der Testsicherung
        std::string saveSid;          // ihre Kennung (gesetzt, wenn der Partner mitgespeichert hat)
        std::string sidBefore;        // Kennung, die vorher in diesem Slot lag (eine alte Auto-Sicherung zaehlt nicht)
        CTHeroSnap heroAtSave, partnerAtSave;
        bool expectLoad = false;      // der naechste eigene Ladevorgang gehoert zum Test
        int loadsSeen = 0;            // CoopWorldLoads beim letzten Bild
        long long loadedMs = 0;       // wann der erwartete Ladevorgang fertig war
        long vidNo = 0;               // u55: Nummer des Test-Videos (Story-Video ueberspringen)
        bool bkOk = false;            // u55: Sicherung beim Partner liegt auf beiden Seiten
        bool bkHidden = false;        // u55: Partner hat seine eigene Kopie beiseitegelegt (Test: verlorene Saves)
        int ok = 0, fail = 0, skip = 0, gap = 0;
        int bOk[10] = {}, bFail[10] = {}, bSkip[10] = {}, bGap[10] = {};   // u30: acht Bereiche (Platz fuer mehr)
        std::vector<std::string> fails;
        CTKv dig;                     // eigener Bestand zur letzten Vergleichsfrage
        // Zustand zwischen den Schritten
        float headWant = 0.f;
        bool headTurned = false;
        std::string nameOld, nameNew;
        bool nameChanged = false;
        int faceOld = -1, faceIniOld = -1, faceNew = -1, faceWant = -1;
        bool faceChanged = false;
        int hpOld = 0, hpMaxOld = 0, strOld = 0, talOld = -1, talNew = -1, protOld = 0;
        unsigned long expOld = 0;
        oCItem* weapon = NULL;        // vom Test angelegt (sonst NULL)
        oCItem* ranged = NULL;
        oCItem* armor = NULL;
        oCItem* ammo = NULL;          // v108: Munition fuer den Fernkampf-Schritt (vom Test angelegt)
        int meleeMode = 0, rangedMode = 0, ownMode = -1;
        std::string overlay, gesture;
        bool overlayOn = false;
        int varValue = 0;
        bool varSet = false;
        std::string heroVar;
        int heroVarOld = 0;
        bool heroVarSet = false;
        int repOld = -1;
        bool repSet = false;
        int guildOld1 = -1, guildOld2 = -1, guildNew = -1;
        bool guildSet = false;
        int gldValue = 0;             // u86 (P110): Wert der Testvariable im letzten Gilden-Schritt
        std::string wpName, molName, bugName;
        zVEC3 wpPos;                  // u40: Lage des Spawn-Wegpunkts (gezaehlt und gesucht wird dort, nicht um mich)
        bool haveWp = false;
        float partnerDist = -1.f;
        bool farKill = false;
        int myActive = -1;
        oCMobContainer* chest = NULL;
        zVEC3 chestPos;
        int itemsBefore = -1;
        std::string pickInst;
        zVEC3 pickPos;
        std::string marker;
    };
    static CoopTestRun CT;
    static long long CTPartnerQueryMs = -100000;   // v108: zuletzt eine Frage aus einem Lauf des Partners (keine zwei Laeufe zugleich)
    static std::string CTPartnerRun;
    static long long CTPartnerEndMs = 0;           // u49: Lauf des Partners zu Ende (SELFTEST "end") - fuer CoopAutoTest.cpp
    static std::string CTPartnerEndText;

    // u39: Weltwechsel auf Wunsch des Testenden (Antwortseite, Paket SELFTEST "go"). w = Zielwelt (leer: zurueck an die gemerkte
    // Stelle), p/a = Lage und Blickrichtung des Testenden (daneben setzen, CoopPlaceBesideHost), keep = vorher eigene Welt und Lage
    // merken. Gewechselt wird wie beim gemeinsamen Weltwechsel (CoopWcGo, ohne Abfrage), gesetzt 1,5 s nach dem Laden.
    struct CTGoOrder {
        bool on = false;
        std::string world, target, from, run;
        bool havePos = false, exact = false, keep = false, tp = false;
        bool mark = false;   // u47 (P88): statt Weltwechsel eigene Welt markieren (Betreten-Schritte des Testenden)
        zVEC3 pos, at;
        int phase = 0;
        int loadsAt = 0;
        long long since = 0, loadedMs = 0;
    };
    static CTGoOrder CTGo;
    static bool CTHomeSet = false;
    static std::string CTHomeWorld;
    static zVEC3 CTHomePos, CTHomeAt;

    // u47 (P88): Markierung fuer die Betreten-Schritte - eine getoetete Fleischwanze und ein Apfel am naechsten Wegpunkt. Beides
    // steht danach nur in der Weltdatei dessen, der markiert hat; wer die Welt betritt, sieht es nur, wenn sein Stand geholt wurde.
    struct CTMark {
        bool have = false;          // Fleischwanze tot, Apfel gelegt
        std::string world, wp;
        zVEC3 pos;
        std::set<oCNpc*> before;    // Fleischwanzen vor dem Einsetzen (die neue erkennen)
        long long insertedMs = 0;
        std::string entryVar;       // u140 (P139): Probe - Eintritts-Merker dieser Welt, von mir gesetzt
    };
    static CTMark CTMarkMine;   // Testender: "Ich markiere meine Welt"
    static CTMark CTMarkMade;   // Antwortseite: auf Wunsch des Testenden ("go" mit "mark")
    static bool CTMarkStart(CTMark& m, std::string& why);
    static int CTMarkFinish(CTMark& m);   // 1 fertig, 0 noch nicht, -1 gescheitert
    static void CTCountMark(const zVEC3& pos, int& dead, int& apples);

    static const float CT_NEAR_R = 2000.f;
    static const char* CT_VAR = "COOP_TEST_VAR";   // COOP_ITEMS.D ab v107

    // u12 (Gothic 2): Instanzen je Spiel - die erste, die die Skripte kennen (G1/Othello zuerst, dann G2 NotR)
    static const char* CTInstOf(std::initializer_list<const char*> names) {
        for (const char* n : names)
            if (parser && parser->GetIndex(zSTRING(n)) >= 0)
                return n;
        return *names.begin();
    }

    static const char* CTApple() { return CTInstOf({ "ITFOAPPLE", "ITFO_APPLE" }); }

    // Gildenpaar, das sich kaum begegnet: Othello GIL_TROLL 20 / GIL_GOLEM 35; u12: aus den Skripten (G2: Troll 45, Drache 47)
    static int CTGuildConst(const char* name, int def) {
        zCPar_Symbol* s = parser ? parser->GetSymbol(zSTRING(name)) : NULL;
        int v = def;
        if (s && s->type == zPAR_TYPE_INT)
            s->GetValue(v, 0);
        return v;
    }

    static int CTG1() { return CTGuildConst("GIL_TROLL", 20); }
    static int CTG2() { return parser && parser->GetSymbol(zSTRING("GIL_GOLEM")) ? CTGuildConst("GIL_GOLEM", 35) : CTGuildConst("GIL_DRAGON", 35); }

    // u12: wie TpInWorld/TpRune (Teleport-Modul) - der Testlauf braucht sie auch, wenn die Rune (G2A) noch aus ist
    static bool CTInWorld(oCNpc* n) {
        return n && n != player && n->GetHomeWorld() && n->vobLeafList.GetNum() > 0 && n->GetModel();
    }

    // u51 (Durchsicht u50, Befund 6): NPC steht wirklich in dieser Welt (auch tot). voblist_npcs fuehrt auch NPCs der Spawn-Liste,
    // die nicht eingesetzt sind (DisableVob: aus dem BSP, Heimatwelt leer; neue auf 0/0/0 - im Minental 1,5 m unter der Burg)
    static bool CTNpcPlaced(oCNpc* n) {
        return n && ogame && n->GetHomeWorld() == (zCWorld*)ogame->GetGameWorld() && n->vobLeafList.GetNum() > 0;
    }

    static int CTRuneInst() {
        return parser ? parser->GetIndex(zSTRING("ITRU_COOP_TELEPORT")) : -1;
    }

    static void CTLog(const char* fmt, ...) {
        char buf[700];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Coop-Test: %s", DiagSeconds(), DiagRole(), buf);
    }

    static void CTSay(const char* fmt, ...) {
        char buf[500];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        ChatLog(string(buf));
        CTLog("%s", buf);
    }

    static std::string CTFmt(const char* fmt, ...) {
        char buf[700];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        return buf;
    }

    static json CTVec(const zVEC3& v) {
        return json::array({ v.n[0], v.n[1], v.n[2] });
    }

    static zVEC3 CTVecOf(const json& j) {
        return zVEC3(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>());
    }

    static int CTInt(const json& o, const char* a, const char* b = NULL, int def = -999) {
        try {
            if (!o.contains(a))
                return def;
            const json& x = b ? o.at(a) : o;
            if (b && !x.contains(b))
                return def;
            const json& v = b ? x.at(b) : x.at(a);
            return v.is_number() ? v.get<int>() : def;
        }
        catch (...) {
            return def;
        }
    }

    static float CTFloat(const json& o, const char* a, const char* b, float def = -1e9f) {
        try {
            if (o.contains(a) && o.at(a).contains(b) && o.at(a).at(b).is_number())
                return o.at(a).at(b).get<float>();
        }
        catch (...) {}
        return def;
    }

    static std::string CTStr(const json& o, const char* a, const char* b) {
        try {
            if (o.contains(a) && o.at(a).contains(b) && o.at(a).at(b).is_string())
                return o.at(a).at(b).get<std::string>();
        }
        catch (...) {}
        return std::string();
    }

    static std::string CTUpper(std::string s) {
        for (auto& c : s)
            c = (char)toupper((unsigned char)c);
        return s;
    }

    static bool CTHas(const json& arr, const std::string& want) {
        if (!arr.is_array())
            return false;
        const std::string w = CTUpper(want);
        for (auto& x : arr)
            if (x.is_string() && CTUpper(x.get<std::string>()) == w)
                return true;
        return false;
    }

    // Winkelabstand in Grad (0..180)
    static float CTAngle(float a, float b) {
        float d = fmodf(fabsf(a - b), 360.f);
        return d > 180.f ? 360.f - d : d;
    }

    static int CTGuild(int a, int b) {
        oCGuilds* g = ogame ? ogame->GetGuilds() : NULL;
        if (!g || !g->attitude || a >= g->size || b >= g->size)
            return -1;
        return g->attitude[b * g->size + a];
    }

    static int CTCountNpcs(const std::string& inst, const zVEC3& p, float r) {
        const int idx = parser->GetIndex(zSTRING(inst.c_str()));
        int n = 0;
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (npc && npc->GetInstance() == idx && !npc->IsDead() && CoopDist(npc->GetPositionWorld(), p) <= r)
                n++;
        }
        return n;
    }

    // naechster lebender, vom Host nachtraeglich erzeugter NPC (GT-Name "<Objekt>-DYNAMIC-<Wegpunkt>-n") dieser Instanz
    static oCNpc* CTFindDynamic(const char* inst) {
        oCNpc* best = NULL;
        float bestD = 3000.f;
        const int idx = parser->GetIndex(zSTRING(inst));
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (!npc || npc->GetInstance() != idx || npc->IsDead())
                continue;
            if (DialogNpcName(npc).find("-DYNAMIC-") == std::string::npos)
                continue;
            // u40 (Test u39, Client: Wegpunkt 35 m entfernt, der neue Molerat lag ausserhalb der 30 m um mich): um den Spawn-Wegpunkt
            const float d = CoopDist(npc->GetPositionWorld(), CT.haveWp ? CT.wpPos : player->GetPositionWorld());
            if (d < bestD) {
                bestD = d;
                best = npc;
            }
        }
        return best;
    }

    static int CTCountItems(const std::string& inst, const zVEC3& p, float r) {
        const int idx = parser->GetIndex(zSTRING(inst.c_str()));
        if (idx < 0)
            return -1;
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, (int)r);
        int n = 0;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(vobs.GetSafe(i));
            if (it && it->GetInstance() == idx && CoopItemLiegtFrei(it) && CoopDist(it->GetPositionWorld(), p) <= r)
                n += CoopItemAmount(it);
        }
        return n;
    }

    // freie Gegenstaende (Stueck Vobs, nicht Menge) einer Instanz hoechstens r um p
    static int CTCountItemVobs(const std::string& inst, const zVEC3& p, float r, oCItem** nearest = NULL) {
        const int idx = parser->GetIndex(zSTRING(inst.c_str()));
        if (idx < 0)
            return -1;
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, (int)r + 50);
        int n = 0;
        float bestD = r + 1.f;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(vobs.GetSafe(i));
            if (!it || it->GetInstance() != idx || !CoopItemLiegtFrei(it))
                continue;
            const float d = CoopDist(it->GetPositionWorld(), p);
            if (d > r)
                continue;
            n++;
            if (nearest && d < bestD) {
                bestD = d;
                *nearest = it;
            }
        }
        return n;
    }

    static oCMobContainer* CTNearestChest(const zVEC3& p, float r) {
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, (int)r);
        oCMobContainer* best = NULL;
        float bestD = r;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCMobContainer* c = zDYNAMIC_CAST<oCMobContainer>(vobs.GetSafe(i));
            if (!c)
                continue;
            const float d = CoopDist(c->GetPositionWorld(), p);
            if (d < bestD) {
                bestD = d;
                best = c;
            }
        }
        return best;
    }

    // lebende, gerade aktive NPCs (in der Welt) im Umkreis von p - ohne Helden und Coop-Figuren
    static int CTCountActive(const zVEC3& p, float r) {
        int n = 0;
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (!npc || npc == player || PlayerNpcs.count(npc) || npc->IsDead() || npc->vobLeafList.GetNum() == 0)
                continue;
            if (CoopDist(npc->GetPositionWorld(), p) < r)
                n++;
        }
        return n;
    }

    // Abstand zur naechsten Figur eines Mitspielers (hier)
    static float CTPartnerDist() {
        float best = -1.f;
        for (auto& kv : PlayerNpcs) {
            if (!kv.first)
                continue;
            zVEC3 p = kv.first->GetPositionWorld();
            auto it = SyncNpcs.find(kv.second);
            if (it != SyncNpcs.end() && it->second && it->second->lastPositionFromServer)
                p = *it->second->lastPositionFromServer;
            const float d = CoopDist(p, player->GetPositionWorld());
            if (best < 0 || d < best)
                best = d;
        }
        return best;
    }

    static std::string CTReceivedLogFile(const std::string& from) {
        return CoopLogPartnerPath(!ServerThread, from);   // u1: Namen je Profil (CoopCore.cpp)
    }

    static bool CTOthello() {
        return CoopProfileIsOthello();   // u6: Profil (CoopCore.cpp) statt nur OTHELLO_ABI.DLL
    }

    // Overlays (ohne das Grundmodell) und laufende Animationen eines NPCs - nur Felder lesen (modelProtoList, aniChannels)
    static json CTOverlays(oCNpc* n) {
        json a = json::array();
        zCModel* m = n ? n->GetModel() : NULL;
        if (!m)
            return a;
        for (int i = 1; i < m->modelProtoList.GetNumInList(); i++)
            if (zCModelPrototype* p = m->modelProtoList.GetSafe(i))
                a.push_back(CTUpper(p->modelProtoFileName.ToChar()));
        return a;
    }

    static json CTAnis(oCNpc* n) {
        json a = json::array();
        zCModel* m = n ? n->GetModel() : NULL;
        if (!m)
            return a;
        for (int i = 0; i < m->numActiveAnis && i < zMDL_MAX_ANIS_PARALLEL; i++) {
            zCModelAniActive* x = m->aniChannels[i];
            if (x && x->protoAni)
                a.push_back(CTUpper(x->protoAni->GetAniName().ToChar()));
        }
        return a;
    }

    static std::string CTItemName(oCItem* it) {
        return it ? std::string(it->GetInstanceName().ToChar()) : std::string();
    }

    // Figur (Coop-Mitspieler) mit allem, was LocalNpc/RemoteNpc abgleichen
    static json CTFigJson(oCNpc* f) {
        json x;
        x["found"] = f ? 1 : 0;
        if (!f)
            return x;
        x["act"] = f->vobLeafList.GetNum() > 0 ? 1 : 0;
        x["p"] = CTVec(f->GetPositionWorld());
        x["h"] = GetHeading(f);
        x["wm"] = f->GetWeaponMode();
        x["melee"] = CTItemName(f->GetEquippedMeleeWeapon());
        x["ranged"] = CTItemName(f->GetEquippedRangedWeapon());
        oCItem* rw = f->GetEquippedRangedWeapon();
        x["ammo"] = (rw && rw->munition > 0 && f->IsInInv(rw->munition, 1)) ? 1 : 0;   // v108
        x["armor"] = CTItemName(f->GetEquippedArmor());
        x["hp"] = f->GetAttribute(NPC_ATR_HITPOINTS);
        x["hm"] = f->GetAttribute(NPC_ATR_HITPOINTSMAX);
        x["tal"] = json::array({ f->GetTalentSkill(oCNpcTalent::NPC_TAL_1H), f->GetTalentSkill(oCNpcTalent::NPC_TAL_2H),
            f->GetTalentSkill(oCNpcTalent::NPC_TAL_BOW), f->GetTalentSkill(oCNpcTalent::NPC_TAL_CROSSBOW) });
        json prot = json::array();
        for (int i = 0; i < oEDamageIndex_MAX; i++)
            prot.push_back(f->GetProtectionByIndex((oEIndexDamage)i));
        x["prot"] = prot;
        x["ovl"] = CTOverlays(f);
        x["ani"] = CTAnis(f);
        x["name"] = std::string(f->name[0].ToChar());
        x["hv"] = CTUpper(f->head_visualName.ToChar());
        x["ht"] = (int)f->head_TexVarNr;
        x["bv"] = CTUpper(f->body_visualName.ToChar());
        x["bt"] = (int)f->body_TexVarNr;
        x["bc"] = (int)f->body_TexColorNr;
        return x;
    }

    // NPC (Test-NPC oder NPC um mich): Lage, Zustand, beim Partner gesendet/gefuehrt
    static json CTNpcJson(oCNpc* b, const std::string& name) {
        json x;
        x["found"] = b ? 1 : 0;
        x["host"] = ServerThread ? 1 : 0;
        if (!b)
            return x;
        x["p"] = CTVec(b->GetPositionWorld());
        x["act"] = b->vobLeafList.GetNum() > 0 ? 1 : 0;
        x["dead"] = b->IsDead() ? 1 : 0;
        x["hp"] = b->GetAttribute(NPC_ATR_HITPOINTS);
        x["hm"] = b->GetAttribute(NPC_ATR_HITPOINTSMAX);
        x["att"] = b->attitude;
        const CoopNpcFields fields = DlgNpcFields(b);   // u139 (P138)
        x["fl"] = fields.flags;
        x["gu"] = fields.guild;
        x["gt"] = fields.guildTrue;
        x["nt"] = fields.npcType;
        x["nf"] = fields.noFocus;
        x["h"] = GetHeading(b);
        x["wm"] = b->GetWeaponMode();
        x["ani"] = CTAnis(b);
        x["s"] = std::string(b->state.curState.name.ToChar());
        // v108: Kampf-Uebergabe (C18) - fuehrt gerade ein Client den NPC im Kampf, sendet der Host ihn nicht (Test v107, Schritt 64)
#if COOP_READY_CombatLead   // u12: Modul je Engine (G2A noch aus) - dann fuehrt nie ein Client
        if (ServerThread) {
            auto l = CoopLeadsHost.find(name);
            x["lead"] = l != CoopLeadsHost.end() ? l->second.owner : std::string();
        }
        else
            x["lead"] = CoopLeadsMine.count(name) ? DialogMe() : std::string();
#else
        x["lead"] = std::string();
#endif
        if (ServerThread)
            x["sent"] = BroadcastNpcs.count(string(name.c_str())) ? 1 : 0;
        else {
            auto it = SyncNpcs.find(string(name.c_str()));
            x["synced"] = (it != SyncNpcs.end() && it->second && !it->second->destroyed && it->second->hasModel) ? 1 : 0;
        }
        return x;
    }

    // Regeln, die im Coop gelten (beim Client die des Hosts) - CoopDifficulty.cpp
    static json CTRulesJson() {
        json x;
#if COOP_READY_Difficulty   // u12: Modul je Engine (G2A noch aus - die Schritte dazu melden "--")
        const DiffRules& r = DiffEff();
        x["p"] = r.preset; x["dp"] = r.dp; x["de"] = r.de; x["hp"] = r.hp; x["xa"] = r.xa; x["xp"] = r.xp; x["xr"] = r.xr;
        x["sq"] = r.sq; x["sl"] = r.sl; x["sw"] = r.sw; x["ss"] = r.ss; x["sa"] = r.sa; x["st"] = r.st;
#endif
        return x;
    }

    // u12: Beschreibung des Schwierigkeitsgrads fuer Logzeilen - ohne das Modul "Vorgabe"
    static std::string CTDiffText() {
#if COOP_READY_Difficulty
        return DiffDescribe(DiffEff());
#else
        return "ohne Schwierigkeits-Modul";
#endif
    }

    // Absturz-Filter: liegt GT noch vorn (A18b)? Wie CoopCrashFilterPulse - danach liegt er wieder vorn.
    static bool CTCrashFront() {
        LPTOP_LEVEL_EXCEPTION_FILTER cur = SetUnhandledExceptionFilter(&CoopTopCrashFilter);
        if (cur == &CoopTopCrashFilter)
            return true;
        if (cur != CoopNextCrashFilter)
            CoopNextCrashFilter = cur;
        return false;
    }

    // --- Bestaende vergleichen (Pruefsummen je Fach) ---------------------------------------------------------------------

    static const int CT_BUCKETS = 64;

    static unsigned CTFnv(const std::string& s) {
        unsigned h = 2166136261u;
        for (unsigned char c : s) {
            h ^= c;
            h *= 16777619u;
        }
        return h;
    }

    static json CTBuckets(const CTKv& m) {
        std::vector<unsigned> b(CT_BUCKETS, 0u);
        for (auto& kv : m)
            b[CTFnv(kv.first) % CT_BUCKETS] += CTFnv(kv.first + "=" + kv.second);
        json a = json::array();
        for (unsigned x : b)
            a.push_back(x);
        return a;
    }

    // Partner: Eintraege der Faecher, deren Pruefsumme von der des Fragenden abweicht
    static json CTDigAnswer(const CTKv& m, const json& theirs) {
        const json mine = CTBuckets(m);
        std::set<int> diff;
        for (int i = 0; i < CT_BUCKETS; i++) {
            bool same = false;
            try {
                same = theirs.is_array() && i < (int)theirs.size() && theirs[i].get<unsigned>() == mine[i].get<unsigned>();
            }
            catch (...) {}
            if (!same)
                diff.insert(i);
        }
        json e = json::object();
        int n = 0;
        bool cut = false;
        for (auto& kv : m) {
            if (!diff.count((int)(CTFnv(kv.first) % CT_BUCKETS)))
                continue;
            if (n >= 800) {
                cut = true;
                break;
            }
            e[kv.first] = kv.second;
            n++;
        }
        json o;
        o["b"] = json::array();
        for (int i : diff)
            o["b"].push_back(i);
        o["e"] = e;
        o["n"] = (int)m.size();
        o["cut"] = cut ? 1 : 0;
        return o;
    }

    // Skript-Ganzzahlen wie der Story-Abgleich (ohne Heldenwerte, ausgeschlossene und staendig wechselnde)
    static void CTMapVars(CTKv& m) {
        StoryReadConfig();
        DlgBuildIntSlots();
        const bool noisyKnown = StoryNoisy.size() == DlgIntSlots.size();
        for (size_t i = 0; i < DlgIntSlots.size(); i++) {
            if (noisyKnown && StoryNoisy[i])
                continue;
            zCPar_Symbol* s = DlgIntSlots[i].sym;
            const std::string nm = s->name.ToChar();
            if (StoryIgnored(nm))   // u48: auch ..._RANDOMIZER (CoopDialogSync.cpp)
                continue;
            int v = 0;
            s->GetValue(v, DlgIntSlots[i].idx);
            if (!StoryValueOk(v))
                continue;
            m[s->ele > 1 ? nm + "[" + std::to_string(DlgIntSlots[i].idx) + "]" : nm] = std::to_string(v);
        }
    }

    static void CTMapLog(CTKv& m) {
        std::vector<CoopDlgTopic> log;
        DlgSnapLog(log);
        for (auto& t : log)
            m[t.t] = std::to_string(t.st) + "|" + std::to_string(t.e.size());
    }

    // dead: alle Welt-NPCs tot/lebendig; sonst lebende mit Tagesablauf, dauerhafter Haltung und Skriptfeldern (wie der Story-Takt)
    static void CTMapNpcs(CTKv& m, bool dead) {
        for (auto* li = CoopWorldNpcListFirst(ogame->GetGameWorld()); li; li = li->next) {
            oCNpc* npc = li->data;
            if (StoryNpcSkip(npc))
                continue;
            const std::string name = DialogNpcName(npc);
            if (name.empty())
                continue;
            if (dead) {
                m[name] = npc->IsDead() ? "1" : "0";
                continue;
            }
            if (npc->IsDead() || StoryNpcNoisy.count(name))
                continue;
            zCPar_Symbol* rs = parser->GetSymbol(npc->daily_routine);
            const CoopNpcFields f = DlgNpcFields(npc);   // u139 (P138)
            m[name] = std::string(rs ? rs->name.ToChar() : "-") + "|" + std::to_string(npc->attitude) + "|" + std::to_string(f.flags) + "|"
                + std::to_string(f.guild) + "|" + std::to_string(f.guildTrue) + "|" + std::to_string(f.npcType) + "|" + std::to_string(f.noFocus);
        }
    }

    static void CTMapGuilds(CTKv& m) {
        oCGuilds* g = ogame ? ogame->GetGuilds() : NULL;
        if (!g || !g->attitude || g->size <= 0 || g->size > 256)
            return;
        for (int b = 0; b < g->size; b++)
            for (int a = 0; a < g->size; a++)
                m[std::to_string(a) + "," + std::to_string(b)] = std::to_string((int)g->attitude[b * g->size + a]);
    }

    static void CTMapChests(CTKv& m) {
        zCArray<zCVob*> chests;
        ogame->GetGameWorld()->SearchVobListByClass(oCMobContainer::classDef, chests, NULL);
        for (int i = 0; i < chests.GetNumInList(); i++)
            if (oCMobContainer* c = zDYNAMIC_CAST<oCMobContainer>(chests.GetSafe(i)))
                m[CoopPosKey(c->GetPositionWorld())] = std::string(c->locked ? "zu|" : "offen|") + BestandText(ChestBestand(c));
    }

    // u137 (P134): alle Mechanismen. Bei Tueren nur das Schloss - offen oder zu steht eine Tuer je PC verschieden, NPCs benutzen sie fuer sich
    static void CTMapMobs(CTKv& m) {
        zCArray<zCVob*> mobs;
        ogame->GetGameWorld()->SearchVobListByBaseClass(oCMobInter::classDef, mobs, NULL);
        for (int i = 0; i < mobs.GetNumInList(); i++) {
            oCMobInter* x = zDYNAMIC_CAST<oCMobInter>(mobs.GetSafe(i));
            if (!MobSyncRelevant(x))
                continue;
            oCMobLockable* l = zDYNAMIC_CAST<oCMobLockable>(x);
            m[CoopPosKey(x->GetPositionWorld())] = (zDYNAMIC_CAST<oCMobDoor>(x) ? std::string() : "Zustand " + std::to_string((int)x->state)) + (l && l->locked ? " zu" : "");
        }
    }

    static void CTMapLoose(const zVEC3& p, float r, CTKv& m) {
        std::map<std::string, int> cnt;
        zCArray<zCVob*> vobs = CollectVobsInRadius(p, (int)r);
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(vobs.GetSafe(i));
            if (it && CoopItemLiegtFrei(it) && CoopDist(it->GetPositionWorld(), p) <= r)
                cnt[CoopItemInst(it)] += CoopItemAmount(it);
        }
        for (auto& kv : cnt)
            m[kv.first] = std::to_string(kv.second);
    }

    static void CTBuildMap(const json& d, CTKv& m) {
        const std::string k = d.value("k", std::string());
        if (k == "vars")
            CTMapVars(m);
        else if (k == "log")
            CTMapLog(m);
        else if (k == "npcs")
            CTMapNpcs(m, false);
        else if (k == "dead")
            CTMapNpcs(m, true);
        else if (k == "guilds")
            CTMapGuilds(m);
        else if (k == "chests")
            CTMapChests(m);
        else if (k == "mobs")
            CTMapMobs(m);   // u137
        else if (k == "loose" && d.contains("p"))
            CTMapLoose(CTVecOf(d["p"]), d.value("r", 2000.f), m);
    }

    static json CTDigQuery(const char* kind, bool aroundMe = false) {
        json d;
        d["k"] = kind;
        if (aroundMe) {
            d["p"] = CTVec(player->GetPositionWorld());
            d["r"] = CT_NEAR_R;
        }
        CT.dig.clear();
        CTBuildMap(d, CT.dig);
        d["b"] = CTBuckets(CT.dig);
        json q;
        q["dig"] = d;
        return q;
    }

    struct CTDigRes {
        int mism = 0, onlyMine = 0, onlyTheirs = 0, nMine = 0, nTheirs = 0, buckets = 0;
        bool cut = false;
        std::vector<std::string> ex;
    };

    static bool CTDigEval(const json& post, CTDigRes& r) {
        if (!post.contains("dig") || !post["dig"].contains("b") || !post["dig"].contains("e"))
            return false;
        const json& o = post["dig"];
        const json& e = o["e"];
        r.nMine = (int)CT.dig.size();
        r.nTheirs = o.value("n", -1);
        r.cut = o.value("cut", 0) == 1;
        std::set<int> diff;
        for (auto& b : o["b"])
            if (b.is_number())
                diff.insert(b.get<int>());
        r.buckets = (int)diff.size();
        int exM = 0, exA = 0, exB = 0;
        for (auto& kv : CT.dig) {
            if (!diff.count((int)(CTFnv(kv.first) % CT_BUCKETS)))
                continue;
            auto f = e.find(kv.first);
            if (f == e.end() || !f->is_string()) {
                r.onlyMine++;
                if (exA++ < 8)
                    r.ex.push_back(kv.first + ": nur hier (" + kv.second + ")");
                continue;
            }
            const std::string theirs = f->get<std::string>();
            if (theirs != kv.second) {
                r.mism++;
                if (exM++ < 8)
                    r.ex.push_back(kv.first + ": hier " + kv.second + ", Partner " + theirs);
            }
        }
        for (auto it = e.begin(); it != e.end(); ++it)
            if (!CT.dig.count(it.key())) {
                r.onlyTheirs++;
                if (exB++ < 8)
                    r.ex.push_back(it.key() + ": nur beim Partner (" + (it->is_string() ? it->get<std::string>() : std::string("?")) + ")");
            }
        return true;
    }

    static std::string CTDigText(const CTDigRes& r, const char* what) {
        return CTFmt("%s: hier %d, beim Partner %d; abweichend %d, nur hier %d, nur beim Partner %d%s", what, r.nMine, r.nTheirs,
            r.mism, r.onlyMine, r.onlyTheirs, r.cut ? " (Liste gekuerzt)" : "");
    }

    // mode: 0 nur Werte zaehlen, 1 auch fehlende Eintraege
    static bool CTDigJudge(const json& post, const char* what, int mode, std::string& d) {
        CTDigRes r;
        if (!CTDigEval(post, r)) {
            d = "Partner meldet keinen Bestand";
            return false;
        }
        d = CTDigText(r, what);
        for (auto& s : r.ex)
            CTLog("  %s", s.c_str());
        return r.mism == 0 && (mode == 0 || (r.onlyMine == 0 && r.onlyTheirs == 0));
    }

    // u30: NPCs, die hier ein anderer PC fuehrt (RemoteNpc ohne Spielerfigur, mit Modell in der Welt)
    static int CTLedHere() {
        int n = 0;
        for (auto& s : SyncNpcs)
            if (s.second && !s.second->destroyed && s.second->hasModel && !IsCoopPlayer(s.first))
                n++;
        return n;
    }

    // --- Partner: melden, was er sieht ---------------------------------------------------------------------------------
    // u39: zweite Helden in der Welt (Instanz des eigenen Helden, nicht er selbst, keine Coop-Figur) - CoopRemoveStrayHeroes nimmt sie
    // nach dem Laden heraus, hier sollen es 0 sein
    static int CTStrayHeroes() {
        if (!player || !ogame || !ogame->GetGameWorld())
            return -1;
        int n = 0;
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next)
            if (l->data && l->data != player && l->data->GetInstance() == player->GetInstance() && !PlayerNpcs.count(l->data))
                n++;
        return n;
    }

    // u86 (P110): Gilden je Held - Testgilde des Partners (harmlos: Bauer) und eine andere fuer "fremde" Gilden-Quests
    static int CTTestGuild() { return CTGuildConst("GIL_BAU", 8); }
    static int CTOtherGuild() {
        const int g = CTGuildConst("GIL_PAL", 1);
        return g != CTTestGuild() && g != 0 ? g : CTTestGuild() + 1;
    }

    static int CTInvCount(const char* inst) {
        if (!player || !parser)
            return -1;
        const int idx = parser->GetIndex(zSTRING(inst));
        int n = 0;
        player->inventory2.UnpackAllItems();
#if ENGINE >= Engine_G2
        for (zCListSort<oCItem>* l = player->inventory2.inventory.next; l; l = l->next)
            if (l->data && l->data->GetInstance() == idx)
                n += l->data->amount;
#else
        for (int c = 0; c < INV_MAX; c++)
            for (zCListSort<oCItem>* l = player->inventory2.inventory[c].next; l; l = l->next)
                if (l->data && l->data->GetInstance() == idx)
                    n += l->data->amount;
#endif
        return n;
    }

    // eigene Gilde vor dem Setzen auf die Testgilde (-1000 = nicht gesetzt); zurueck mit "back" oder am Ende des Laufs des Partners
    static int CTGldOrig = -1000;

    static void CTGldRestore(const char* why) {
        if (CTGldOrig != -1000 && player) {
            player->guild = CTGldOrig;
            CTLog("Gilden je Held: eigene Gilde wieder %d (%s)", CTGldOrig, why);
        }
        CTGldOrig = -1000;
        if (zCPar_Symbol* s = parser ? parser->GetSymbol(zSTRING(CT_VAR)) : NULL)
            DlgGuildVars.erase(s);   // wieder ein Story-Wert wie vor den Gilden-Schritten
    }

    static json CoopTestObserve(const json& q, const std::string& from) {
        json o = json::object();
        if (q.contains("gld")) {   // u86 (P110): eigene Gilde (auf Wunsch des Testenden gesetzt bzw. zurueck), Testvariable, Aepfel, Thema
            const json& g = q["gld"];
            if (player && g.contains("set") && g["set"].is_number_integer()) {
                if (CTGldOrig == -1000)
                    CTGldOrig = player->guild;
                player->guild = g["set"].get<int>();
            }
            if (player && g.contains("back"))
                CTGldRestore("Testender: zurueck");
            json x;
            x["g"] = player ? player->guild : -1;
            int v = 0;
            if (zCPar_Symbol* s = parser->GetSymbol(zSTRING(CT_VAR)))
                s->GetValue(v, 0);
            x["var"] = v;
            x["apples"] = CTInvCount(CTApple());
            oCLogTopic* tp = DialogFindTopic("Coop-Test Gilde");
            x["tex"] = tp ? 1 : 0;
            x["tst"] = tp ? (int)tp->m_enuStatus : -1;
            o["gld"] = x;
        }
        if (q.contains("mi") && q["mi"].is_string() && !q["mi"].get<std::string>().empty()) {   // u96 (P118): Missions-Gegenstand
            json x;
            x["n"] = CTInvCount(q["mi"].get<std::string>().c_str());
            x["mark"] = CoopMissionTestMark(q["mi"].get<std::string>().c_str());   // u104 (P126): Kennzeichnung
            o["mi"] = x;
        }
        if (q.contains("truhe") && q["truhe"].is_string()) {   // u97 (P119), u99 (P121): Coop-Truhe
            // auf Wunsch des Testenden erst handeln (Halten und Nehmen je Kennung einmal - die Frage kommt mehrmals), dann melden:
            // Menge der Instanz in der Kopie hier, wer sie offen hat (laut Host), eigenes Fenster, Rueckgaben, Menge im Inventar
            const std::string inst = q["truhe"].get<std::string>();
            static std::string haltId, nimmId;
            if (q.contains("truheHalten") && q["truheHalten"].is_string() && q["truheHalten"].get<std::string>() != haltId) {
                haltId = q["truheHalten"].get<std::string>();
                CoopTruheTestHalten(true);
            }
            const json* nimm = q.contains("truheNimm") ? &q["truheNimm"] : NULL;
            if (nimm && nimm->is_array() && nimm->size() >= 3 && (*nimm)[0].is_string() && (*nimm)[1].is_number_integer() &&
                (*nimm)[2].is_number_integer() && (*nimm)[0].get<std::string>() != nimmId) {
                nimmId = (*nimm)[0].get<std::string>();
                CoopTruheTestNimm(inst.c_str(), (*nimm)[1].get<int>(), (*nimm)[2].get<int>() != 0);
                CoopTruheTestHalten(false);
            }
            if (q.contains("truheFenster") && q["truheFenster"].is_number_integer())
                CoopTruheTestFenster(q["truheFenster"].get<int>());
            if (q.contains("truheZu"))
                CoopTruheTestZu();
            json x;
            int n = 0;
            std::string offen;
            CoopTruheTestStand(n, offen, inst.c_str());
            x["n"] = n;
            x["offen"] = offen;
            x["auf"] = CoopTruheTestOffenHier() ? 1 : 0;
            x["rueck"] = CoopTruheTestRueckgaben();
            x["inv"] = CTInvCount(inst.c_str());
            o["truhe"] = x;
        }
        if (q.contains("base")) {
            json x;
            int day = 0, h = 0, m = 0;
            ogame->GetTime(day, h, m);
            x["ver"] = COOP_FASSUNG;
            x["world"] = std::string(ogame->GetGameWorld()->GetWorldFilename().ToChar());
            x["wk"] = CoopWorldKey();              // u29 (P65): eigener Weltschluessel
            x["wkme"] = CoopPeerWorldKey(from);    //            und die Welt, die der Fragende zuletzt gemeldet hat
            x["led"] = CTLedHere();                // u30: vom Partner gefuehrte NPCs mit Modell hier
            x["alone"] = CoopClientAlone() ? 1 : 0;   // u30 (P66): Client ohne Host in seiner Welt
            x["loads"] = CoopWorldLoads;           // u30: Ladevorgaenge (Speichern und Laden)
            x["day"] = day;
            x["min"] = h * 60 + m;
            x["talk"] = IsPlayerTalkingWithAnybody() ? 1 : 0;   // u74: im Gespraech uebernimmt der Client die Uhr des Hosts nicht
            x["run"] = CoopSaveSyncCurrentRun();
            std::string why;
#if COOP_READY_Save   // u12: Modul je Engine
            x["save"] = CoopCanSaveNow(why) ? 1 : 0;
#else
            why = "Speichern-Modul aus";
            x["save"] = 0;
#endif
            x["sw"] = why;
            x["rules"] = CTRulesJson();
            x["q"] = ReadyToSendJsons.size();
            x["veh"] = CoopVehHandle ? 1 : 0;
            x["top"] = CTCrashFront() ? 1 : 0;
            x["id"] = std::string(MyselfId.ToChar());
            x["host"] = ServerThread ? 1 : 0;
            x["stray"] = CTStrayHeroes();   // u39
            x["going"] = (CTGo.on || CoopTeleportAcrossBusy()) ? 1 : 0;   // u39: Weltwechsel/Teleport fuer den Test laeuft noch
            json figs = json::array();
            for (auto& kv : PlayerNameToNpc) {
                json f;
                string id = kv.first;
                f["id"] = std::string(id.ToChar());
                f["act"] = CTInWorld(kv.second) ? 1 : 0;
                figs.push_back(f);
            }
            x["figs"] = figs;
            o["base"] = x;
        }
        if (q.contains("emark")) {   // u47 (P88): meine Markierung fuers Betreten (auf Wunsch des Testenden, "go" mit "mark")
            json x;
            x["have"] = CTMarkMade.have ? 1 : 0;
            x["w"] = CTMarkMade.world;
            x["wp"] = CTMarkMade.wp;
            x["ev"] = CTMarkMade.entryVar;   // u140 (P139)
            if (CTMarkMade.insertedMs) {
                int dead = 0, apples = 0;
                if (CoopWcBase(CTMarkMade.world) == CoopWcBase(CoopWorldName()))
                    CTCountMark(CTMarkMade.pos, dead, apples);
                x["p"] = CTVec(CTMarkMade.pos);
                x["dead"] = dead;
                x["apples"] = apples;
            }
            o["emark"] = x;
        }
        if (q.contains("markchk") && q["markchk"].contains("p") && q["markchk"]["p"].is_array()) {   // u47: Markierung des Testenden hier?
            json x;
            int dead = 0, apples = 0;
            CTCountMark(CTVecOf(q["markchk"]["p"]), dead, apples);
            x["dead"] = dead;
            x["apples"] = apples;
            x["w"] = CoopWorldName();
            o["markchk"] = x;
        }
        if (q.contains("auto") && q["auto"].is_object()) {   // u105 (P127): Auto-Sicherung alle 3 min
            // Host auf Wunsch des testenden Clients (je Kennung einmal): 3 min, letzte Sicherung vor 2:58 min. Dann melden: Regel,
            // Auto-Sicherungen, Slot der letzten und Kennung (im erfragten Slot), ms vom Umstellen bis faellig, Sicherung frei
            const json& a = q["auto"];
            static std::string autoId;
            static long long autoLosMs = 0;
            const std::string id = a.value("id", std::string());
            if (ServerThread && a.contains("los") && !id.empty() && id != autoId) {
                autoId = id;
                CoopRuleAutoSaveTest(3, 40000);
                CoopSaveSyncTestAutoAgo(3 * 60000 - 2000);
                autoLosMs = CurrentMs;
            }
            int zahl = 0, slot = -1;
            long long faellig = 0;
            bool frei = true;
            CoopSaveSyncTestAutoInfo(zahl, slot, faellig, frei);
            const int frage = a.contains("slot") && a["slot"].is_number_integer() ? a["slot"].get<int>() : slot;
            json x;
            x["min"] = CoopRuleAutoSaveMinutes();
            x["zahl"] = zahl;
            x["slot"] = slot;
            x["sid"] = frage >= 0 ? CoopSaveSyncSlotSid(frage) : std::string();
            x["nach"] = autoLosMs && faellig >= autoLosMs ? (int)(faellig - autoLosMs) : -1;
            x["frei"] = frei ? 1 : 0;
            o["auto"] = x;
        }
        if (q.contains("save")) {   // u30: Speichern und Laden - Kennung im Slot, eigener Held dazu, Welt, Werte des Helden
            json x;
            const int slot = q["save"].value("slot", -1);
            const std::string asked = q["save"].value("sid", std::string());
            x["sid"] = slot >= 0 ? CoopSaveSyncSlotSid(slot) : std::string();
            x["hero"] = (!asked.empty() && CoopSaveSyncHasOwnHero(asked)) ? 1 : 0;
            x["world"] = CoopWorldName();
            x["lvl"] = player->level;
            x["exp"] = (int)player->experience_points;
            x["str"] = player->GetAttribute(NPC_ATR_STRENGTH);
            x["dex"] = player->GetAttribute(NPC_ATR_DEXTERITY);
            x["hpmax"] = player->GetAttribute(NPC_ATR_HITPOINTSMAX);
            x["p"] = CTVec(player->GetPositionWorld());   // u31
            o["save"] = x;
        }
        if (q.contains("vid")) {   // u55: Story-Video - zuletzt gestartete und zuletzt wegen des Partners beendete Nummer
            json x;
            long begun = 0, remote = 0, sent = 0;
            CoopStoryVideoState(begun, remote, sent);
            x["begun"] = begun;
            x["stopped"] = remote;
            o["vid"] = x;
        }
        if (q.contains("bk")) {   // u55: Sicherung beim Partner - Stand des Fragenden hier?, eigene PC-Kennung, zurueckgeholt?
            json x;
            const std::string sid = q["bk"].value("sid", std::string()), pc = q["bk"].value("pc", std::string());
            x["have"] = CoopSaveSyncHasBackupOf(sid, pc) ? 1 : 0;
            x["pc"] = CoopSaveSyncMyPc();
            x["restored"] = (!sid.empty() && CoopSaveSyncLastRestoredSid() == sid) ? 1 : 0;
            if (q["bk"].value("hide", 0) == 1) {   // Lauf des Hosts: verlorene Saves am Client nachstellen (nur dort)
                std::string what;
                x["gone"] = CoopSaveSyncTestHideOwnCopy(sid, what) ? 1 : 0;
                x["what"] = what;
            }
            o["bk"] = x;
        }
        if (q.contains("load")) {
            json x;
            x["n"] = CoopWorldLoads;
            x["sid"] = CoopSaveSyncLastLoadSid();
            o["load"] = x;
        }
        if (q.contains("wx")) {   // u39: Weltstand-Tausch - CRC der Weltdateien im Slot und in Saves\current, offene Abholungen
            json x;
            const int slot = q["wx"].value("slot", -1);
            json ws = json::object();
            if (q["wx"].contains("w") && q["wx"]["w"].is_array())
                for (auto& w : q["wx"]["w"]) {
                    if (!w.is_string())
                        continue;
                    unsigned cs = 0, cc = 0;
                    long long ss = 0, sc = 0;
                    json f;
                    f["s"] = CoopSaveSyncWorldFileCrc(slot, w.get<std::string>(), cs, ss) ? (long long)cs : -1LL;
                    f["c"] = CoopSaveSyncWorldFileCrc(-1, w.get<std::string>(), cc, sc) ? (long long)cc : -1LL;
                    ws[w.get<std::string>()] = f;
                }
            x["w"] = ws;
            x["pend"] = CoopSaveSyncWxPending();
            x["sid"] = slot >= 1 ? CoopSaveSyncSlotSid(slot) : std::string();
            o["wx"] = x;
        }
        if (q.contains("audit")) {   // Fingerabdruck der Selbstpruefung (hoechstens alle 10 s neu abgetastet)
#if COOP_READY_Audit   // u12: Modul je Engine (G2A: eigene Haken-Tabelle fehlt noch)
            static long long lastScan = -100000;
            const bool refresh = CurrentMs - lastScan > 10000;
            if (refresh)
                lastScan = CurrentMs;
            json x = CoopAuditSummaryJson(refresh);
            if (x.value("h", std::string()) != q["audit"].value("h", std::string()) ||
                x.value("rh", std::string()) != q["audit"].value("rh", std::string()))
                CoopAuditDumpToLog("Testlauf: weicht vom Partner ab");
            o["audit"] = x;
#else
            o["audit"] = json{ { "ok", 0 }, { "err", "Selbstpruefung in dieser Engine aus" } };
#endif
        }
        if (q.contains("fig"))
            o["fig"] = CTFigJson(DialogFigureOf(q["fig"].get<std::string>()));
        if (q.contains("hero")) {
            json x;
            x["str"] = player->GetAttribute(NPC_ATR_STRENGTH);
            x["dex"] = player->GetAttribute(NPC_ATR_DEXTERITY);
            x["lvl"] = player->level;
            x["exp"] = (long long)player->experience_points;
            o["hero"] = x;
        }
        if (q.contains("var") && q["var"].is_array()) {
            json x = json::object();
            for (auto& n : q["var"]) {
                zCPar_Symbol* s = n.is_string() ? parser->GetSymbol(zSTRING(n.get<std::string>().c_str())) : NULL;
                if (!s || s->type != zPAR_TYPE_INT)
                    continue;
                int v = 0;
                s->GetValue(v, 0);
                x[n.get<std::string>()] = v;
            }
            o["var"] = x;
        }
        if (q.contains("topic")) {
            oCLogTopic* tp = DialogFindTopic(q["topic"].get<std::string>());
            const std::string tag = q.value("tag", std::string()), tag2 = q.value("tag2", std::string());
            json t;
            t["ex"] = tp ? 1 : 0;
            if (tp) {
                int n = 0, has = 0, has2 = 0;
                for (auto* e = tp->m_lstEntries.GetNextInList(); e; e = e->GetNextInList())
                    if (oCLogEntry* le = e->GetData()) {
                        n++;
                        if (!tag.empty() && strstr(le->m_strDescription.ToChar(), tag.c_str()))
                            has = 1;
                        if (!tag2.empty() && strstr(le->m_strDescription.ToChar(), tag2.c_str()))
                            has2 = 1;
                    }
                t["st"] = (int)tp->m_enuStatus;
                t["n"] = n;
                t["tag"] = has;
                t["tag2"] = has2;
            }
            o["topic"] = t;
        }
        if (q.contains("exp"))
            o["exp"] = player ? (long long)player->experience_points : -1;
        if (q.contains("guild"))
            o["guild"] = json::array({ CTGuild(CTG1(), CTG2()), CTGuild(CTG2(), CTG1()) });
        if (q.contains("count")) {   // NPCs einer Instanz um die Stelle des Ausloesers (der Partner kann woanders stehen)
            const json& c = q["count"];
            o["count"] = CTCountNpcs(c["inst"].get<std::string>(), CTVecOf(c["p"]), 2500.f);
        }
        if (q.contains("npc")) {
            const std::string name = q["npc"].get<std::string>();
            o["npc"] = CTNpcJson(DialogNpcByName(name), name);
        }
        if (q.contains("chest")) {
            const json& c = q["chest"];
            oCMobContainer* ch = FindChest(CTVecOf(c["p"]));
            json x;
            x["found"] = ch ? 1 : 0;
            if (ch) {
                CoopBestand b = ChestBestand(ch);
                auto f = b.find(c["inst"].get<std::string>());
                x["n"] = f == b.end() ? 0 : f->second;
                x["all"] = BestandText(b);
            }
            o["chest"] = x;
        }
        if (q.contains("mob")) {   // u137 (P134): Mechanismus und das Tor dahinter
            const json& c = q["mob"];
            oCMobInter* m = FindMob(CTVecOf(c["p"]));
            zCMover* mv = zDYNAMIC_CAST<zCMover>(ogame->GetGameWorld()->SearchVobByName(zSTRING(c["z"].get<std::string>().c_str())));
            json x;
            x["found"] = m ? 1 : 0;
            x["s"] = m ? (int)m->state : -1;
            x["ms"] = mv ? (int)mv->moverState : -1;
            o["mob"] = x;
        }
        if (q.contains("items")) {
            const json& c = q["items"];
            o["items"] = CTCountItems(c["inst"].get<std::string>(), CTVecOf(c["p"]), 2500.f);
        }
        if (q.contains("item")) {   // freie Gegenstaende einer Instanz an genau dieser Stelle (Aufheben)
            const json& c = q["item"];
            o["item"] = CTCountItemVobs(c["inst"].get<std::string>(), CTVecOf(c["p"]), 120.f);
        }
        if (q.contains("far")) {   // Verfolgungspunkt - sieht der Partner meine Figur und die NPCs um mich?
            const zVEC3 p = CTVecOf(q["far"]["p"]);
            json x;
            x["fol"] = CoopFollowActive() ? 1 : 0;
            x["dist"] = CoopDist(player->GetPositionWorld(), p);
            oCNpc* f = DialogFigureOf(from);
            x["fig"] = f ? 1 : 0;
            x["figact"] = (f && f->vobLeafList.GetNum() > 0) ? 1 : 0;
            x["act"] = CTCountActive(p, (float)BROADCAST_DISTANCE);
            o["far"] = x;
        }
        if (q.contains("near")) {   // NPCs um die Stelle des Ausloesers - auch tote (Feld d)
            const zVEC3 p = CTVecOf(q["near"]["p"]);
            const float r = q["near"].value("r", 2500.f);
            json arr = json::array();
            int cnt = 0;
            for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
                oCNpc* npc = l->data;
                // u51 (Befund 6): nur eingesetzte NPCs - in der Minental-Burg fuellten die 0/0/0-NPCs der Spawn-Liste die 60 Plaetze,
                // die echten fehlten beim Vergleich ("fehlen/nicht in der Welt")
                if (!npc || npc == player || PlayerNpcs.count(npc) || !CTNpcPlaced(npc))
                    continue;
                auto nm = NpcToUniqueNameList.find(npc);
                if (nm == NpcToUniqueNameList.end() || CoopDist(npc->GetPositionWorld(), p) >= r)
                    continue;
                cnt++;
                if (arr.size() >= 60)
                    continue;
                json n;
                n["n"] = std::string(nm->second.ToChar());
                n["p"] = CTVec(npc->GetPositionWorld());
                n["s"] = std::string(npc->state.curState.name.ToChar());
                n["a"] = npc->vobLeafList.GetNum() > 0 ? 1 : 0;
                n["d"] = npc->IsDead() ? 1 : 0;
                n["hp"] = npc->GetAttribute(NPC_ATR_HITPOINTS);
                n["wm"] = npc->GetWeaponMode();
                n["h"] = GetHeading(npc);
                n["an"] = CTAnis(npc);
                if (ServerThread)
                    n["l"] = BroadcastNpcs.count(nm->second) ? 1 : 0;
                else {
                    auto it = SyncNpcs.find(nm->second);
                    n["l"] = (it != SyncNpcs.end() && it->second && !it->second->destroyed && it->second->hasModel) ? 1 : 0;
                }
                arr.push_back(n);
            }
            json x;
            x["l"] = arr;
            x["cnt"] = cnt;
            x["host"] = ServerThread ? 1 : 0;
            o["near"] = x;
        }
        if (q.contains("logmark")) {
            const std::string mark = q["logmark"].get<std::string>();
            int has = 0;
            if (!mark.empty())
                if (FILE* f = fopen(CTReceivedLogFile(from).c_str(), "rb")) {
                    std::string all;
                    char buf[8192];
                    size_t got;
                    while ((got = fread(buf, 1, sizeof(buf), f)) > 0)
                        all.append(buf, got);
                    fclose(f);
                    has = all.find(mark) != std::string::npos ? 1 : 0;
                }
            o["logmark"] = has;
        }
        if (q.contains("dig")) {
            CTKv m;
            CTBuildMap(q["dig"], m);
            o["dig"] = CTDigAnswer(m, q["dig"].contains("b") ? q["dig"]["b"] : json());
        }
        if (q.contains("trig")) {   // Host-Ereignisse (CoopDialogSync.cpp: Trigger/Untrigger, hier ausgefuehrt bzw. angefragt)
            json x;
            x["n"] = CoopTestTrigCount;
            x["last"] = CoopTestTrigLast;
            o["trig"] = x;
        }
        if (q.contains("lead")) {   // Kampf-Uebergabe: Host - abgegebene NPCs mit Besitzer, Client - selbst gefuehrte
            json x;
            x["host"] = ServerThread ? 1 : 0;
            json a = json::array();
#if COOP_READY_CombatLead   // u12: Module je Engine - aus: leer bzw. "nicht da"
            if (ServerThread)
                for (auto& kv : CoopLeadsHost)
                    a.push_back(json::array({ kv.first, kv.second.owner }));
            else
                for (auto& kv : CoopLeadsMine)
                    a.push_back(kv.first);
#endif
            x["l"] = a;
            o["lead"] = x;
        }
        if (q.contains("rune")) {
            json x;
#if COOP_READY_Teleport
            const int idx = TpRune();
            oCItem* it = (idx >= 0 && player) ? player->IsInInv(idx, 1) : NULL;
            x["idx"] = idx;
            x["has"] = it ? 1 : 0;
            x["st"] = TpLookState;
            x["look"] = TpRuneHasOwnLook(it) ? 1 : 0;   // u89: ueber den Modellnamen (P113)
#else
            x["idx"] = -1;
            x["has"] = 0;
            x["st"] = -1;
            x["look"] = 0;
#endif
            o["rune"] = x;
        }
        if (q.contains("mark")) {   // Mitspieler-Markierung: wann zuletzt fuer meine Figur gezeichnet
            oCNpc* f = DialogFigureOf(from);
            json x;
            x["inw"] = CTInWorld(f) ? 1 : 0;
#if COOP_READY_Marker
            x["hud"] = PmHudOff() ? 1 : 0;
            auto it = f ? PmShownAt.find(f) : PmShownAt.end();
            const long long age = it == PmShownAt.end() ? -1 : GetCurrentMs() - it->second.first;
            x["age"] = age > 999999 ? 999999 : (int)age;
            x["mode"] = it == PmShownAt.end() ? -1 : it->second.second;
#else
            x["hud"] = 0;
            x["age"] = -1;
            x["mode"] = -1;
#endif
            o["mark"] = x;
        }
        return o;
    }

    // --- Fragen -----------------------------------------------------------------------------------------------------------

    static json QBase() { json q; q["base"] = 1; return q; }
    static json QFig() { json q; q["fig"] = DialogMe(); return q; }
    static json QHero() { json q; q["hero"] = 1; return q; }
    // u40: Story-Schritte laufen zweimal (Bereich 3 in derselben Welt, Bereich 9 ueber Welten) - eigene Kennung je Runde, sonst
    // faende die zweite Runde den Eintrag der ersten
    static std::string CTStoryTag() { return CT.otherWorld ? CT.tag + "-W" : CT.tag; }
    static json QTopic() { json q; q["topic"] = "Coop-Test"; q["tag"] = CTStoryTag(); q["tag2"] = "Story-Takt " + CTStoryTag(); return q; }
    static json QExp() { json q; q["exp"] = 1; return q; }
    static json QGuild() { json q; q["guild"] = 1; return q; }
    static json QGldSet() { json q; q["gld"] = json{ { "set", CTTestGuild() } }; return q; }   // u86
    static json QGld() { json q; q["gld"] = json::object(); return q; }
    static json QGldBack() { json q; q["gld"] = json{ { "back", 1 } }; return q; }
    static json QVarTest() { json q; q["var"] = json::array({ CT_VAR }); return q; }
    static json QVarHero() { json q; q["var"] = json::array({ CT.heroVar }); return q; }
    static json QVarRep() { json q; q["var"] = json::array({ "REPUTATIONOC" }); return q; }
    static json QVarChapter() { json q; q["var"] = json::array({ "KAPITEL" }); return q; }
    // u40: um den Spawn-Wegpunkt zaehlen (bis zu 40 m von mir - um mich lag er im Test u39 ausserhalb der 25 m)
    static json QCountMol() { json q; q["count"]["inst"] = "MOLERAT"; q["count"]["p"] = CTVec(CT.haveWp ? CT.wpPos : player->GetPositionWorld()); return q; }
    static json QCountBug() { json q; q["count"]["inst"] = "MEATBUG"; q["count"]["p"] = CTVec(CT.haveWp ? CT.wpPos : player->GetPositionWorld()); return q; }
    static json QMol() { json q; q["npc"] = CT.molName; return q; }
    static json QMolExp() { json q; q["npc"] = CT.molName; q["exp"] = 1; return q; }
    static json QBug() { json q; q["npc"] = CT.bugName; return q; }
    static json QChest() { json q; q["chest"]["p"] = CTVec(CT.chestPos); q["chest"]["inst"] = CTApple(); return q; }
    static json QItems() { json q; q["items"]["inst"] = CTApple(); q["items"]["p"] = CTVec(player->GetPositionWorld()); return q; }
    static json QPick() { json q; q["item"]["inst"] = CT.pickInst; q["item"]["p"] = CTVec(CT.pickPos); return q; }
    static json QFar() { json q; q["far"]["p"] = CTVec(player->GetPositionWorld()); return q; }
    static json QNear() { json q; q["near"]["p"] = CTVec(player->GetPositionWorld()); q["near"]["r"] = CT_NEAR_R + 500.f; return q; }
    static json QLog() { json q; q["logmark"] = CT.marker; return q; }
    static json QTrig() { json q; q["trig"] = 1; return q; }
    static json QLead() { json q; q["lead"] = 1; return q; }
    static json QRune() { json q; q["rune"] = 1; return q; }
    static json QMark() { json q; q["mark"] = 1; return q; }
    static json QAudit() {
        json q;
#if COOP_READY_Audit   // u12: Modul je Engine
        json mine = CoopAuditSummaryJson(false);
        q["audit"]["h"] = mine.value("h", std::string());
        q["audit"]["rh"] = mine.value("rh", std::string());
#else
        q["audit"]["h"] = std::string();
#endif
        return q;
    }
    static json QDigVars() { return CTDigQuery("vars"); }
    static json QDigLog() { return CTDigQuery("log"); }
    static json QDigNpcs() { return CTDigQuery("npcs"); }
    static json QDigDead() { return CTDigQuery("dead"); }
    static json QDigGuilds() { return CTDigQuery("guilds"); }
    static json QDigChests() { return CTDigQuery("chests"); }
    static json QDigLoose() { return CTDigQuery("loose", true); }

    static bool ANone(std::string&) { return true; }

    // =====================================================================================================================
    // Bereich 1: Verbindung und Grundstand
    // =====================================================================================================================

    // Rundlauf Frage -> Antwort (beide Netzfaeden, Weiterleitung beim Host, Hauptfaden des Partners)
    static bool JRtt(const json&, const json&, std::string& d) {
        d = CTFmt("Antwort nach %d ms (soll < 1000)", CT.rttMs);
        return CT.rttMs >= 0 && CT.rttMs < 1000;
    }

    static bool JVer(const json&, const json& post, std::string& d) {
        const std::string v = CTStr(post, "base", "ver");
        d = "Partner " + v + ", hier " + COOP_FASSUNG;
        return v == COOP_FASSUNG;
    }

    static bool JWorld(const json&, const json& post, std::string& d) {
        const std::string w = CTStr(post, "base", "world"), mine = ogame->GetGameWorld()->GetWorldFilename().ToChar();
        d = "Partner " + w + ", hier " + mine;
        return w == mine;
    }

    // u29 (P65): getrennte Welten - der Partner ist in derselben Welt und kennt meine aus den Grundpaketen ("wk")
    static bool JWorldKey(const json&, const json& post, std::string& d) {
        const int mine = CoopWorldKey();
        const int theirs = CTInt(post, "base", "wk"), ofMe = CTInt(post, "base", "wkme");
        d = CTFmt("Partner %08X, hier %08X; meine Welt beim Partner %08X", (unsigned)theirs, (unsigned)mine, (unsigned)ofMe);
        return mine != 0 && theirs == mine && ofMe == mine;
    }

    // u30 (P65): Partner in einer anderen Welt - beide wissen, wo der andere ist
    static bool JSepWorlds(const json&, const json& post, std::string& d) {
        const int mine = CoopWorldKey(), theirs = CTInt(post, "base", "wk"), ofMe = CTInt(post, "base", "wkme");
        const int ofThem = CoopPeerWorldKey(CT.partner);
        d = CTFmt("Partner in %s, hier %s; Partner kennt meine Welt: %s, ich seine: %s", CTStr(post, "base", "world").c_str(),
            CoopWorldName().c_str(), ofMe == mine ? "ja" : "nein", ofThem == theirs ? "ja" : "nein");
        return mine != 0 && theirs > 0 && theirs != mine && ofMe == mine && ofThem == theirs;
    }

    // u30 (P65): keine Figur in der Welt des anderen - weder meine beim Partner noch seine hier
    static bool JSepFigs(const json&, const json& post, std::string& d) {
        int theirsOfMe = -1;
        if (post.contains("base") && post["base"].contains("figs"))
            for (auto& f : post["base"]["figs"])
                if (f.value("id", std::string()) == DialogMe())
                    theirsOfMe = f.value("act", 0);
        const bool mineOfThem = PlayerNameToNpc.count(string(CT.partner.c_str())) != 0;
        d = CTFmt("meine Figur beim Partner: %s; Figur von %s hier: %s", theirsOfMe == -1 ? "keine" : "DA",
            CT.partner.c_str(), mineOfThem ? "DA" : "keine");
        return theirsOfMe == -1 && !mineOfThem;
    }

    // u30 (P65): keine NPCs aus der anderen Welt - niemand fuehrt hier oder beim Partner NPCs des anderen
    static bool JSepNpcs(const json&, const json& post, std::string& d) {
        const int theirs = CTInt(post, "base", "led"), mine = CTLedHere();
        d = CTFmt("vom anderen PC gefuehrte NPCs: beim Partner %d, hier %d (soll 0)", theirs, mine);
        return theirs == 0 && mine == 0;
    }

    // u30 (P66): der Client fuehrt die NPCs seiner Welt selbst genau dann, wenn der Host in einer anderen Welt ist
    static bool JWhoLeads(const json&, const json& post, std::string& d) {
        const int alone = ServerThread ? CTInt(post, "base", "alone") : (CoopClientAlone() ? 1 : 0);
        const int want = CT.otherWorld ? 1 : 0;
        d = CTFmt("Client fuehrt seine NPCs %s (soll %s)", alone == 1 ? "selbst" : alone == 0 ? "nicht - der Host fuehrt sie" : "?",
            want ? "selbst: Host in anderer Welt" : "Host: gleiche Welt");
        return alone == want;
    }

    static int CTMinDiff(int a, int b) {
        int diff = abs(a - b);
        return diff > 12 * 60 ? 24 * 60 - diff : diff;
    }

    static bool JDayTime(const json&, const json& post, std::string& d) {
        int day = 0, h = 0, m = 0;
        ogame->GetTime(day, h, m);
        const int pd = CTInt(post, "base", "day"), pm = CTInt(post, "base", "min");
        d = CTFmt("Partner Tag %d %02d:%02d, hier Tag %d %02d:%02d", pd, pm / 60, pm % 60, day, h, m);
        const bool same = pd == day && pm >= 0 && CTMinDiff(pm, h * 60 + m) <= 5;
        // u74 (Dauerlauf u73: beim Host zweimal FEHLER, der Client 30 bzw. 69 min vor - beide Male war er im Gespraech): im Gespraech
        // uebernimmt der Client die Uhr des Hosts nicht (PacketProcessor.cpp), danach mit dem naechsten Paket (alle 10 s). Bei Spielzeit
        // x4 laufen die Uhren dazwischen auseinander: die Engine rechnet je Bild hoechstens 100 ms Spielzeit, der Host (weniger Bilder
        // je Sekunde) kam so nur auf x2,5. Dann nicht bewertbar
        const bool clientTalks = ServerThread ? CTInt(post, "base", "talk") == 1 : IsPlayerTalkingWithAnybody();
        if (!same && clientTalks)
            d = "-- Client im Gespraech, uebernimmt die Uhr des Hosts erst danach: " + d;
        return same;
    }

    // Uhrzeit +30 min (nur Host; GT schickt sonst alle 10 s - bis u68 alle 60 s)
    static bool ATime(std::string&) {
        int day = 0, h = 0, m = 0;
        ogame->GetTime(day, h, m);
        const int t = (h * 60 + m + 30) % (24 * 60);
        ogame->GetWorldTimer()->SetTime(t / 60, t % 60);
        if (Myself)
            Myself->SyncTime();
        return true;
    }

    static bool JTime(const json&, const json& post, std::string& d) {
        int day = 0, h = 0, m = 0;
        ogame->GetTime(day, h, m);
        const int pm = CTInt(post, "base", "min");
        d = CTFmt("Partner %02d:%02d, hier %02d:%02d", pm / 60, pm % 60, h, m);
        return pm >= 0 && CTMinDiff(pm, h * 60 + m) <= 5;
    }

    static bool CTRulesSame(const json& post, const char* const* keys, int n, std::string& d) {
        const json mine = CTRulesJson();
        if (!post.contains("base") || !post["base"].contains("rules")) {
            d = "Partner meldet keine Regeln";
            return false;
        }
        const json& p = post["base"]["rules"];
        std::string diff;
        for (int i = 0; i < n; i++)
            if (p.value(keys[i], -1) != mine.value(keys[i], -2))
                diff += CTFmt("%s%s hier %d/Partner %d", diff.empty() ? "" : ", ", keys[i], mine.value(keys[i], -2), p.value(keys[i], -1));
        d = diff.empty() ? std::string("alle gleich") : "abweichend: " + diff;
        return diff.empty();
    }

    // u12: Schritte gesperrter Module laufen nicht (CTModuleOff) - die Urteile werden trotzdem fuer jede Engine gebaut
#define CT_MODULE_OFF_JUDGE(mod) { d = "-- Modul " mod " in dieser Engine aus"; return false; }

    static bool JRules(const json&, const json& post, std::string& d) {
#if COOP_READY_Difficulty
        static const char* keys[] = { "sq", "sl", "sw", "ss", "sa", "st" };
        const bool same = CTRulesSame(post, keys, 6, d);
        d = SyncDescribe(DiffEff()) + " - " + d;
        return same;
#else
        CT_MODULE_OFF_JUDGE("Schwierigkeit/Regeln")
#endif
    }

    static bool JDiff(const json&, const json& post, std::string& d) {
#if COOP_READY_Difficulty
        static const char* keys[] = { "p", "dp", "de", "hp", "xa", "xp", "xr" };
        const bool same = CTRulesSame(post, keys, 7, d);
        d = DiffDescribe(DiffEff()) + " - " + d;
        return same;
#else
        CT_MODULE_OFF_JUDGE("Schwierigkeit/Regeln")
#endif
    }

    // Engine-Aenderungen wie beim Partner - Fingerabdruck der Selbstpruefung (CoopAudit.cpp) je Verursacher-Modul
    static bool JAudit(const json&, const json& post, std::string& d) {
#if !COOP_READY_Audit
        CT_MODULE_OFF_JUDGE("Selbstpruefung")
#else
        if (!post.contains("audit") || post["audit"].value("ok", 0) != 1) {
            d = "Partner hat keine Selbstpruefung (" + (post.contains("audit") ? post["audit"].value("err", std::string("?")) : std::string("keine Antwort")) + ")";
            return false;
        }
        const json& p = post["audit"];
        json m = CoopAuditSummaryJson(true);
        const bool same = m.value("h", std::string()) == p.value("h", std::string()) && m.value("rh", std::string()) == p.value("rh", std::string());
        d = CTFmt("hier %d Stellen .text / %d .rdata, GT-Haken %d/%d - Partner %d / %d, %d/%d; Fingerabdruck %s",
            m.value("n", -1), m.value("rn", -1), m.value("ha", -1), m.value("ht", -1), p.value("n", -1), p.value("rn", -1),
            p.value("ha", -1), p.value("ht", -1), same ? "gleich" : "VERSCHIEDEN");
        std::string diff;
        const json none = json::object();
        for (const char* sec : { "o", "ro" }) {
            const json& mo = m.contains(sec) ? m[sec] : none;
            const json& po = p.contains(sec) ? p[sec] : none;
            std::set<std::string> keys;
            for (auto it = mo.begin(); it != mo.end(); ++it) keys.insert(it.key());
            for (auto it = po.begin(); it != po.end(); ++it) keys.insert(it.key());
            for (auto& k : keys) {
                const int a = mo.value(k, 0), b = po.value(k, 0);
                if (a != b)
                    diff += CTFmt("%s%s%s hier %d/Partner %d", diff.empty() ? "" : ", ", k.c_str(), strcmp(sec, "ro") == 0 ? " (.rdata)" : "", a, b);
            }
        }
        if (!diff.empty())
            d += "; abweichend: " + diff;
        if (!same) {
            CoopAuditDumpToLog("Testlauf: weicht vom Partner ab");
            d += " - beide Listen im Log (HL-Zeilen, tools\\auditnames.py)";
        }
        return same && m.value("ha", -1) == m.value("ht", -2) && p.value("ha", -1) == p.value("ht", -2);
#endif
    }

    // gemeinsamer Durchlauf (Spielstand-Reihe, H31) - beide spielen dieselbe
    static bool JRun(const json&, const json& post, std::string& d) {
        const std::string p = CTStr(post, "base", "run"), m = CoopSaveSyncCurrentRun();
        d = "Partner \"" + p + "\", hier \"" + m + "\"";
        if (p.empty() && m.empty()) {
            d = "beide ohne Durchlauf (freies Spiel)";
            return true;
        }
        return p == m;
    }

    // gemeinsame Sicherung waere jetzt moeglich (SAVE_CHECK, H4) - ein beschaeftigter Held ist kein Abgleichfehler
    static bool JSaveReady(const json&, const json& post, std::string& d) {
#if !COOP_READY_Save
        CT_MODULE_OFF_JUDGE("Speichern/Laden")
#else
        std::string why;
        const bool mine = CoopCanSaveNow(why);
        const int p = CTInt(post, "base", "save");
        if (!mine || p != 1) {
            d = "-- nicht bewertbar: " + std::string(mine ? "" : "hier " + why) + (mine || p == 1 ? "" : ", ") +
                (p == 1 ? "" : "beim Partner " + CTStr(post, "base", "sw"));
            return false;
        }
        d = "beide koennten jetzt gemeinsam speichern";
        return true;
#endif
    }

    static bool JQueue(const json&, const json& post, std::string& d) {
        const int mine = ReadyToSendJsons.size(), p = CTInt(post, "base", "q");
        d = CTFmt("Pakete vor dem Senden: hier %d, beim Partner %d (soll < 300)", mine, p);
        return p >= 0 && p < 300 && mine < 300;
    }

    static bool JCrash(const json&, const json& post, std::string& d) {
        const bool veh = CoopVehHandle != NULL, top = CTCrashFront();
        const int pv = CTInt(post, "base", "veh"), pt = CTInt(post, "base", "top");
        d = CTFmt("erste Stufe (VEH) hier %s/Partner %s, Filter vorn hier %s/Partner %s", veh ? "an" : "AUS", pv == 1 ? "an" : "AUS",
            top ? "ja" : "NEIN", pt == 1 ? "ja" : "NEIN");
        return veh && top && pv == 1 && pt == 1;
    }

    // Log-Versand: eine Marke im eigenen Log kommt in der Log-Datei beim Partner an (G4/G4b)
    static bool ALogMark(std::string&) {
        CT.marker = "Coop-Test-Marke " + CT.tag;
        CTLog("%s", CT.marker.c_str());
        return true;
    }

    static bool JLogMark(const json&, const json& post, std::string& d) {
        d = CTInt(post, "logmark") == 1 ? "Marke in der Log-Datei des Partners" : "Marke beim Partner nicht gefunden";
        return CTInt(post, "logmark") == 1;
    }

    // =====================================================================================================================
    // Bereich 2: eigene Figur beim Partner
    // =====================================================================================================================

    // beide Figuren stehen: meine beim Partner, seine hier - jeweils in der Welt
    static bool JFigs(const json&, const json& post, std::string& d) {
        int theirsOfMe = -1;
        if (post.contains("base") && post["base"].contains("figs"))
            for (auto& f : post["base"]["figs"])
                if (f.value("id", std::string()) == DialogMe())
                    theirsOfMe = f.value("act", 0);
        auto it = PlayerNameToNpc.find(string(CT.partner.c_str()));
        const int mineOfThem = it == PlayerNameToNpc.end() ? -1 : (CTInWorld(it->second) ? 1 : 0);
        d = CTFmt("meine Figur beim Partner: %s; Figur von %s hier: %s", theirsOfMe == 1 ? "in der Welt" : theirsOfMe == 0 ? "da, nicht in der Welt" : "fehlt",
            CT.partner.c_str(), mineOfThem == 1 ? "in der Welt" : mineOfThem == 0 ? "da, nicht in der Welt" : "fehlt");
        return theirsOfMe == 1 && mineOfThem == 1;
    }

    static bool JFigPos(const json&, const json& post, std::string& d) {
        if (CTInt(post, "fig", "found") != 1) {
            d = "Partner findet die Figur nicht";
            return false;
        }
        const float dist = CoopDist(CTVecOf(post["fig"]["p"]), player->GetPositionWorld());
        d = CTFmt("Abstand %.0f cm (soll < 300)", dist);
        return dist < 300.f;
    }

    // Blickrichtung (SYNC_HEADING): +90 Grad und zurueck
    static bool ATurn(std::string&) {
        player->RotateWorldY(90.f);
        CT.headTurned = true;
        CT.headWant = GetHeading(player);
        return true;
    }

    static bool PTurnBack(std::string& why) {
        if (!CT.headTurned) {
            why = "keine Drehung aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool ATurnBack(std::string&) {
        player->RotateWorldY(-90.f);
        CT.headTurned = false;
        CT.headWant = GetHeading(player);
        return true;
    }

    static bool JHead(const json&, const json& post, std::string& d) {
        const float p = CTFloat(post, "fig", "h");
        float mine = GetHeading(player);
        // v108 (Test v107: Held drehte sich waehrend der Wartezeit weiter): auch gegen die eigene Richtung zur Zeit der Frage
        if (CT.myHeadAsk >= 0.f && CTAngle(p, CT.myHeadAsk) < CTAngle(p, mine))
            mine = CT.myHeadAsk;
        d = CTFmt("Figur beim Partner %.0f Grad, hier %.0f Grad (Abweichung %.0f, soll < 15)", p, mine, CTAngle(p, mine));
        return p > -1e8f && CTAngle(p, mine) < 15.f;
    }

    // Name der Figur (INIT_NPC "nickname", H8)
    static bool JName(const json&, const json& post, std::string& d) {
        const std::string p = CTStr(post, "fig", "name");
        d = "Figur beim Partner \"" + p + "\", hier \"" + std::string(MyNickname.ToChar()) + "\"";
        return !p.empty() && p == MyNickname.ToChar();
    }

    // Name aendern wie im COOP-Menue: Gothic.ini [GOTHICCOOP] coopName - CoopLookPulse uebernimmt ihn, INIT_NPC geht neu hinaus
    static bool PName(std::string& why) {
        if (!zoptions) {
            why = "keine Optionen";
            return false;
        }
        CT.nameOld = IniStr("coopName", std::string(MyNickname.ToChar()));
        CT.nameNew = std::string(MyNickname.ToChar()) + "-T";
        return true;
    }

    static bool AName(std::string&) {
        IniSetStr("coopName", CT.nameNew);
        CT.nameChanged = true;
        return true;
    }

    static bool JNameNew(const json&, const json& post, std::string& d) {
        const std::string p = CTStr(post, "fig", "name");
        d = "Figur beim Partner \"" + p + "\" (soll \"" + CT.nameNew + "\")";
        return p == CT.nameNew;
    }

    static bool PNameBack(std::string& why) {
        if (!CT.nameChanged) {
            why = "kein geaenderter Name aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool ANameBack(std::string&) {
        IniSetStr("coopName", CT.nameOld);
        CT.nameChanged = false;
        return true;
    }

    static bool JNameBack(const json&, const json& post, std::string& d) {
        const std::string p = CTStr(post, "fig", "name");
        d = "Figur beim Partner \"" + p + "\", hier \"" + std::string(MyNickname.ToChar()) + "\"";
        return !p.empty() && p == MyNickname.ToChar() && p != CT.nameNew;
    }

    // Aussehen (INIT_NPC headMesh/headVarNr/bodyMesh/bodyTextVarNr/skin); mit Ruestung bestimmt sie den Koerper
    static bool JLook(const json&, const json& post, std::string& d) {
        if (CTInt(post, "fig", "found") != 1) {
            d = "Partner findet die Figur nicht";
            return false;
        }
        const std::string hv = CTStr(post, "fig", "hv"), bv = CTStr(post, "fig", "bv");
        const int ht = CTInt(post, "fig", "ht"), bt = CTInt(post, "fig", "bt"), bc = CTInt(post, "fig", "bc");
        const int skin = MySkin >= 0 ? MySkin : DefaultBodyTexColorNr;
        const bool body = !player->GetEquippedArmor();
        d = CTFmt("Figur beim Partner: Kopf %s/%d, Koerper %s/%d/%d - hier gewaehlt: %s/%d, %s/%d/%d%s", hv.c_str(), ht, bv.c_str(), bt, bc,
            CTUpper(MyHeadMesh).c_str(), MyHeadVarNr, CTUpper(MyBodyMesh).c_str(), MyBodyTextVarNr, skin, body ? "" : " (Ruestung: nur Kopf verglichen)");
        const bool head = hv == CTUpper(MyHeadMesh) && ht == MyHeadVarNr;
        return head && (!body || (bv == CTUpper(MyBodyMesh) && bt == MyBodyTextVarNr && bc == skin));
    }

    // Gesicht aendern wie im COOP-Menue (coopFace) - ein Wert, den LookResolve nicht auf das jetzige Gesicht zurueckfuehrt
    static bool PFace(std::string& why) {
        if (!zoptions) {
            why = "keine Optionen";
            return false;
        }
        CT.faceOld = MyHeadVarNr;
        CT.faceIniOld = IniInt("coopFace", MyHeadVarNr);
        // dieselben Rohwerte wie CoopSettingsRefresh (Gothic.ini, begrenzt) - sonst loest LookResolve anders auf
        int bodyMesh = IniInt("coopBodyMesh", 0), body0 = IniInt("coopBody", MyBodyTextVarNr), skin0 = IniInt("coopSkin", DefaultBodyTexColorNr);
        if (bodyMesh < 0 || bodyMesh >= CoopBodyMeshCount)
            bodyMesh = 0;
        if (body0 < 0 || body0 > LookBodyMax())   // u5: Bereiche je Profil (CoopLook.cpp)
            body0 = 0;
        if (skin0 < 0 || skin0 > 3)
            skin0 = 0;
        for (int k = 1; k <= 60; k++) {
            int face = (MyHeadVarNr + k) % (LookFaceMax() + 1), body = body0, skin = skin0;
            const int raw = face;
            LookResolve(bodyMesh, face, body, skin);
            if (face != MyHeadVarNr) {
                CT.faceNew = raw;
                CT.faceWant = face;
                return true;
            }
        }
        why = "kein anderes Gesicht mit diesem Hautton";
        return false;
    }

    static bool AFace(std::string&) {
        IniSetInt("coopFace", CT.faceNew);
        CT.faceChanged = true;
        return true;
    }

    static bool JFace(const json&, const json& post, std::string& d) {
        const int ht = CTInt(post, "fig", "ht");
        d = CTFmt("Gesicht der Figur beim Partner %d, hier jetzt %d (soll %d, vorher %d)", ht, MyHeadVarNr, CT.faceWant, CT.faceOld);
        return ht == MyHeadVarNr && MyHeadVarNr == CT.faceWant;
    }

    static bool PFaceBack(std::string& why) {
        if (!CT.faceChanged) {
            why = "kein geaendertes Gesicht aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool AFaceBack(std::string&) {
        IniSetInt("coopFace", CT.faceIniOld);
        CT.faceChanged = false;
        return true;
    }

    static bool JFaceBack(const json&, const json& post, std::string& d) {
        const int ht = CTInt(post, "fig", "ht");
        d = CTFmt("Gesicht der Figur beim Partner %d, hier %d (soll %d)", ht, MyHeadVarNr, CT.faceOld);
        return ht == MyHeadVarNr && MyHeadVarNr == CT.faceOld;
    }

    // Lebenspunkte und Maximum (SYNC_HP)
    static bool AHp(std::string& why) {
        CT.hpOld = player->GetAttribute(NPC_ATR_HITPOINTS);
        if (CT.hpOld <= 2) {
            why = "zu wenig Lebenspunkte";
            return false;
        }
        player->SetAttribute(NPC_ATR_HITPOINTS, CT.hpOld - 1);
        return true;
    }

    static bool JHp(const json&, const json& post, std::string& d) {
        const int hp = CTInt(post, "fig", "hp");
        d = CTFmt("Figur beim Partner %d HP (soll %d)", hp, CT.hpOld - 1);
        return hp == CT.hpOld - 1;
    }

    static void CHp() { player->SetAttribute(NPC_ATR_HITPOINTS, CT.hpOld); }

    static bool AHpMax(std::string&) {
        CT.hpMaxOld = player->GetAttribute(NPC_ATR_HITPOINTSMAX);
        player->SetAttribute(NPC_ATR_HITPOINTSMAX, CT.hpMaxOld + 5);
        return true;
    }

    static bool JHpMax(const json&, const json& post, std::string& d) {
        const int hm = CTInt(post, "fig", "hm");
        d = CTFmt("Figur beim Partner Maximum %d (soll %d)", hm, CT.hpMaxOld + 5);
        return hm == CT.hpMaxOld + 5;
    }

    static void CHpMax() {
        player->SetAttribute(NPC_ATR_HITPOINTSMAX, CT.hpMaxOld);
        if (player->GetAttribute(NPC_ATR_HITPOINTS) > CT.hpMaxOld)
            player->SetAttribute(NPC_ATR_HITPOINTS, CT.hpMaxOld);
    }

    // Waffenmodus (SYNC_WEAPON_MODE)
    static bool CTJudgeWm(const json& post, int want, std::string& d) {
        const int wm = CTInt(post, "fig", "wm");
        d = CTFmt("Figur beim Partner Modus %d (soll %d)", wm, want);
        return wm == want;
    }

    static bool AFist(std::string&) { player->SetWeaponMode2(NPC_WEAPON_FIST); return true; }
    static bool ANoWeapon(std::string&) { player->SetWeaponMode2(NPC_WEAPON_NONE); return true; }
    static bool JFist(const json&, const json& post, std::string& d) { return CTJudgeWm(post, NPC_WEAPON_FIST, d); }
    static bool JNoWeapon(const json&, const json& post, std::string& d) { return CTJudgeWm(post, NPC_WEAPON_NONE, d); }

    // Waffen und Ruestung anlegen / ablegen (wie die Wiederherstellung des eigenen Helden: erzeugen, PutInInv, EquipItem;
    // ablegen: UnequipItem, RemoveFromInv, RemoveVob). Traegt der Held schon etwas, prueft der Schritt nur, ob die Figur es zeigt.
    static oCItem* CTEquipNew(const char* inst, std::string& why) {
        oCItem* it = CreateCoopItem(parser->GetIndex(zSTRING(inst)));
        oCItem* inv = it ? player->PutInInv(it) : NULL;
        if (!inv) {
            why = std::string(inst) + " nicht erzeugt";
            return NULL;
        }
        player->EquipItem(inv);
        return inv;
    }

    static void CTDropOwn(oCItem*& it) {
        if (!it)
            return;
        player->UnequipItem(it);
        if (oCItem* r = player->RemoveFromInv(it, 1))
            ogame->GetGameWorld()->RemoveVob(r);
        it = NULL;
    }

    static int CTMeleeMode(oCItem* w) {
        return w && (w->flags & ((1 << 16) | (1 << 17))) ? NPC_WEAPON_2HS : NPC_WEAPON_1HS;   // ITEM_2HD_SWD / ITEM_2HD_AXE
    }

    static bool AMelee(std::string& why) {
        if (oCItem* own = player->GetEquippedMeleeWeapon()) {
            CT.meleeMode = CTMeleeMode(own);
            return true;   // schon angelegt - Figur muss sie zeigen
        }
        const char* inst = CTInstOf({ "ALTESSCHWERT", "ITMW_1H_VLK_SWORD", "ITMW_1H_BAU_MACE" });   // u12: G2 Schwert/Pruegel
        if (parser->GetIndex(zSTRING(inst)) < 0) {
            why = "ALTESSCHWERT/ITMW_1H_VLK_SWORD fehlt";
            return false;
        }
        CT.weapon = CTEquipNew(inst, why);
        if (!CT.weapon)
            return false;
        if (player->GetEquippedMeleeWeapon() != CT.weapon) {   // z. B. Staerke-Anforderung nicht erfuellt
            CTDropOwn(CT.weapon);
            why = "Held kann das Schwert nicht anlegen (Anforderungen)";
            return false;
        }
        CT.meleeMode = CTMeleeMode(CT.weapon);
        return true;
    }

    static bool JMelee(const json&, const json& post, std::string& d) {
        const std::string m = CTStr(post, "fig", "melee"), mine = CTItemName(player->GetEquippedMeleeWeapon());
        d = "Figur beim Partner: " + (m.empty() ? std::string("keine Waffe") : m) + ", hier " + (mine.empty() ? std::string("keine") : mine) +
            (CT.weapon ? "" : " (eigene Waffe)");
        return !mine.empty() && m == mine;
    }

    static bool PMeleeDraw(std::string& why) {
        if (!player->GetEquippedMeleeWeapon()) {
            why = "keine Nahkampfwaffe angelegt";
            return false;
        }
        return true;
    }

    static bool AMeleeDraw(std::string&) { player->SetWeaponMode2(CT.meleeMode); return true; }
    static bool JMeleeDraw(const json&, const json& post, std::string& d) { return CTJudgeWm(post, CT.meleeMode, d); }

    static bool AMeleeOff(std::string&) {
        CTDropOwn(CT.weapon);   // eine eigene Waffe bleibt angelegt
        return true;
    }

    static bool JMeleeOff(const json&, const json& post, std::string& d) {
        const std::string m = CTStr(post, "fig", "melee"), mine = CTItemName(player->GetEquippedMeleeWeapon());
        d = "Figur beim Partner: " + (m.empty() ? std::string("keine Waffe") : m) + ", hier " + (mine.empty() ? std::string("keine") : mine + " (eigene)");
        return m == mine;
    }

    static bool ARanged(std::string& why) {
        if (oCItem* own = player->GetEquippedRangedWeapon()) {
            CT.rangedMode = (own->flags & (1 << 20)) ? NPC_WEAPON_CBOW : NPC_WEAPON_BOW;   // ITEM_CROSSBOW
            return true;
        }
        static const char* cand[] = { "ITRW_BOW_SMALL_01", "ITRW_BOW_SMALL_02", "ITRW_BOW_SMALL_03", "ITRW_BOW_LONG_01", "ITRW_CROSSBOW_01",
            "ITRW_BOW_L_01", "ITRW_BOW_L_02", "ITRW_BOW_M_01", "ITRW_CROSSBOW_L_01" };   // u12: G2 NotR dahinter
        for (const char* inst : cand) {
            if (parser->GetIndex(zSTRING(inst)) < 0)
                continue;
            std::string w;
            CT.ranged = CTEquipNew(inst, w);
            if (!CT.ranged)
                continue;
            if (player->GetEquippedRangedWeapon() == CT.ranged) {
                CT.rangedMode = (CT.ranged->flags & (1 << 20)) ? NPC_WEAPON_CBOW : NPC_WEAPON_BOW;
                return true;
            }
            CTDropOwn(CT.ranged);   // Anforderungen nicht erfuellt - naechster
        }
        why = "kein Bogen/keine Armbrust anlegbar (Instanzen oder Anforderungen)";
        return false;
    }

    static bool JRanged(const json&, const json& post, std::string& d) {
        const std::string m = CTStr(post, "fig", "ranged"), mine = CTItemName(player->GetEquippedRangedWeapon());
        d = "Figur beim Partner: " + (m.empty() ? std::string("keine Fernkampfwaffe") : m) + ", hier " + (mine.empty() ? std::string("keine") : mine) +
            (CT.ranged ? "" : " (eigene Waffe)");
        return !mine.empty() && m == mine;
    }

    static bool PRangedDraw(std::string& why) {
        if (!player->GetEquippedRangedWeapon()) {
            why = "keine Fernkampfwaffe angelegt";
            return false;
        }
        return true;
    }

    static void CTDropAmmo() {
        if (!CT.ammo)
            return;
        if (oCItem* r = player->RemoveFromInv(CT.ammo, CoopItemAmount(CT.ammo)))
            ogame->GetGameWorld()->RemoveVob(r);
        CT.ammo = NULL;
    }

    // v108 (Test v107: Modus 0 statt 5 beim Partner): wie beim echten Spielen mit Munition ziehen; der eigene Modus muss stimmen
    static bool ARangedDraw(std::string& why) {
        oCItem* r = player->GetEquippedRangedWeapon();
        if (r && r->munition > 0 && !player->IsInInv(r->munition, 1))
            if (oCItem* m = CreateCoopItem(r->munition)) {
                m->amount = 20;
                WorldSyncApplying++;   // nicht als Erhaltenes fuers Pluendern zaehlen
                CT.ammo = player->PutInInv(m);
                WorldSyncApplying--;
            }
        player->SetWeaponMode2(CT.rangedMode);
        CT.ownMode = player->GetWeaponMode();
        if (CT.ownMode != CT.rangedMode) {
            why = CTFmt("eigener Held zieht die Fernkampfwaffe nicht (Modus %d, Munition %s)", CT.ownMode,
                r && r->munition > 0 && player->IsInInv(r->munition, 1) ? "da" : "fehlt");
            player->SetWeaponMode2(NPC_WEAPON_NONE);
            return false;
        }
        return true;
    }

    static bool JRangedDraw(const json&, const json& post, std::string& d) {
        const bool ok = CTJudgeWm(post, CT.rangedMode, d);
        d += CTFmt("; hier Modus %d, Munition der Figur beim Partner %s", player->GetWeaponMode(), CTInt(post, "fig", "ammo") == 1 ? "da" : "fehlt");
        return ok;
    }

    static bool ARangedOff(std::string&) {
        CTDropOwn(CT.ranged);
        CTDropAmmo();
        return true;
    }

    static bool JRangedOff(const json&, const json& post, std::string& d) {
        const std::string m = CTStr(post, "fig", "ranged"), mine = CTItemName(player->GetEquippedRangedWeapon());
        d = "Figur beim Partner: " + (m.empty() ? std::string("keine Fernkampfwaffe") : m) + ", hier " + (mine.empty() ? std::string("keine") : mine + " (eigene)");
        return m == mine;
    }

    static bool AArmor(std::string& why) {
        if (player->GetEquippedArmor())
            return true;
        const char* inst = CTInstOf({ "VLK_ARMOR_L", "ITAR_VLK_L", "ITAR_BAU_L" });   // u12: G2 Buergerkleidung
        if (parser->GetIndex(zSTRING(inst)) < 0) {
            why = "VLK_ARMOR_L/ITAR_VLK_L fehlt";
            return false;
        }
        CT.armor = CTEquipNew(inst, why);
        if (!CT.armor)
            return false;
        if (player->GetEquippedArmor() != CT.armor) {
            CTDropOwn(CT.armor);
            why = "Held kann die Ruestung nicht anlegen (Anforderungen)";
            return false;
        }
        return true;
    }

    static bool JArmor(const json&, const json& post, std::string& d) {
        const std::string a = CTStr(post, "fig", "armor"), mine = CTItemName(player->GetEquippedArmor());
        d = "Figur beim Partner: " + (a.empty() ? std::string("keine Ruestung") : a) + ", hier " + (mine.empty() ? std::string("keine") : mine) +
            (CT.armor ? "" : " (eigene)");
        return !mine.empty() && a == mine;
    }

    static bool AArmorOff(std::string&) {
        CTDropOwn(CT.armor);
        return true;
    }

    static bool JArmorOff(const json&, const json& post, std::string& d) {
        const std::string a = CTStr(post, "fig", "armor"), mine = CTItemName(player->GetEquippedArmor());
        d = "Figur beim Partner: " + (a.empty() ? std::string("keine Ruestung") : a) + ", hier " + (mine.empty() ? std::string("keine") : mine + " (eigene)");
        return a == mine;
    }

    // Talent Einhand (SYNC_TALENTS) und Schutz gegen Klingen (SYNC_PROTECTIONS) - danach zurueck
    static bool ATalent(std::string&) {
        CT.talOld = player->GetTalentSkill(oCNpcTalent::NPC_TAL_1H);
        CT.talNew = CT.talOld < 2 ? CT.talOld + 1 : CT.talOld - 1;
        player->SetTalentSkill(oCNpcTalent::NPC_TAL_1H, CT.talNew);
        return true;
    }

    static bool JTalent(const json&, const json& post, std::string& d) {
        int t = -999;
        try { t = post.at("fig").at("tal").at(0).get<int>(); } catch (...) {}
        d = CTFmt("Einhand der Figur beim Partner %d (soll %d, vorher %d)", t, CT.talNew, CT.talOld);
        return t == CT.talNew;
    }

    static void CTalent() {
        if (CT.talOld >= 0)
            player->SetTalentSkill(oCNpcTalent::NPC_TAL_1H, CT.talOld);
        CT.talOld = -1;
    }

    static bool AProt(std::string&) {
        CT.protOld = player->GetProtectionByIndex(oEDamageIndex_Edge);
        player->SetProtectionByIndex(oEDamageIndex_Edge, CT.protOld + 5);
        return true;
    }

    static bool JProt(const json&, const json& post, std::string& d) {
        int p = -999;
        try { p = post.at("fig").at("prot").at((int)oEDamageIndex_Edge).get<int>(); } catch (...) {}
        d = CTFmt("Schutz Klinge der Figur beim Partner %d (soll %d)", p, CT.protOld + 5);
        return p == CT.protOld + 5;
    }

    static void CProt() { player->SetProtectionByIndex(oEDamageIndex_Edge, CT.protOld); }

    // Overlay (SYNC_OVERLAYS, Name statt Nummer, B4) an und ab
    static bool POverlay(std::string& why) {
        const json have = CTOverlays(player);
        for (const char* o : { "HUMANS_TIRED.MDS", "HUMANS_RELAXED.MDS", "HUMANS_ARROGANCE.MDS" })
            if (!CTHas(have, o)) {
                CT.overlay = o;
                return true;
            }
        why = "alle Test-Overlays schon aktiv";
        return false;
    }

    static bool AOverlay(std::string& why) {
        if (!player->ApplyOverlay(zSTRING(CT.overlay.c_str()))) {
            why = CT.overlay + " nicht ladbar";
            return false;
        }
        CT.overlayOn = true;
        return true;
    }

    static bool JOverlay(const json&, const json& post, std::string& d) {
        const bool has = post.contains("fig") && CTHas(post["fig"].value("ovl", json::array()), CT.overlay);
        d = "Figur beim Partner " + std::string(has ? "hat " : "OHNE ") + CT.overlay;
        return has;
    }

    static bool POverlayOff(std::string& why) {
        if (!CT.overlayOn) {
            why = "kein Overlay aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool AOverlayOff(std::string&) {
        player->RemoveOverlay(zSTRING(CT.overlay.c_str()));
        CT.overlayOn = false;
        return true;
    }

    static bool JOverlayOff(const json&, const json& post, std::string& d) {
        const bool has = post.contains("fig") && CTHas(post["fig"].value("ovl", json::array()), CT.overlay);
        d = "Figur beim Partner " + std::string(has ? "hat noch " : "ohne ") + CT.overlay;
        return post.contains("fig") && !has;
    }

    // Geste (SYNC_ANIMATION, Namen in Startreihenfolge, L5)
    static bool AGesture(std::string& why) {
        zCModel* m = player->GetModel();
        if (!m) {
            why = "Held ohne Modell";
            return false;
        }
        for (const char* a : { "T_DONTKNOW", "T_SEARCH", "T_YES" }) {
            const int id = m->GetAniIDFromAniName(zSTRING(a));
            if (id >= 0) {
                CT.gesture = a;
                m->StartAni(id, 0);
                return true;
            }
        }
        why = "keine Test-Geste im Modell";
        return false;
    }

    static bool JGesture(const json&, const json& post, std::string& d) {
        const bool has = post.contains("fig") && CTHas(post["fig"].value("ani", json::array()), CT.gesture);
        const json mine = CTAnis(player);
        d = "Figur beim Partner spielt " + CT.gesture + (has ? "" : " NICHT") + (post.contains("fig") ? " (laufend: " + post["fig"].value("ani", json::array()).dump() + ")" : "") +
            ", hier " + mine.dump();
        // v108 (Test v107, Lauf am Client): beim eigenen Helden schon vorbei/unterbrochen - dann nicht bewertbar
        if (!has && !CTHas(mine, CT.gesture))
            d = "-- " + d + " - Geste beim eigenen Helden schon vorbei";
        return has;
    }

    // Heldenwerte bleiben beim eigenen Helden (D10/F2) - Staerke und Erfahrung ohne Skript
    static bool AStr(std::string&) {
        CT.strOld = player->GetAttribute(NPC_ATR_STRENGTH);
        player->SetAttribute(NPC_ATR_STRENGTH, CT.strOld + 1);
        return true;
    }

    static bool JStr(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "hero", "str"), b = CTInt(post, "hero", "str");
        d = CTFmt("Staerke des Partner-Helden %d -> %d (soll gleich)", a, b);
        return a != -999 && a == b && CTInt(pre, "hero", "dex") == CTInt(post, "hero", "dex");
    }

    static void CStr() { player->SetAttribute(NPC_ATR_STRENGTH, CT.strOld); }

    static bool AExpLocal(std::string&) {
        CT.expOld = player->experience_points;
        player->experience_points += 7;
        return true;
    }

    static bool JExpLocal(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "hero", "exp"), b = CTInt(post, "hero", "exp");
        d = CTFmt("EP des Partner-Helden %d -> %d (soll gleich - nur Quest-EP aus Skripten werden geteilt)", a, b);
        return a != -999 && a == b;
    }

    static void CExpLocal() { player->experience_points = CT.expOld; }

    // =====================================================================================================================
    // Bereich 3: Story und Tagebuch
    // =====================================================================================================================

    static bool PStory(std::string& why) {
        StoryReadConfig();
        if (!CoopRuleQuests() || !StorySyncOn) {
            why = "Abgleich Quests aus";
            return false;
        }
        return true;
    }

    static bool PDiary(std::string& why) {
        if (!CoopRuleLog()) {
            why = "Abgleich Tagebuch aus";
            return false;
        }
        return true;
    }

    // Tagebuch: Thema + Eintrag wie aus einem Dialog (Schnappschuss, B_LogEntry, Unterschied senden)
    static bool ATopic(std::string& why) {
        zSTRING topic("Coop-Test");
        CoopDlgSnap a, b;
        DlgSnapshot(a);
        if (!DialogFindTopic("Coop-Test")) {
            CoopParserExternal create = DialogExternal("LOG_CREATETOPIC");
            CoopParserExternal status = DialogExternal("LOG_SETTOPICSTATUS");
            if (!create || !status) {
                why = "LOG_CREATETOPIC fehlt";
                return false;
            }
            DialogPushString(&topic);
            DialogPushInt(0);   // LOG_MISSION
            create();
            DialogPushString(&topic);
            DialogPushInt(1);   // LOG_RUNNING
            status();
        }
        zSTRING entry(("Testlauf " + CTStoryTag() + " von " + DialogMe()).c_str());
        if (!DialogScriptLogEntry(topic, entry))
            if (CoopParserExternal add = DialogExternal("LOG_ADDENTRY")) {
                DialogPushString(&topic);
                DialogPushString(&entry);
                add();
            }
        DlgSnapshot(b);
        DlgSendDiff(a, b, std::string(), "Coop-Test");
        StoryAbsorb(a, b);
        return true;
    }

    static bool JTopic(const json&, const json& post, std::string& d) {
        const int ex = CTInt(post, "topic", "ex"), tag = CTInt(post, "topic", "tag");
        d = std::string("Partner: Thema ") + (ex == 1 ? "da" : "fehlt") + ", Eintrag " + (tag == 1 ? "da" : "fehlt");
        return ex == 1 && tag == 1;
    }

    // Themenstatus (Story-Takt)
    static bool CTSetStatus(int st, std::string& why) {
        zSTRING topic("Coop-Test");
        CoopParserExternal status = DialogExternal("LOG_SETTOPICSTATUS");
        if (!status || !DialogFindTopic("Coop-Test")) {
            why = "Thema fehlt (Schritt davor)";
            return false;
        }
        DialogPushString(&topic);
        DialogPushInt(st);
        status();
        return true;
    }

    static bool AStatusDone(std::string& why) { return CTSetStatus(2, why); }
    static bool AStatusRun(std::string& why) { return CTSetStatus(1, why); }

    static bool CTJudgeStatus(const json& post, int want, std::string& d) {
        const int st = CTInt(post, "topic", "st");
        d = CTFmt("Partner-Status %d (soll %d)", st, want);
        return st == want;
    }

    static bool JStatusDone(const json&, const json& post, std::string& d) { return CTJudgeStatus(post, 2, d); }
    static bool JStatusRun(const json&, const json& post, std::string& d) { return CTJudgeStatus(post, 1, d); }

    // Eintrag ausserhalb eines Dialogs: nur LOG_ADDENTRY - muss mit dem Story-Takt (jede Sekunde) ankommen (D7)
    static bool AEntryTick(std::string& why) {
        CoopParserExternal add = DialogExternal("LOG_ADDENTRY");
        if (!add || !DialogFindTopic("Coop-Test")) {
            why = "Thema oder LOG_ADDENTRY fehlt";
            return false;
        }
        zSTRING topic("Coop-Test");
        zSTRING entry(("Story-Takt " + CTStoryTag() + " von " + DialogMe()).c_str());
        DialogPushString(&topic);
        DialogPushString(&entry);
        add();
        return true;
    }

    static bool JEntryTick(const json&, const json& post, std::string& d) {
        const int has = CTInt(post, "topic", "tag2");
        d = std::string("Eintrag beim Partner ") + (has == 1 ? "da" : "fehlt");
        return has == 1;
    }

    // Quest-EP +10 wie aus einem Dialog (B_GiveXP beim Partner, D11)
    static bool AQuestXp(std::string& why) {
        CoopDlgSnap a, b;
        DlgSnapshot(a);
        if (!DialogScriptGiveXP(10)) {
            why = "B_GiveXP fehlt";
            return false;
        }
        DlgSnapshot(b);
        DlgSendDiff(a, b, std::string(), "Coop-Test");
        StoryAbsorb(a, b);
        return true;
    }

    static bool JQuestXp(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "exp"), b = CTInt(post, "exp");
        d = CTFmt("Partner-EP %d -> %d (soll +10)", a, b);
        return a != -999 && b - a == 10;
    }

    // Skript-Variable COOP_TEST_VAR (COOP_ITEMS.D ab v107): Story-Takt, Dialog-Weg, zurueck
    static zCPar_Symbol* CTVarSym() {
        zCPar_Symbol* s = parser ? parser->GetSymbol(zSTRING(CT_VAR)) : NULL;
        return s && s->type == zPAR_TYPE_INT && !(s->flags & zPAR_FLAG_CONST) ? s : NULL;
    }

    static bool PVar(std::string& why) {
        if (!PStory(why))
            return false;
        if (!CTVarSym()) {
            why = "COOP_TEST_VAR fehlt (COOP_ITEMS.D vor v107)";
            return false;
        }
        return true;
    }

    static bool AVarTick(std::string&) {
        CT.varValue = 1000 + (int)((GetCurrentMs() / 1000) % 8000);
        CTVarSym()->SetValue(CT.varValue, 0);
        CT.varSet = true;
        return true;
    }

    static bool AVarDialog(std::string&) {
        CoopDlgSnap a, b;
        DlgSnapshot(a);
        CT.varValue++;
        CTVarSym()->SetValue(CT.varValue, 0);
        CT.varSet = true;
        DlgSnapshot(b);
        DlgSendDiff(a, b, std::string(), "Coop-Test");
        StoryAbsorb(a, b);
        return true;
    }

    static bool AVarBack(std::string&) {
        CT.varValue = 0;
        CTVarSym()->SetValue(0, 0);
        CT.varSet = false;
        return true;
    }

    static bool JVar(const json&, const json& post, std::string& d) {
        const int v = post.contains("var") ? post["var"].value(CT_VAR, -999) : -999;
        d = CTFmt("%s beim Partner %d (soll %d)", CT_VAR, v, CT.varValue);
        return v == CT.varValue;
    }

    // Heldenvariable (v31, Othellos Ausruestungs-/Bonuswerte) bleibt beim eigenen Helden
    static bool PHeroVar(std::string& why) {
        if (!PStory(why))
            return false;
        DlgBuildIntSlots();   // ordnet die Heldenwerte ein
        zCPar_Symbol* pick = NULL;
        for (zCPar_Symbol* s : StoryHeroVars)
            if (s && s->type == zPAR_TYPE_INT && !(s->flags & zPAR_FLAG_CONST) && s->ele <= 1 &&
                (!pick || strcmp(s->name.ToChar(), pick->name.ToChar()) < 0))
                pick = s;
        if (!pick) {
            why = "keine Heldenvariable erkannt";
            return false;
        }
        CT.heroVar = pick->name.ToChar();
        return true;
    }

    static bool AHeroVar(std::string&) {
        zCPar_Symbol* s = parser->GetSymbol(zSTRING(CT.heroVar.c_str()));
        s->GetValue(CT.heroVarOld, 0);
        s->SetValue(CT.heroVarOld + 1, 0);
        CT.heroVarSet = true;
        return true;
    }

    static bool JHeroVar(const json& pre, const json& post, std::string& d) {
        const int a = pre.contains("var") ? pre["var"].value(CT.heroVar, -999) : -999;
        const int b = post.contains("var") ? post["var"].value(CT.heroVar, -999) : -999;
        d = CTFmt("%s beim Partner %d -> %d (soll gleich; hier %d -> %d)", CT.heroVar.c_str(), a, b, CT.heroVarOld, CT.heroVarOld + 1);
        return a != -999 && a == b;
    }

    static void CHeroVar() {
        if (!CT.heroVarSet)
            return;
        if (zCPar_Symbol* s = parser->GetSymbol(zSTRING(CT.heroVar.c_str())))
            s->SetValue(CT.heroVarOld, 0);
        CT.heroVarSet = false;
    }

    // Ruf (nur Othello, D17): B_IncreaseRep mit Meldung, der Partner bekommt dieselbe Meldung; danach zurueck
    static bool PRep(std::string& why) {
        if (!PStory(why))
            return false;
        zCPar_Symbol* s = parser->GetSymbol(zSTRING("REPUTATIONOC"));
        if (!s || s->type != zPAR_TYPE_INT || parser->GetIndex(zSTRING("B_INCREASEREP")) < 0) {
            why = "REPUTATIONOC / B_INCREASEREP fehlen";
            return false;
        }
        return true;
    }

    static int CTRep() {
        int v = 0;
        if (zCPar_Symbol* s = parser->GetSymbol(zSTRING("REPUTATIONOC")))
            s->GetValue(v, 0);
        return v;
    }

    static bool ARep(std::string& why) {
        CT.repOld = CTRep();
        if (!DialogScriptIncreaseRep(1, 1)) {
            why = "B_IncreaseRep nicht ausfuehrbar";
            return false;
        }
        CT.repSet = true;
        return true;
    }

    static bool JRep(const json&, const json& post, std::string& d) {
        const int v = post.contains("var") ? post["var"].value("REPUTATIONOC", -999) : -999;
        d = CTFmt("REPUTATIONOC beim Partner %d, hier %d (vorher %d)", v, CTRep(), CT.repOld);
        return v == CTRep() && CTRep() == CT.repOld + 1;
    }

    static bool PRepBack(std::string& why) {
        if (!CT.repSet) {
            why = "kein Ruf aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool ARepBack(std::string&) {
        if (zCPar_Symbol* s = parser->GetSymbol(zSTRING("REPUTATIONOC")))
            s->SetValue(CT.repOld, 0);
        CT.repSet = false;
        return true;
    }

    static bool JRepBack(const json&, const json& post, std::string& d) {
        const int v = post.contains("var") ? post["var"].value("REPUTATIONOC", -999) : -999;
        d = CTFmt("REPUTATIONOC beim Partner %d (soll %d)", v, CT.repOld);
        return v == CT.repOld;
    }

    // Gildenhaltung Troll/Golem (Story-Takt D9) und zurueck
    static bool PGuild(std::string& why) {
        if (!PStory(why))
            return false;
        CT.guildOld1 = CTGuild(CTG1(), CTG2());
        CT.guildOld2 = CTGuild(CTG2(), CTG1());
        if (CT.guildOld1 < 0 || CT.guildOld2 < 0) {
            why = "Gildentabelle nicht lesbar";
            return false;
        }
        return true;
    }

    static bool AGuild(std::string&) {
        oCGuilds* g = ogame->GetGuilds();
        CT.guildNew = CT.guildOld1 == 2 ? 1 : 2;
        g->SetAttitude(CTG1(), CTG2(), (unsigned char)CT.guildNew);
        g->SetAttitude(CTG2(), CTG1(), (unsigned char)CT.guildNew);
        CT.guildSet = true;
        return true;
    }

    static bool PGuildBack(std::string& why) {
        if (!CT.guildSet) {
            why = "kein Wert aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool AGuildBack(std::string&) {
        oCGuilds* g = ogame->GetGuilds();
        g->SetAttitude(CTG1(), CTG2(), (unsigned char)CT.guildOld1);
        g->SetAttitude(CTG2(), CTG1(), (unsigned char)CT.guildOld2);
        CT.guildSet = false;
        return true;
    }

    // u136 (P133): Gildenhaltung mit Gilde -1, wie sie C_NpcIsBotheredByPlayerRoomGuild in den Kastell-Raeumen abfragt. Die Engine liest dann vor
    // der Tabelle; gestuerzt ist sie im Testlauf nur, weil davor kein Speicher lag - das stellt hier eine Kopie der Tabelle hinter einer
    // gesperrten Seite nach. Fehlt der Haken in Hooks.cpp, stuerzt das Spiel in diesem Schritt ab (bei Gothic2.exe+0x300D7F).
    static int CTGuildNegA = -9, CTGuildNegB = -9;
    static bool CTGuildNegSame = false;

    static bool PGuildNeg(std::string& why) {
        if (!CoopAddr::oCGuilds_GetAttitude) {
            why = "nur G2A (kein Skript gibt hier einem Raum die Gilde -1)";
            return false;
        }
        oCGuilds* g = ogame ? ogame->GetGuilds() : NULL;
        if (!g || !g->attitude || g->size < 3) {
            why = "Gildentabelle nicht lesbar";
            return false;
        }
        return true;
    }

    static bool AGuildNeg(std::string& why) {
        oCGuilds* g = ogame->GetGuilds();
        const SIZE_T page = 0x1000, bytes = (SIZE_T)g->size * g->size, room = (bytes + page - 1) / page * page;
        unsigned char* base = (unsigned char*)VirtualAlloc(NULL, page + room, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        DWORD old = 0;
        if (!base || !VirtualProtect(base, page, PAGE_NOACCESS, &old)) {
            why = "Speicher fuer die Kopie nicht bekommen";
            if (base)
                VirtualFree(base, 0, MEM_RELEASE);
            return false;
        }
        unsigned char* was = g->attitude;
        memcpy(base + page, was, bytes);
        g->attitude = base + page;
        CTGuildNegA = g->GetAttitude(1, -1);
        CTGuildNegB = g->GetAttitude(-1, 1);
        CTGuildNegSame = g->GetAttitude(1, 2) == was[2 * g->size + 1] && g->GetAttitude(2, 1) == was[g->size + 2];
        g->attitude = was;
        VirtualFree(base, 0, MEM_RELEASE);
        return true;
    }

    static bool JGuildNeg(const json&, const json&, std::string& d) {
        oCPortalRoomManager* pm = ogame->GetPortalRoomManager();
        int rooms = 0, neg = 0;
        std::string names;
        for (int i = 0; pm && i < pm->portals.GetNum(); i++) {
            oCPortalRoom* r = pm->portals[i];
            rooms++;
            if (r && r->ownerGuild < 0 && neg++ < 5)
                names += std::string(neg > 1 ? ", " : " (") + r->portalName.ToChar();
        }
        d = CTFmt("Gilde -1 liefert %d/%d (neutral %d), gueltige Gilden %s; %d Raeume, %d mit Gilde -1%s%s", CTGuildNegA, CTGuildNegB, (int)NPC_ATT_NEUTRAL,
            CTGuildNegSame ? "wie in der Tabelle" : "FALSCH", rooms, neg, names.c_str(), neg ? ")" : "");
        return CTGuildNegA == NPC_ATT_NEUTRAL && CTGuildNegB == NPC_ATT_NEUTRAL && CTGuildNegSame;
    }

    static bool CTJudgeGuild(const json& post, std::string& d) {
        int p1 = -999, p2 = -999;
        try {
            p1 = post.at("guild").at(0).get<int>();
            p2 = post.at("guild").at(1).get<int>();
        }
        catch (...) {}
        const int m1 = CTGuild(CTG1(), CTG2()), m2 = CTGuild(CTG2(), CTG1());
        d = CTFmt("Partner %d/%d, hier %d/%d", p1, p2, m1, m2);
        return p1 == m1 && p2 == m2;
    }

    static bool JGuild(const json&, const json& post, std::string& d) { return CTJudgeGuild(post, d) && CTGuild(CTG1(), CTG2()) == CT.guildNew; }
    static bool JGuildBack(const json&, const json& post, std::string& d) { return CTJudgeGuild(post, d) && CTGuild(CTG1(), CTG2()) == CT.guildOld1; }

    // --- u86 (P110): Gilden je Held (Variante C) ----------------------------------------------------------------------
    // Nutzer: "mach außerdem dann den Test für die Gilden quests simuliere ein paar belohnungen, erfülllte quests, beitritt etc.".
    // Der Partner steht fuer die Schritte in einer Testgilde (Bauer, auf Wunsch per Frage "gld"); jeder Schritt schickt einen
    // Fortschritt wie aus einem Dialog - Testvariable, drei Aepfel als Belohnung, Tagebuch-Thema erfuellt - mit vorgegebenen
    // abgefragten Gilden (DlgSendDiff giTest) bzw. als Aufnahme (eigene Gilde waehrend des Infos geaendert). Am Ende ist alles
    // zurueck (Gilde, Testvariable 0, sie ist wieder ein Story-Wert) - die Vergleiche danach ("Alle Skript-Variablen gleich",
    // "Ganzes Tagebuch gleich") muessen weiter stimmen.
    // u96 (P118): Missions-Gegenstaende spiegeln - eine alte Muenze (G2A ITMI_OLDCOIN, G1 ITMI_STUFF_OLDCOIN_02, beide mit ITEM_MISSION)
    // bekommen bzw. wieder abgeben; der Partner hat sie danach auch bzw. nicht mehr (CoopDialogSync.cpp, "mi")
    static const char* CTMissionItem() {
        static const char* found = NULL;
        static bool done = false;
        if (done || !parser)
            return found;
        done = true;
        for (const char* c : { "ITMI_OLDCOIN", "ITMI_STUFF_OLDCOIN_02" }) {
            const int idx = parser->GetIndex(zSTRING(c));
            oCItem* it = idx >= 0 ? CreateCoopItem(idx) : NULL;
            if (!it)
                continue;
            const bool mi = it->HasFlag(ITM_FLAG_MI) != 0;
            it->Release();
            if (mi) {
                found = c;
                break;
            }
        }
        return found;
    }

    static bool PMi(std::string& why) {
        if (!player || !CTMissionItem()) {
            why = "kein Missions-Gegenstand zum Testen in den Skripten";
            return false;
        }
        if (!CoopRuleQuests()) {
            why = "Abgleich \"Quests\" aus";
            return false;
        }
        return true;
    }

    static json QMi() { json q; q["mi"] = CTMissionItem() ? CTMissionItem() : ""; return q; }

    static bool AMiGive(std::string& why) {
        const int idx = parser->GetIndex(zSTRING(CTMissionItem()));
        oCItem* it = idx >= 0 ? CreateCoopItem(idx) : NULL;
        if (!it) {
            why = "Gegenstand nicht erzeugt";
            return false;
        }
        it->amount = 1;
        player->PutInInv(it);   // wie ein erhaltener Gegenstand - nicht unter WorldSyncApplying
        return true;
    }

    static bool JMiGive(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "mi", "n"), b = CTInt(post, "mi", "n");
        d = CTFmt("%s beim Partner %d -> %d (soll +1)", CTMissionItem(), a, b);
        return a >= 0 && b - a == 1;
    }

    // u104 (P126): die gespiegelte Muenze traegt hier und beim Partner die Kennzeichnung (Name, Beschreibung, Infozeile), ein Apfel nie
    static bool PMiMark(std::string& why) {
        if (!PMi(why))
            return false;
        if (CTInvCount(CTMissionItem()) < 1) {
            why = "keine Muenze im eigenen Inventar (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool JMiMark(const json&, const json& post, std::string& d) {
        const int hier = CoopMissionTestMark(CTMissionItem()), dort = CTInt(post, "mi", "mark");
        const int apfel = CTApple() ? CoopMissionTestMark(CTApple()) : -1;
        d = CTFmt("%s gekennzeichnet hier %d, beim Partner %d (soll je 1); Apfel %d (soll nicht 1)", CTMissionItem(), hier, dort, apfel);
        return hier == 1 && dort == 1 && apfel != 1;
    }

    static bool AMiTake(std::string& why) {
        const std::string inst = CTMissionItem();
        player->inventory2.UnpackAllItems();
        oCItem* have = NULL;
#if ENGINE >= Engine_G2
        for (zCListSort<oCItem>* n = player->inventory2.inventory.next; n && !have; n = n->next)
            if (n->data && inst == n->data->GetInstanceName().ToChar())
                have = n->data;
#else
        for (int c = 0; c < INV_MAX && !have; c++)
            for (zCListSort<oCItem>* n = player->inventory2.inventory[c].next; n && !have; n = n->next)
                if (n->data && inst == n->data->GetInstanceName().ToChar())
                    have = n->data;
#endif
        if (!have) {
            why = "keine Muenze im eigenen Inventar";
            return false;
        }
        oCItem* r = player->RemoveFromInv(have, 1);   // wie abgegeben
        if (r)
            ogame->GetGameWorld()->RemoveVob(r);
        return true;
    }

    static bool JMiTake(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "mi", "n"), b = CTInt(post, "mi", "n");
        d = CTFmt("%s beim Partner %d -> %d (soll -1)", CTMissionItem(), a, b);
        return a >= 1 && b - a == -1;
    }

    // u97 (P119): gemeinsame Coop-Truhe (CoopTruhe.cpp) - zwei Aepfel hinein wie beim Rueberziehen, das Fenster ueber den
    // Gegenstand oeffnen (EV_USEITEM wie aus dem Inventar; GT schliesst es nach 1 s selbst). u99 (P121, beide gleichzeitig offen):
    // beide Fenster offen - ein Apfel hinein erscheint beim Partner im offenen Fenster; beide nehmen zugleich alle Aepfel - der Host
    // zuerst, der Client danach aus seiner veralteten Kopie (er haelt dazu die Staende des Hosts zurueck): der Client gibt sie zurueck
    // ("undo"), beide Truhen sind leer, sein Inventar wie vorher; zuletzt beide Fenster zu, gleicher Inhalt.
    static int CTTruheAuf0 = 0;
    static int CTTruheRunde = 0;        // je Gleichstand-Schritt eine Kennung - der Partner handelt je Kennung einmal
    static bool CTTruheNachA = false;   // Abfrage vor bzw. nach der Handlung
    static int CTTruheMenge = 0, CTTruheInv0 = 0, CTTruheRueck0 = 0;

    static bool PTruhe(std::string& why) {
        int n = 0;
        std::string offen;
        CoopTruheTestStand(n, offen, CTApple() ? CTApple() : "");
        if (n < 0) {
            why = "Coop-Truhe in dieser Engine aus";
            return false;
        }
        if (!player || !parser || parser->GetIndex(zSTRING("ITMI_COOP_TRUHE")) < 0) {
            why = "ITMI_COOP_TRUHE fehlt im Skript (COOP_ITEMS.D zu alt)";
            return false;
        }
        if (!CTApple() || parser->GetIndex(zSTRING(CTApple())) < 0) {
            why = "kein Apfel in den Skripten";
            return false;
        }
        return true;
    }

    static json QTruhe() { json q; q["truhe"] = CTApple() ? CTApple() : ""; return q; }

    static bool ATruheRein(std::string& why) {
        if (!CoopTruheTestAendern(CTApple(), 2)) {
            why = "zwei Aepfel nicht hineingelegt";
            return false;
        }
        return true;
    }

    static bool JTruheRein(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "truhe", "n"), b = CTInt(post, "truhe", "n");
        const std::string offen = CTStr(post, "truhe", "offen");
        d = CTFmt("%s in der Truhe beim Partner %d -> %d (soll +2), offen bei: %s", CTApple(), a, b, offen.empty() ? "niemandem" : offen.c_str());
        return a >= 0 && b - a == 2 && offen.empty();
    }

    static bool PTruheAuf(std::string& why) {
        if (!PTruhe(why))
            return false;
        if (player->GetWeaponMode() != NPC_WEAPON_NONE) {
            why = "Waffe gezogen - Benutzen geht nicht";
            return false;
        }
        if (!player->IsInInv(parser->GetIndex(zSTRING("ITMI_COOP_TRUHE")), 1)) {
            why = "Coop-Truhe nicht im Inventar (GT legt sie nach dem Laden ein)";
            return false;
        }
        return true;
    }

    static int CTTruheDirekt0 = 0, CTTruheGegeben0 = 0;

    static bool ATruheAuf(std::string& why) {
        CTTruheAuf0 = CoopTruheTestOeffnungen();
        CTTruheDirekt0 = CoopTruheTestDirekt();
        CTTruheGegeben0 = CoopTruheTestGegeben();
        if (!CoopTruheTestBenutzen(1000)) {
            why = "Benutzen nicht moeglich (Fenster offen bzw. Anfrage laeuft)";
            return false;
        }
        return true;
    }

    // u101: direkt ueber den Haken (ohne Bewegung) und nicht verbraucht - kein neuer Gegenstand noetig
    static bool JTruheAuf(const json&, const json& post, std::string& d) {
        const int auf = CoopTruheTestOeffnungen() - CTTruheAuf0;
        const int direkt = CoopTruheTestDirekt() - CTTruheDirekt0, neu = CoopTruheTestGegeben() - CTTruheGegeben0;
        const std::string offen = CTStr(post, "truhe", "offen");
        const int item = CTInvCount("ITMI_COOP_TRUHE");
        // u102: Titel des Fensters (G2; G1 hat keinen)
        const std::string titel = CoopTruheTestTitel();
#if ENGINE >= Engine_G2
        const bool titelOk = titel == CoopTr("Coop-Inventar", "Coop inventory");
#else
        const bool titelOk = true;
#endif
        d = CTFmt("hier ueber den Gegenstand geoeffnet und wieder zu: %d (soll 1), direkt ohne Bewegung: %d (soll 1), neu eingelegt: %d "
            "(soll 0), Titel '%s', offen laut Partner bei: %s, Coop-Truhe im Inventar: %d (soll 1)", auf, direkt, neu, titel.c_str(),
            offen.empty() ? "niemandem" : offen.c_str(), item);
        return auf == 1 && direkt == 1 && neu == 0 && titelOk && offen.empty() && item == 1;
    }

    // u99: der Partner oeffnet sein Fenster schon bei der Frage vorher, ich danach meins und lege einen Apfel hinein
    static json QTruheBeide() { json q = QTruhe(); q["truheFenster"] = 30000; return q; }

    static bool ATruheBeide(std::string& why) {
        if (!CoopTruheTestFenster(30000)) {
            why = "Fenster hier nicht offen";
            return false;
        }
        if (!CoopTruheTestAendern(CTApple(), 1)) {
            why = "Apfel nicht hineingelegt";
            return false;
        }
        return true;
    }

    static bool JTruheBeide(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "truhe", "n"), b = CTInt(post, "truhe", "n"), auf = CTInt(post, "truhe", "auf");
        const std::string offen = CTStr(post, "truhe", "offen");
        const bool hier = CoopTruheTestOffenHier();
        d = CTFmt("%s beim Partner %d -> %d (soll +1), sein Fenster %s, meins %s, offen laut Host bei: %s", CTApple(), a, b,
            auf == 1 ? "offen" : "zu", hier ? "offen" : "zu", offen.empty() ? "niemandem" : offen.c_str());
        return a >= 0 && b - a == 1 && auf == 1 && hier && offen.find(MyselfId.ToChar()) != std::string::npos;
    }

    // u99: Gleichstand - nur ein Client kann eine veraltete Kopie haben: der Host nimmt zuerst, der Client danach aus der Kopie
    static std::string CTTruheKennung() {
        return CTFmt("%s-%d", CT.tag.c_str(), CTTruheRunde);
    }

    static bool PTruheGleich(std::string& why) {
        if (!PTruhe(why))
            return false;
        int n = 0;
        std::string offen;
        CoopTruheTestStand(n, offen, CTApple());
        if (n <= 0) {
            why = "keine Aepfel in der Truhe (Schritte davor)";
            return false;
        }
        CTTruheMenge = n;
        CTTruheNachA = false;
        CTTruheRunde++;
        CTTruheInv0 = CTInvCount(CTApple());
        CTTruheRueck0 = CoopTruheTestRueckgaben();
        if (!ServerThread)
            CoopTruheTestHalten(true);   // Client testet: Staende des Hosts ab jetzt zurueckhalten - die Kopie hier veraltet
        return true;
    }

    static json QTruheGleich() {
        json q = QTruhe();
        if (ServerThread) {   // Host testet: der Client haelt vorher zurueck und nimmt nachher aus seiner veralteten Kopie
            if (!CTTruheNachA)
                q["truheHalten"] = CTTruheKennung();
            else
                q["truheNimm"] = json::array({ CTTruheKennung(), CTTruheMenge, 1 });
        }
        else if (!CTTruheNachA)   // Client testet: der Host nimmt vorher (verbraucht), ich danach aus der Kopie
            q["truheNimm"] = json::array({ CTTruheKennung(), CTTruheMenge, 0 });
        return q;
    }

    static bool ATruheGleich(std::string& why) {
        CTTruheNachA = true;
        if (ServerThread) {
            const int r = CoopTruheTestNimm(CTApple(), CTTruheMenge, false);
            if (r != CTTruheMenge) {
                why = CTFmt("Host nahm %d von %d", r, CTTruheMenge);
                return false;
            }
            return true;
        }
        const int r = CoopTruheTestNimm(CTApple(), CTTruheMenge, true);   // aus der veralteten Kopie ins Inventar
        CoopTruheTestHalten(false);
        if (r != CTTruheMenge) {
            why = CTFmt("aus der Kopie nur %d von %d genommen (Kopie nicht veraltet?)", r, CTTruheMenge);
            return false;
        }
        return true;
    }

    static bool JTruheGleich(const json& pre, const json& post, std::string& d) {
        int hier = 0;
        std::string offen;
        CoopTruheTestStand(hier, offen, CTApple());
        const int dort = CTInt(post, "truhe", "n");
        if (ServerThread) {
            const int r0 = CTInt(pre, "truhe", "rueck"), r1 = CTInt(post, "truhe", "rueck");
            const int i0 = CTInt(pre, "truhe", "inv"), i1 = CTInt(post, "truhe", "inv");
            d = CTFmt("ich (Host) nahm %d %s zuerst; der Client danach aus seiner Kopie: zurueckgegeben %d -> %d (soll +1), %s im "
                "Inventar %d -> %d (soll gleich); Truhe hier %d, beim Client %d (soll 0)", CTTruheMenge, CTApple(), r0, r1, CTApple(), i0, i1,
                hier, dort);
            return r0 >= 0 && r1 - r0 == 1 && i1 == i0 && hier == 0 && dort == 0;
        }
        const int rueck = CoopTruheTestRueckgaben() - CTTruheRueck0, inv = CTInvCount(CTApple());
        d = CTFmt("der Host nahm %d %s zuerst, ich danach aus der veralteten Kopie: zurueckgegeben %d (soll 1), %s im Inventar %d -> %d "
            "(soll gleich); Truhe hier %d, beim Host %d (soll 0)", CTTruheMenge, CTApple(), rueck, CTApple(), CTTruheInv0, inv, hier, dort);
        return rueck == 1 && inv == CTTruheInv0 && hier == 0 && dort == 0;
    }

    // u99: beide Fenster zu - der Partner schon bei der Frage vorher; danach gleicher Inhalt, bei niemandem offen
    static json QTruheZu() { json q = QTruhe(); q["truheZu"] = 1; return q; }

    static bool ATruheZu(std::string&) {
        CoopTruheTestZu();
        return true;
    }

    static bool JTruheZu(const json&, const json& post, std::string& d) {
        int hier = 0;
        std::string offen;
        CoopTruheTestStand(hier, offen, CTApple());
        const int dort = CTInt(post, "truhe", "n"), auf = CTInt(post, "truhe", "auf");
        const std::string dortOffen = CTStr(post, "truhe", "offen");
        d = CTFmt("Fenster beim Partner %s, hier %s; offen laut Partner bei: %s; %s hier %d, beim Partner %d (soll gleich)",
            auf == 0 ? "zu" : "offen", CoopTruheTestOffenHier() ? "offen" : "zu", dortOffen.empty() ? "niemandem" : dortOffen.c_str(),
            CTApple(), hier, dort);
        return auf == 0 && !CoopTruheTestOffenHier() && dortOffen.empty() && hier == dort;
    }

    // u103 (P125): Gegenstaende im Coop-Inventar zaehlen bei Quests (CoopTruhe.cpp) - vier Aepfel ins Coop-Inventar, dann die
    // Skript-Funktionen fuer den eigenen Helden wie aus einem Dialog: Npc_HasItems zaehlt eigene plus Coop-Inventar,
    // Npc_RemoveInvItems (eigene + 2, wie B_GiveInvItems) nimmt erst alle eigenen, dann 2 aus dem Coop-Inventar, Npc_RemoveInvItem
    // ohne eigene einen aus dem Coop-Inventar. Beim Partner kommt +1 an; die eigenen Aepfel bekommt der Held danach zurueck.
    static void CTGiveApples(int n);
    static int CTQuestEigen0 = 0, CTQuestCoop0 = 0, CTQuestHat = 0, CTQuestRetN = 0, CTQuestEigen1 = 0, CTQuestCoop1 = 0;
    static int CTQuestRet1 = 0, CTQuestCoop2 = 0, CTQuestEigen2 = 0, CTQuestGenommen0 = 0, CTQuestGenommen = 0;

    // External wie aus einem Skript fuer HERO: Instanz als Symbolnummer (PUSHINST), int-Parameter [Wert, 0x40]; Rueckgabe vom Stapel
    static int CTHeroExt(const char* ext, int a, int b, int n) {
        CoopParserExternal f = DialogExternal(ext);
        const int hero = parser ? parser->GetIndex(zSTRING("HERO")) : -1;
        if (!f || hero < 0 || zCParser::GetParser() != parser)
            return -1000;
        parser->datastack.Push(hero);
        DialogPushInt(a);
        if (n > 1)
            DialogPushInt(b);
        f();
        int r = 0;
        parser->GetParameter(r);
        return r;
    }

    static bool PTruheQuest(std::string& why) {
        if (!PTruhe(why))
            return false;
        if (!CoopTruheTestQuestBereit()) {
            why = "Npc_HasItems/Npc_RemoveInvItems nicht umgeleitet";
            return false;
        }
        if (parser->GetIndex(zSTRING("HERO")) < 0 || zCParser::GetParser() != parser) {
            why = "HERO fehlt bzw. anderer Parser aktiv";
            return false;
        }
        return true;
    }

    static bool ATruheQuest(std::string& why) {
        const int apple = parser->GetIndex(zSTRING(CTApple()));
        std::string offen;
        CTQuestEigen0 = CTInvCount(CTApple());
        CoopTruheTestStand(CTQuestCoop0, offen, CTApple());
        CTQuestGenommen0 = CoopTruheTestQuestGenommen();
        if (!CoopTruheTestAendern(CTApple(), 4)) {
            why = "vier Aepfel nicht ins Coop-Inventar gelegt";
            return false;
        }
        CTQuestHat = CTHeroExt("NPC_HASITEMS", apple, 0, 1);
        CTQuestRetN = CTHeroExt("NPC_REMOVEINVITEMS", apple, CTQuestEigen0 + 2, 2);
        CTQuestEigen1 = CTInvCount(CTApple());
        CoopTruheTestStand(CTQuestCoop1, offen, CTApple());
        CTQuestRet1 = CTHeroExt("NPC_REMOVEINVITEM", apple, 0, 1);
        CoopTruheTestStand(CTQuestCoop2, offen, CTApple());
        CTQuestGenommen = CoopTruheTestQuestGenommen() - CTQuestGenommen0;
        CTGiveApples(CTQuestEigen0 - CTInvCount(CTApple()));   // die eigenen zurueck
        CTQuestEigen2 = CTInvCount(CTApple());
        return true;
    }

    static bool JTruheQuest(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "truhe", "n"), b = CTInt(post, "truhe", "n");
        const int c0 = CTQuestCoop0;
        d = CTFmt("Npc_HasItems(Held, %s) = %d (soll %d eigene + %d im Coop-Inventar); Npc_RemoveInvItems(%d) -> %d, eigene %d (soll 0), "
            "Coop-Inventar %d (soll %d); Npc_RemoveInvItem -> %d, Coop-Inventar %d (soll %d); genommen %d (soll 3); eigene zurueck %d "
            "(soll %d); beim Partner %d -> %d (soll +1)", CTApple(), CTQuestHat, CTQuestEigen0, c0 + 4, CTQuestEigen0 + 2, CTQuestRetN,
            CTQuestEigen1, CTQuestCoop1, c0 + 2, CTQuestRet1, CTQuestCoop2, c0 + 1, CTQuestGenommen, CTQuestEigen2, CTQuestEigen0, a, b);
        return CTQuestHat == CTQuestEigen0 + c0 + 4 && CTQuestRetN == 1 && CTQuestEigen1 == 0 && CTQuestCoop1 == c0 + 2 &&
            CTQuestRet1 == 1 && CTQuestCoop2 == c0 + 1 && CTQuestGenommen == 3 && CTQuestEigen2 == CTQuestEigen0 && a >= 0 && b - a == 1;
    }

    static bool PGld(std::string& why) {
        if (!PVar(why) || !PDiary(why))
            return false;
        if (!parser || parser->GetIndex(zSTRING(CTApple())) < 0) {
            why = "kein Apfel in den Skripten";
            return false;
        }
        return true;
    }

    static void CTGiveApples(int n) {
        const int idx = parser ? parser->GetIndex(zSTRING(CTApple())) : -1;
        while (idx >= 0 && n > 0) {
            oCItem* it = CreateCoopItem(idx);
            if (!it)
                return;
            const bool multi = (it->flags & ITM_FLAG_MULTI) != 0;
            it->amount = multi ? n : 1;
            n -= it->amount;
            player->PutInInv(it);
        }
    }

    // Fortschritt wie aus einem Gilden-Info: Testvariable = value, Aepfel als Belohnung, ggf. Thema erfuellt; joinGuild >= 0: die
    // eigene Gilde steht waehrend des "Infos" darauf (Aufnahme), danach wieder wie vorher
    static bool CTGldSend(int value, int apples, const std::vector<int>* gi, bool topicDone, int joinGuild, std::string& why) {
        zCPar_Symbol* s = CTVarSym();
        if (!s || !player) {
            why = "COOP_TEST_VAR fehlt";
            return false;
        }
        CoopDlgSnap a, b;
        DlgSnapshot(a);
        CT.gldValue = value;
        s->SetValue(value, 0);
        if (apples > 0)
            CTGiveApples(apples);
        if (topicDone) {
            zSTRING topic("Coop-Test Gilde");
            CoopParserExternal create = DialogExternal("LOG_CREATETOPIC");
            CoopParserExternal status = DialogExternal("LOG_SETTOPICSTATUS");
            if (!create || !status) {
                why = "LOG_CREATETOPIC fehlt";
                return false;
            }
            if (!DialogFindTopic("Coop-Test Gilde")) {
                DialogPushString(&topic);
                DialogPushInt(0);   // LOG_MISSION
                create();
            }
            DialogPushString(&topic);
            DialogPushInt(2);   // LOG_SUCCESS - erfuellte Quest
            status();
        }
        const int g0 = player->guild;
        if (joinGuild >= 0)
            player->guild = joinGuild;
        DlgSnapshot(b);
        if (joinGuild >= 0)
            player->guild = g0;
        DlgSendDiff(a, b, std::string(), "Coop-Test Gilde", NULL, gi);
        StoryAbsorb(a, b);
        return true;
    }

    static bool JGldSet(const json&, const json& post, std::string& d) {
        const int g = CTInt(post, "gld", "g");
        d = CTFmt("Gilde des Partners %d (Testgilde %d)", g, CTTestGuild());
        return g == CTTestGuild();
    }

    static bool AGldSame(std::string& why) {
        const std::vector<int> gi = { CTTestGuild() };
        return CTGldSend(7000 + (int)((GetCurrentMs() / 1000) % 900), 3, &gi, true, -1, why);
    }

    static bool JGldSame(const json& pre, const json& post, std::string& d) {
        const int v = CTInt(post, "gld", "var"), a0 = CTInt(pre, "gld", "apples"), a1 = CTInt(post, "gld", "apples");
        const int tex = CTInt(post, "gld", "tex"), tst = CTInt(post, "gld", "tst");
        d = CTFmt("Partner in derselben Gilde: Variable %d (soll %d), Aepfel %d -> %d (soll +3), Thema %s, Status %d (soll 2 = erfuellt)", v,
            CT.gldValue, a0, a1, tex == 1 ? "da" : "fehlt", tst);
        return v == CT.gldValue && a0 >= 0 && a1 - a0 == 3 && tex == 1 && tst == 2;
    }

    static bool AGldOther(std::string& why) {
        const std::vector<int> gi = { CTOtherGuild() };
        return CTGldSend(CT.gldValue + 1, 3, &gi, false, -1, why);
    }

    // Variable und Aepfel bleiben beim Partner, wie sie waren
    static bool CTJudgeGldKept(const json& pre, const json& post, const char* what, std::string& d) {
        const int v0 = CTInt(pre, "gld", "var"), v = CTInt(post, "gld", "var"), a0 = CTInt(pre, "gld", "apples"), a1 = CTInt(post, "gld", "apples");
        d = CTFmt("%s: Variable %d (soll unveraendert %d, nicht %d), Aepfel %d -> %d (soll gleich)", what, v, v0, CT.gldValue, a0, a1);
        return v0 != -999 && v == v0 && v != CT.gldValue && a1 == a0;
    }

    static bool JGldOther(const json& pre, const json& post, std::string& d) {
        return CTJudgeGldKept(pre, post, "Partner in anderer Gilde (eigener Fortschritt)", d);
    }

    static bool AGldJoin(std::string& why) {
        return CTGldSend(CT.gldValue + 1, 3, NULL, false, CTOtherGuild(), why);
    }

    static bool JGldJoin(const json& pre, const json& post, std::string& d) {
        return CTJudgeGldKept(pre, post, "Aufnahme des Testenden (Partner tritt selbst bei)", d);
    }

    static bool AGldTick(std::string& why) {
        zCPar_Symbol* s = CTVarSym();
        if (!s) {
            why = "COOP_TEST_VAR fehlt";
            return false;
        }
        CT.gldValue++;
        s->SetValue(CT.gldValue, 0);   // ausserhalb eines Dialogs - Werte aus Gilden-Infos schickt der Story-Takt nicht
        return true;
    }

    static bool JGldTick(const json& pre, const json& post, std::string& d) {
        return CTJudgeGldKept(pre, post, "Story-Takt nach Gilden-Infos", d);
    }

    static bool AGldBack(std::string& why) {
        zCPar_Symbol* s = CTVarSym();
        if (!s) {
            why = "COOP_TEST_VAR fehlt";
            return false;
        }
        DlgGuildVars.erase(s);   // wieder ein Story-Wert (der Partner hat seinen mit der Frage "back" geloescht)
        CoopDlgSnap a, b;
        DlgSnapshot(a);
        CT.gldValue = 0;
        s->SetValue(0, 0);
        DlgSnapshot(b);
        DlgSendDiff(a, b, std::string(), "Coop-Test Gilde");
        StoryAbsorb(a, b);
        return true;
    }

    static bool JGldBack(const json&, const json& post, std::string& d) {
        const int g = CTInt(post, "gld", "g"), v = CTInt(post, "gld", "var");
        d = CTFmt("Partner: Gilde %d (Testgilde %d zurueckgenommen), Variable %d (soll 0)", g, CTTestGuild(), v);
        return g != -999 && g != CTTestGuild() && v == 0;
    }

    static bool JChapter(const json&, const json& post, std::string& d) {
        zCPar_Symbol* s = parser->GetSymbol(zSTRING("KAPITEL"));
        if (!s) {
            d = "-- Skript ohne KAPITEL";
            return false;
        }
        int mine = 0;
        s->GetValue(mine, 0);
        const int v = post.contains("var") ? post["var"].value("KAPITEL", -999) : -999;
        d = CTFmt("Kapitel beim Partner %d, hier %d", v, mine);
        return v == mine;
    }

    static bool JDigVars(const json&, const json& post, std::string& d) { return CTDigJudge(post, "Skript-Variablen", 0, d); }
    static bool JDigLog(const json&, const json& post, std::string& d) { return CTDigJudge(post, "Tagebuch-Themen (Status|Eintraege)", 1, d); }
    static bool JDigGuilds(const json&, const json& post, std::string& d) { return CTDigJudge(post, "Gildenhaltungen", 1, d); }
    static bool JDigNpcs(const json&, const json& post, std::string& d) { return CTDigJudge(post, "lebende NPCs (Tagesablauf|Haltung|Felder)", 0, d); }

    // =====================================================================================================================
    // Bereich 4: Test-NPCs und Welt-Ereignisse
    // =====================================================================================================================

    // u13 (D12g, Testlauf u12: Cavalorn erschoss den Test-Molerat 3 s nach dem Erscheinen): der naechste Wegpunkt bis 40 m um
    // mich, an dem kein lebender NPC (ausser Spielern) naeher als 25 m steht - sonst NULL (dann wie bisher der naechste)
    static zCWaypoint* CTQuietWaypoint() {
        zCWorld* w = ogame->GetGameWorld();
        zCWayNet* net = w ? w->wayNet : NULL;
        if (!net)
            return NULL;
        const zVEC3 me = player->GetPositionWorld();
        zCWaypoint* best = NULL;
        float bestD = 4000.f;
        for (zCListSort<zCWaypoint>* l = net->wplist.next; l; l = l->next) {
            zCWaypoint* wp = l->data;
            if (!wp)
                continue;
            const zVEC3 p = wp->GetPositionWorld();
            const float d = CoopDist(p, me);
            if (d >= bestD)
                continue;
            bool busy = false;
            for (auto* n = CoopWorldNpcListFirst(ogame->GetGameWorld()); n && !busy; n = n->next) {
                oCNpc* npc = n->data;
                if (!npc || npc == player || PlayerNpcs.count(npc) || npc->IsDead() || npc->vobLeafList.GetNum() == 0)
                    continue;
                busy = CoopDist(npc->GetPositionWorld(), p) < 2500.f;
            }
            if (!busy) {
                best = wp;
                bestD = d;
            }
        }
        return best;
    }

    // Test-NPC einfuegen (Wld_InsertNpc ueber den Host, D13) - der Lauf legt seine Ziele selbst an
    static bool CTPrepSpawn(const char* inst, std::string& why) {
        zCWaypoint* wp = ogame->GetGameWorld() && ogame->GetGameWorld()->wayNet ? CTQuietWaypoint() : NULL;
        const bool quiet = wp != NULL;
        if (!wp && ogame->GetGameWorld() && ogame->GetGameWorld()->wayNet)
            wp = ogame->GetGameWorld()->wayNet->GetNearestWaypoint(player->GetPositionWorld());
        if (!wp || !DialogExternal("WLD_INSERTNPC") || parser->GetIndex(zSTRING(inst)) < 0) {
            why = std::string("kein Wegpunkt / WLD_INSERTNPC / ") + inst;
            return false;
        }
        CT.wpName = wp->GetName().ToChar();
        CT.wpPos = wp->GetPositionWorld();   // u40
        CT.haveWp = true;
        CTLog("Test-NPC %s an %s (%.0f m von mir, %s)", inst, CT.wpName.c_str(), CoopDist(wp->GetPositionWorld(), player->GetPositionWorld()) / 100.f,
            quiet ? "kein NPC in 25 m" : "naechster Wegpunkt - ueberall NPCs in der Naehe");
        return true;
    }

    // u27 (Test u26, Client-Lauf Schritte 71 und 76: "aus einem Todesskript - nicht angefragt"): SELF stand noch auf dem Test-Molerat
    // aus Schritt 69 (sein ZS_Dead) - HostEvInDeathScript (CoopDialogSync.cpp) hielt den Aufruf fuer ein Todesskript und fragte den
    // Host nicht. Ein Skript ruft die Externals immer mit gesetztem SELF; der Testlauf setzt SELF darum fuer seine Aufrufe auf den
    // eigenen Helden (wie DialogRunScript) und danach zurueck.
    struct CTSelfHero {
        zCPar_Symbol* sym;
        void* old;
        CTSelfHero() {
            sym = parser ? parser->GetSymbol(zSTRING("SELF")) : NULL;
            old = sym ? (void*)sym->offset : NULL;
            if (sym && player)
                parser->SetInstance(zSTRING("SELF"), player);
        }
        ~CTSelfHero() {
            if (sym)
                parser->SetInstance(zSTRING("SELF"), old);
        }
    };

    // u47 (P88): Markierung fuer die Betreten-Schritte (CTMark) - Fleischwanze und Apfel am naechsten Wegpunkt einsetzen
    // (Wld_InsertNpc/Wld_InsertItem wie ein Skript; allein in einer Welt fuehrt ein Client sie selbst aus, P67), die neue
    // Fleischwanze im naechsten Bild toeten (CoopKillNoXp - kein EP, die Leiche bleibt in der Welt)
    static const char* CTBug() { return CTInstOf({ "MEATBUG" }); }

    static bool CTMarkStart(CTMark& m, std::string& why) {
        m = CTMark();
        oCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        zCWaypoint* wp = w && w->wayNet && player ? w->wayNet->GetNearestWaypoint(player->GetPositionWorld()) : NULL;
        const int bug = parser->GetIndex(zSTRING(CTBug())), apple = parser->GetIndex(zSTRING(CTApple()));
        if (!wp || bug < 0 || apple < 0 || !DialogExternal("WLD_INSERTNPC") || !DialogExternal("WLD_INSERTITEM")) {
            why = "kein Wegpunkt / MEATBUG / Apfel / WLD_INSERTNPC / WLD_INSERTITEM";
            return false;
        }
        m.world = CoopWorldName();
        m.wp = wp->GetName().ToChar();
        m.pos = wp->GetPositionWorld();
        for (auto* l = CoopWorldNpcListFirst(w); l; l = l->next)
            if (l->data && l->data->GetInstance() == bug)
                m.before.insert(l->data);
        {
            CTSelfHero self;
            zSTRING wpName(m.wp.c_str());
            DialogPushInt(bug);
            DialogPushString(&wpName);
            DialogExternal("WLD_INSERTNPC")();
            DialogPushInt(apple);
            DialogPushString(&wpName);
            DialogExternal("WLD_INSERTITEM")();
        }
        m.insertedMs = CurrentMs ? CurrentMs : 1;
        CTLog("Markierung (Betreten): Fleischwanze und Apfel an %s in %s (%.0f m von mir)", m.wp.c_str(), m.world.c_str(),
            CoopDist(m.pos, player->GetPositionWorld()) / 100.f);
        return true;
    }

    static int CTMarkFinish(CTMark& m) {
        if (!m.insertedMs)
            return -1;
        if (m.have)
            return 1;
        const int bug = parser->GetIndex(zSTRING(CTBug()));
        oCNpc* fresh = NULL;
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l && !fresh; l = l->next) {
            oCNpc* n = l->data;
            // u49 (automatischer Lauf u49, Schritt 143 auf beiden Seiten FEHLER "tote Fleischwanzen 0"): die Antwortseite toetete die
            // Fleischwanze im Bild nach dem Einsetzen - noch in keinem BSP-Blatt, CoopKillNoXp setzte nur HP 0 ("nicht in der Welt"),
            // ohne DoDie, danach war sie weg. Erst toeten, wenn sie wirklich in der Welt steht (wie FarActiveInWorld)
            if (n && n->GetInstance() == bug && !m.before.count(n) && !n->IsDead() && CoopDist(n->GetPositionWorld(), m.pos) < 1500.f &&
                n->GetHomeWorld() && n->vobLeafList.GetNum() > 0 && n->GetModel())
                fresh = n;
        }
        if (!fresh)
            return CurrentMs > m.insertedMs + 8000 ? -1 : 0;
        CoopKillNoXp(fresh, NULL, "Coop-Test: Markierung fuer das Betreten");
        m.have = true;
        return 1;
    }

    // tote Fleischwanzen bis 6 m und Aepfel bis 3 m um die Markierung
    static void CTCountMark(const zVEC3& pos, int& dead, int& apples) {
        dead = 0;
        apples = 0;
        if (!ogame || !ogame->GetGameWorld() || !parser)
            return;
        const int bug = parser->GetIndex(zSTRING(CTBug()));
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next)
            if (l->data && l->data->GetInstance() == bug && l->data->IsDead() && CoopDist(l->data->GetPositionWorld(), pos) < 600.f)
                dead++;
        apples = CTCountItems(CTApple(), pos, 300.f);
    }

    static void CTSpawn(const char* inst) {
        CTSelfHero self;   // u27
        zSTRING wpName(CT.wpName.c_str());
        DialogPushInt(parser->GetIndex(zSTRING(inst)));
        DialogPushString(&wpName);
        DialogExternal("WLD_INSERTNPC")();
    }

    // u7 (Testlauf u6, Client-Lauf am Aussenring des Alten Lagers): ein frisch eingefuegter Test-NPC, der hier schon tot liegt, ist
    // nicht vom Test getoetet (Wachen/Buddler greifen Monster an) - dann ist der Schritt nicht bewertbar statt FEHLER
    static oCNpc* CTFindDynamicDead(const char* inst) {
        if (CT.wpName.empty())
            return NULL;
        auto up = [](std::string s) { for (auto& c : s) c = (char)toupper((unsigned char)c); return s; };
        const int idx = parser->GetIndex(zSTRING(inst));
        const std::string wp = up("-DYNAMIC-" + CT.wpName + "-");
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (npc && npc->GetInstance() == idx && npc->IsDead() && up(DialogNpcName(npc)).find(wp) != std::string::npos &&
                CoopDist(npc->GetPositionWorld(), CT.haveWp ? CT.wpPos : player->GetPositionWorld()) < 3000.f)   // u40: um den Wegpunkt
                return npc;
        }
        return NULL;
    }

    static bool CTJudgeSpawn(const json& pre, const json& post, const char* what, std::string& name, const char* inst, std::string& d) {
        const int a = CTInt(pre, "count"), b = CTInt(post, "count");
        oCNpc* mine = CTFindDynamic(inst);
        name = mine ? DialogNpcName(mine) : std::string();
        if (!mine) {
            if (oCNpc* dead = CTFindDynamicDead(inst)) {
                d = CTFmt("-- nicht bewertbar: %s liegt hier schon tot (nicht vom Test getoetet - Wachen/Monster in der Naehe?), "
                    "beim Partner %d -> %d", DialogNpcName(dead).c_str(), a, b);
                return false;
            }
        }
        d = CTFmt("%s beim Partner %d -> %d (soll +1), Name hier %s", what, a, b, name.empty() ? "-" : name.c_str());
        return a >= 0 && b == a + 1 && !name.empty();
    }

    // v108 (Test v108: der Test-Molerat - 191 HP, "Schaden Gegner 150 %" - toetete im Host-Lauf den Host-Helden): freundlich,
    // bis ihn der Treffer-Schritt angreift. Dauerhafte Haltung geht ueber den Story-Takt auch zum Partner.
    static void CTCalm(oCNpc* m) {
        if (m && !m->IsDead()) {
            m->SetAttitude(3);      // ATT_FRIENDLY
            m->SetTmpAttitude(3);
        }
    }

    static bool PMolSpawn(std::string& why) { return CTPrepSpawn("MOLERAT", why); }

    static bool AMolSpawn(std::string&) {
        CTSpawn("MOLERAT");
        if (ServerThread)   // beim Host steht er sofort, beim Client erst nach der Meldung des Hosts (dann im Urteil)
            CTCalm(CTFindDynamic("MOLERAT"));
        return true;
    }

    static bool JMolSpawn(const json& pre, const json& post, std::string& d) {
        const bool ok = CTJudgeSpawn(pre, post, "Molerats", CT.molName, "MOLERAT", d);
        if (!CT.molName.empty())
            CTCalm(DialogNpcByName(CT.molName));
        return ok;
    }
    static bool PBugSpawn(std::string& why) { return CTPrepSpawn("MEATBUG", why); }
    static bool ABugSpawn(std::string&) { CTSpawn("MEATBUG"); return true; }
    static bool JBugSpawn(const json& pre, const json& post, std::string& d) { return CTJudgeSpawn(pre, post, "Fleischwanzen", CT.bugName, "MEATBUG", d); }

    static bool PMol(std::string& why) {
        oCNpc* m = CT.molName.empty() ? NULL : DialogNpcByName(CT.molName);
        if (!m || m->IsDead()) {
            why = "kein lebender Test-Molerat aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool PBugAlive(std::string& why) {
        oCNpc* b = CT.bugName.empty() ? NULL : DialogNpcByName(CT.bugName);
        if (!b || b->IsDead()) {
            why = "keine lebende Test-Fleischwanze aus dem Schritt davor";
            return false;
        }
        return true;
    }

    // beim Partner schon tot (z. B. von Wachen getoetet, Test v42) - nicht bewertbar
    static bool CTMolDeadThere(const json& post, std::string& d) {
        if (CTInt(post, "npc", "found") == 1 && CTInt(post, "npc", "dead") == 1) {
            d = "-- nicht bewertbar: " + CT.molName + " beim Partner schon tot";
            return true;
        }
        return false;
    }

    // Test-NPC beim Partner vom Host gefuehrt (Host: gesendet, Client: RemoteNpc mit Modell)
    static bool JMolSync(const json&, const json& post, std::string& d) {
        if (CTMolDeadThere(post, d))
            return false;
        const int found = CTInt(post, "npc", "found"), act = CTInt(post, "npc", "act"), host = CTInt(post, "npc", "host");
        const int sent = CTInt(post, "npc", "sent"), synced = CTInt(post, "npc", "synced");
        const std::string lead = CTStr(post, "npc", "lead");
        if (found == 1 && act == 1 && !lead.empty()) {   // v108: im Kampf von einem Client gefuehrt - dessen PC rechnet ihn
            d = CT.molName + " beim Partner in der Welt, im Kampf gefuehrt von " + lead + " (Kampf-Uebergabe C18)";
            return true;
        }
        if (host == 1)
            d = CTFmt("%s beim Host %s, %s, %s", CT.molName.c_str(), found == 1 ? "da" : "fehlt",
                act == 1 ? "in der Welt" : "nicht in der Welt", sent == 1 ? "wird gesendet" : "wird NICHT gesendet");
        else
            d = CTFmt("%s beim Client %s, %s, %s", CT.molName.c_str(), found == 1 ? "da" : "fehlt",
                act == 1 ? "in der Welt" : "nicht in der Welt", synced == 1 ? "vom Host gefuehrt" : "NICHT vom Host gefuehrt");
        return found == 1 && act == 1 && (host == 1 ? sent == 1 : synced == 1);
    }

    static bool JMolPos(const json&, const json& post, std::string& d) {
        if (CTMolDeadThere(post, d))
            return false;
        oCNpc* m = DialogNpcByName(CT.molName);
        if (!m || CTInt(post, "npc", "found") != 1 || !post["npc"].contains("p")) {
            d = "Test-Molerat fehlt hier oder beim Partner";
            return false;
        }
        const float dist = CoopDist(CTVecOf(post["npc"]["p"]), m->GetPositionWorld());
        d = CTFmt("%s: Lage beim Partner %.0f cm von hier (soll < 500)", CT.molName.c_str(), dist);
        return dist < 500.f;
    }

    static bool JMolHead(const json&, const json& post, std::string& d) {
        if (CTMolDeadThere(post, d))
            return false;
        oCNpc* m = DialogNpcByName(CT.molName);
        const float p = CTFloat(post, "npc", "h");
        if (!m || p < -1e8f) {
            d = "Test-Molerat fehlt hier oder beim Partner";
            return false;
        }
        const float mine = GetHeading(m);
        d = CTFmt("%s: Blickrichtung beim Partner %.0f, hier %.0f Grad (Abweichung %.0f, soll < 25)", CT.molName.c_str(), p, mine, CTAngle(p, mine));
        return CTAngle(p, mine) < 25.f;
    }

    // Leben des Test-NPCs - mit "Leben der Gegner" skaliert der Host das Maximum, der Client uebernimmt es (H16b)
    static bool JMolHpMax(const json&, const json& post, std::string& d) {
        if (CTMolDeadThere(post, d))
            return false;
        oCNpc* m = DialogNpcByName(CT.molName);
        const int p = CTInt(post, "npc", "hm"), ph = CTInt(post, "npc", "hp");
        if (!m || p == -999) {
            d = "Test-Molerat fehlt hier oder beim Partner";
            return false;
        }
        const int mine = m->GetAttribute(NPC_ATR_HITPOINTSMAX), mh = m->GetAttribute(NPC_ATR_HITPOINTS);
        d = CTFmt("%s: Leben beim Partner %d/%d, hier %d/%d (%s)", CT.molName.c_str(), ph, p, mh, mine, CTDiffText().c_str());
        return p == mine && ph == mh;
    }

    // Treffer und Tod laufen ueber den Host, HP und Tod kommen zurueck (C1)
    static bool CTHit(const std::string& name, float dmg, std::string& why) {
        oCNpc* b = DialogNpcByName(name);
        if (!b || b->IsDead()) {
            why = "Test-NPC weg oder schon tot";
            return false;
        }
        b->GetEM(false)->OnDamage(b, player, dmg, 4 /* DAM_EDGE */, b->GetPositionWorld());
        return true;
    }

    static bool AMolHit(std::string& why) { return CTHit(CT.molName, 2.f, why); }        // Othello: mindestens 12

    static bool JMolHit(const json& pre, const json& post, std::string& d) {
        oCNpc* b = DialogNpcByName(CT.molName);
        const int mine = b ? b->GetAttribute(NPC_ATR_HITPOINTS) : -1;
        const int a = CTInt(pre, "npc", "hp"), p = CTInt(post, "npc", "hp");
        d = CTFmt("%s: Partner-HP %d -> %d, hier %d", CT.molName.c_str(), a, p, mine);
        return p >= 0 && p == mine && p < a;
    }

    static bool CTJudgeDead(const json& post, const std::string& name, std::string& d) {
        const int dead = CTInt(post, "npc", "dead"), found = CTInt(post, "npc", "found");
        d = name + " beim Partner " + (found == 1 ? (dead == 1 ? "tot" : "lebt noch") : "nicht gefunden");
        return dead == 1;
    }

    // Toeten mit EP-Regel - weit weg bekommt der Partner keine Kill-EP (I2), in der Naehe teilt ihr sie. v108 (Test v108: 44 m,
    // EP-Reichweite im Schwierigkeitsgrad 20 m): Grenze aus dem Schwierigkeitsgrad (CoopDiffXpRangeCm, -1 = ueberall) statt 45 m
    static bool AMolKill(std::string& why) {
        CT.partnerDist = CTPartnerDist();
        const float range = CoopDiffXpRangeCm();
        CT.farKill = range >= 0.f && CT.partnerDist > range;
        return CTHit(CT.molName, 10000.f, why);
    }

    static bool JMolKill(const json& pre, const json& post, std::string& d) {
        const bool dead = CTJudgeDead(post, CT.molName, d);
        const int a = CTInt(pre, "exp"), b = CTInt(post, "exp");
        const float range = CoopDiffXpRangeCm();
        d += CTFmt("; Abstand %.0f m (%s, EP-Reichweite %s), Partner-EP %d -> %d (soll %s)", CT.partnerDist / 100.f, CT.farKill ? "fern" : "nah",
            range < 0.f ? "ueberall" : CTFmt("%.0f m", range / 100.f).c_str(), a, b, CT.farKill ? "gleich" : "mehr");
        const bool xpOk = a != -999 && b != -999 && (CT.farKill ? b == a : b > a);
        return dead && xpOk;
    }

    // tot bleibt tot - kein Wiederaufstehen mit HP 1 (I3)
    static bool PMolDead(std::string& why) {
        if (CT.molName.empty()) {
            why = "kein Test-Molerat";
            return false;
        }
        return true;
    }

    static bool JMolStillDead(const json&, const json& post, std::string& d) {
        const int dead = CTInt(post, "npc", "dead"), hp = CTInt(post, "npc", "hp"), found = CTInt(post, "npc", "found");
        d = CTFmt("%s beim Partner 5 s spaeter %s (HP %d)", CT.molName.c_str(), found == 1 ? (dead == 1 ? "tot" : "LEBT") : "nicht mehr da", hp);
        return found != 1 || dead == 1;   // weggeraeumt zaehlt als tot
    }

    // Haltung der Test-Fleischwanze (Story-Takt D9): freundlich, dann neutral
    static bool CTSetBugAtt(int att) {
        oCNpc* m = DialogNpcByName(CT.bugName);
        m->SetAttitude(att);
        m->SetTmpAttitude(att);
        return true;
    }

    static bool ABugFriendly(std::string&) { return CTSetBugAtt(3); }   // ATT_FRIENDLY
    static bool ABugNeutral(std::string&) { return CTSetBugAtt(2); }    // ATT_NEUTRAL

    static bool CTJudgeBugAtt(const json& post, int want, std::string& d) {
        const int a = CTInt(post, "npc", "att");
        d = CTFmt("%s beim Partner Haltung %d (soll %d)", CT.bugName.c_str(), a, want);
        return a == want;
    }

    static bool JBugFriendly(const json&, const json& post, std::string& d) { return CTJudgeBugAtt(post, 3, d); }
    static bool JBugNeutral(const json&, const json& post, std::string& d) { return CTJudgeBugAtt(post, 2, d); }

    // u139 (P138): Skriptfelder der Test-Fleischwanze. Erst unsterblich im Dialog-Weg (DIA_DRAGON_*_EXIT in der Gegenrichtung), dann im Takt
    // wieder verwundbar mit fremder Gilde, Typ und ohne Fokus, dann zurueck - sonst waere "Fleischwanze toeten" beim Partner wirkungslos
    static CoopNpcFields CTBugOld;

    static bool CTJudgeBugFields(const json& post, const CoopNpcFields& want, std::string& d) {
        const CoopNpcFields got = { CTInt(post, "npc", "fl"), CTInt(post, "npc", "gu"), CTInt(post, "npc", "gt"), CTInt(post, "npc", "nt"), CTInt(post, "npc", "nf") };
        d = CTFmt("%s beim Partner: Schutz %d (soll %d), Gilde %d (%d), wahre Gilde %d (%d), Typ %d (%d), ohne Fokus %d (%d)", CT.bugName.c_str(), got.flags, want.flags,
            got.guild, want.guild, got.guildTrue, want.guildTrue, got.npcType, want.npcType, got.noFocus, want.noFocus);
        return DlgNpcFieldsSame(got, want);
    }

    static bool ABugImmortal(std::string&) {
        oCNpc* m = DialogNpcByName(CT.bugName);
        CTBugOld = DlgNpcFields(m);
        CoopDlgSnap a, b;
        DlgSnapshot(a);
        m->variousFlags = CTBugOld.flags | NPC_FLAG_IMMORTAL;
        DlgSnapshot(b);
        DlgSendDiff(a, b, std::string(), "Coop-Test");
        StoryAbsorb(a, b);
        return true;
    }

    static bool JBugImmortal(const json&, const json& post, std::string& d) {
        CoopNpcFields want = CTBugOld;
        want.flags |= NPC_FLAG_IMMORTAL;
        return CTJudgeBugFields(post, want, d);
    }

    static CoopNpcFields CTBugChanged() {
        CoopNpcFields f = CTBugOld;
        f.flags &= ~NPC_FLAG_IMMORTAL;
        f.guild = CTBugOld.guild + 1;
        f.guildTrue = CTBugOld.guildTrue + 1;
        f.npcType = CTBugOld.npcType == 3 ? 0 : 3;   // NPCTYPE_FRIEND
#if ENGINE == Engine_G2A
        f.noFocus = CTBugOld.noFocus ? 0 : 1;
#endif
        return f;
    }

    static void CTSetBugFields(const CoopNpcFields& f) {
        oCNpc* m = DialogNpcByName(CT.bugName);
        m->variousFlags = f.flags;
        m->guild = f.guild;
        m->SetTrueGuild(f.guildTrue);
        m->npcType = f.npcType;
#if ENGINE == Engine_G2A
        m->noFocus = f.noFocus;
#endif
    }

    static bool ABugFieldsTick(std::string&) { CTSetBugFields(CTBugChanged()); return true; }
    static bool JBugFieldsTick(const json&, const json& post, std::string& d) { return CTJudgeBugFields(post, CTBugChanged(), d); }
    static bool ABugFieldsBack(std::string&) { CTSetBugFields(CTBugOld); return true; }
    static bool JBugFieldsBack(const json&, const json& post, std::string& d) { return CTJudgeBugFields(post, CTBugOld, d); }

    // v108 (Test v108: mit "Leben der Gegner 150 %" hat sie 8 HP - ein Schlag mit Mindestschaden reichte nicht mehr): toedlicher Schlag
    static bool ABugKill(std::string& why) { return CTHit(CT.bugName, 10000.f, why); }
    static bool JBugKill(const json&, const json& post, std::string& d) { return CTJudgeDead(post, CT.bugName, d); }

    static bool JDigDead(const json&, const json& post, std::string& d) { return CTDigJudge(post, "NPCs tot/lebendig (1/0)", 0, d); }

    // Trigger/Untrigger an einen Namen ohne Vob (tut nichts) - der Client fragt den Host, der Host fuehrt aus und verteilt (D13)
    static bool PTrig(std::string& why) {
        if (!DialogExternal("WLD_SENDTRIGGER") || !DialogExternal("WLD_SENDUNTRIGGER")) {
            why = "WLD_SENDTRIGGER fehlt";
            return false;
        }
        return true;
    }

    static bool CTSendTrig(const char* ext) {
        CTSelfHero self;   // u27
        zSTRING s("COOP_TEST_TRIGGER");
        DialogPushString(&s);
        DialogExternal(ext)();
        return true;
    }

    static bool ATrig(std::string&) { return CTSendTrig("WLD_SENDTRIGGER"); }
    static bool AUntrig(std::string&) { return CTSendTrig("WLD_SENDUNTRIGGER"); }

    static bool CTJudgeTrig(const json& pre, const json& post, const char* want, std::string& d) {
        const int a = CTInt(pre, "trig", "n"), b = CTInt(post, "trig", "n");
        const std::string last = CTStr(post, "trig", "last");
        d = CTFmt("Host-Ereignisse beim Partner %d -> %d, zuletzt \"%s\" (soll \"%s\")", a, b, last.c_str(), want);
        return a >= 0 && b > a && last == want;
    }

    static bool JTrig(const json& pre, const json& post, std::string& d) { return CTJudgeTrig(pre, post, "trigger:COOP_TEST_TRIGGER", d); }
    static bool JUntrig(const json& pre, const json& post, std::string& d) { return CTJudgeTrig(pre, post, "untrigger:COOP_TEST_TRIGGER", d); }

    // Skript legt einen Apfel in die Welt (Wld_InsertItem). Bis u26 bekannte Luecke (nur lokal); u27 (D13d) ueber den Host - der
    // Apfel muss hier UND beim Partner liegen.
    static bool PInsertItem(std::string& why) {
        if (CT.wpName.empty() || !DialogExternal("WLD_INSERTITEM")) {
            why = "kein Wegpunkt / WLD_INSERTITEM";
            return false;
        }
        CT.itemsBefore = CTCountItems(CTApple(), player->GetPositionWorld(), 2500.f);
        return true;
    }

    static bool AInsertItem(std::string&) {
        CTSelfHero self;   // u27
        zSTRING wpName(CT.wpName.c_str());
        DialogPushInt(parser->GetIndex(zSTRING(CTApple())));
        DialogPushString(&wpName);
        DialogExternal("WLD_INSERTITEM")();
        return true;
    }

    static bool JInsertItem(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "items"), b = CTInt(post, "items");
        const int mine = CTCountItems(CTApple(), player->GetPositionWorld(), 2500.f);
        d = CTFmt("Aepfel am Boden beim Partner %d -> %d, hier %d -> %d (je soll +1)", a, b, CT.itemsBefore, mine);
        return a >= 0 && b == a + 1 && CT.itemsBefore >= 0 && mine == CT.itemsBefore + 1;
    }

    // Kampf-Uebergabe (C18): Host und Client sind sich einig, welche NPCs der Client gerade fuehrt
    static bool JLead(const json&, const json& post, std::string& d) {
#if !COOP_READY_CombatLead
        CT_MODULE_OFF_JUDGE("Kampf-Uebergabe")
#else
        if (!post.contains("lead") || !post["lead"].contains("l")) {
            d = "Partner meldet keine Kampf-Uebergabe";
            return false;
        }
        std::set<std::string> mine, theirs;
        const json& l = post["lead"]["l"];
        if (ServerThread) {
            for (auto& kv : CoopLeadsHost)
                if (kv.second.owner == CT.partner)
                    mine.insert(kv.first);
            for (auto& x : l)
                if (x.is_string())
                    theirs.insert(x.get<std::string>());
        }
        else {
            for (auto& kv : CoopLeadsMine)
                mine.insert(kv.first);
            for (auto& x : l)
                if (x.is_array() && x.size() == 2 && x[1].get<std::string>() == DialogMe())
                    theirs.insert(x[0].get<std::string>());
        }
        std::string diff;
        for (auto& n : mine)
            if (!theirs.count(n))
                diff += (diff.empty() ? "" : ", ") + n + " nur hier";
        for (auto& n : theirs)
            if (!mine.count(n))
                diff += (diff.empty() ? "" : ", ") + n + " nur beim Partner";
        d = CTFmt("vom Client gefuehrt: hier %u, beim Partner %u%s%s", (unsigned)mine.size(), (unsigned)theirs.size(),
            diff.empty() ? " - einig" : " - abweichend: ", diff.c_str());
        return diff.empty();
#endif
    }

    // =====================================================================================================================
    // Bereich 5: NPCs um mich
    // =====================================================================================================================

    // Verfolgungspunkt - meine Figur ist beim Partner in der Welt, die NPCs um mich laufen dort (I1)
    static bool AFar(std::string&) {
        CT.myActive = CTCountActive(player->GetPositionWorld(), (float)BROADCAST_DISTANCE);
        return true;
    }

    static bool JFar(const json&, const json& post, std::string& d) {
        const int fol = CTInt(post, "far", "fol"), fig = CTInt(post, "far", "fig"), figact = CTInt(post, "far", "figact");
        const int act = CTInt(post, "far", "act");
        const float dist = CTFloat(post, "far", "dist");
        d = CTFmt("Abstand %.0f m (%s), Verfolgungspunkt beim Partner %s, meine Figur %s, NPCs um mich: hier %d aktiv, beim Partner %d",
            dist / 100.f, dist > (float)BROADCAST_DISTANCE ? "fern" : "nah", fol == 1 ? "an" : "aus",
            fig == 1 ? (figact == 1 ? "in der Welt" : "da, aber nicht in der Welt") : "fehlt", CT.myActive, act);
        if (fig != 1 || figact != 1)
            return false;
        if (dist > (float)BROADCAST_DISTANCE)   // fern: Verfolgungspunkt muss an sein und NPCs um mich aktiv halten
            return fol == 1 && (CT.myActive <= 0 || act > 0);
        return true;
    }

    struct CTNearPair {
        oCNpc* npc;
        std::string name;
        const json* their;
    };

    // eigene NPCs bis 20 m (lebend in der Welt, mit withDead auch tote) mit dem Eintrag beim Partner; Rueckgabe: Zahl hier, -1 ohne Liste
    static int CTNearPairs(const json& post, bool withDead, std::vector<CTNearPair>& out, int& missing) {
        out.clear();
        missing = 0;
        if (!post.contains("near") || !post["near"].contains("l"))
            return -1;
        std::map<std::string, const json*> theirs;
        for (auto& x : post["near"]["l"])
            theirs[x.value("n", std::string())] = &x;
        int here = 0;
        for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l; l = l->next) {
            oCNpc* npc = l->data;
            if (!npc || npc == player || PlayerNpcs.count(npc))
                continue;
            if (!CTNpcPlaced(npc) || (!withDead && npc->IsDead()))   // u51 (Befund 6): auch mit Toten nur eingesetzte
                continue;
            auto nm = NpcToUniqueNameList.find(npc);
            if (nm == NpcToUniqueNameList.end() || CoopDist(npc->GetPositionWorld(), player->GetPositionWorld()) >= CT_NEAR_R)
                continue;
            here++;
            auto f = theirs.find(std::string(nm->second.ToChar()));
            if (f == theirs.end() || (!withDead && (f->second->value("a", 0) != 1 || f->second->value("d", 0) == 1))) {
                missing++;
                continue;
            }
            out.push_back({ npc, std::string(nm->second.ToChar()), f->second });
        }
        return here;
    }

    // NPCs um mich beim Partner - gleich benannt, Lage, KI-Zustand, abgeglichen (Host: gesendet, Client: vom Host gefuehrt)
    static bool JNear(const json&, const json& post, std::string& d) {
        std::vector<CTNearPair> pairs;
        int missing = 0;
        const int here = CTNearPairs(post, false, pairs, missing);
        if (here < 0) {
            d = "Partner meldet keine NPC-Liste";
            return false;
        }
        if (here == 0) {
            d = "-- keine NPCs in 20 m um mich";
            return false;
        }
        const bool partnerHost = post["near"].value("host", 0) == 1;
        int led = 0, partnerAlive = 0;
        for (auto& x : post["near"]["l"])
            if (x.value("d", 0) == 0 && x.value("a", 0) == 1)
                partnerAlive++;
        // v108 (Test v108: 3 von 4 = 75 % - der vierte stand beim Partner zwischen 20 und 25 m): abgeglichen nur unter den gepaarten;
        // vom Client im Kampf gefuehrte zaehlen beim Host als abgeglichen (der Client sendet sie)
        for (auto& p : pairs)
            if (p.their->value("l", 0) == 1 || (partnerHost && CoopLeadIsMine(p.npc)))
                led++;
        std::vector<float> devs;
        std::vector<std::pair<float, std::string>> rows;
        for (auto& p : pairs) {
            const float dev = CoopDist(CTVecOf((*p.their)["p"]), p.npc->GetPositionWorld());
            devs.push_back(dev);
            rows.push_back({ dev, p.name + ": " + std::to_string((int)dev) + " cm - hier " + p.npc->state.curState.name.ToChar() +
                ", beim Partner " + p.their->value("s", std::string()) + (p.their->value("l", 0) == 1 ? "" : (partnerHost ? ", nicht gesendet" : ", NICHT gefuehrt")) });
        }
        std::sort(devs.begin(), devs.end());
        const float median = devs.empty() ? -1.f : devs[devs.size() / 2];
        const float worst = devs.empty() ? -1.f : devs.back();
        const float dist = CTPartnerDist();
        d = CTFmt("Abstand %.0f m (%s): hier %d NPCs in 20 m, beim Partner %d in der Welt (%s %d); fehlen/nicht in der Welt %d; "
            "Lage-Abweichung Mitte %.0f cm, groesste %.0f cm", dist / 100.f, dist > (float)BROADCAST_DISTANCE ? "fern" : "nah", here,
            partnerAlive, partnerHost ? "gesendet" : "vom Host gefuehrt", led, missing, median, worst);
        std::sort(rows.begin(), rows.end(), [](const std::pair<float, std::string>& a, const std::pair<float, std::string>& b) { return a.first > b.first; });
        for (size_t i = 0; i < rows.size() && i < 8; i++)
            CTLog("  NPC %s", rows[i].second.c_str());
        // v108 (Test v107: "fehlen 1" ohne Namen): die fehlenden mit Zustand beim Partner
        if (missing) {
            std::set<std::string> paired;
            for (auto& p : pairs)
                paired.insert(p.name);
            int logged = 0;
            for (auto* l = CoopWorldNpcListFirst(ogame->GetGameWorld()); l && logged < 8; l = l->next) {
                oCNpc* npc = l->data;
                if (!npc || npc == player || PlayerNpcs.count(npc) || npc->IsDead() || npc->vobLeafList.GetNum() == 0)
                    continue;
                auto nm = NpcToUniqueNameList.find(npc);
                if (nm == NpcToUniqueNameList.end() || CoopDist(npc->GetPositionWorld(), player->GetPositionWorld()) >= CT_NEAR_R)
                    continue;
                const std::string name = nm->second.ToChar();
                if (paired.count(name))
                    continue;
                std::string there = "fehlt";
                for (auto& x : post["near"]["l"])
                    if (x.value("n", std::string()) == name)
                        there = x.value("d", 0) == 1 ? "tot" : x.value("a", 0) == 1 ? "?" : "nicht in der Welt";
                CTLog("  NPC %s fehlt beim Partner (%s) - hier %s", name.c_str(), there.c_str(), npc->state.curState.name.ToChar());
                logged++;
            }
        }
        const bool ledOk = pairs.empty() || led * 5 >= (int)pairs.size() * 4;   // mind. 80 % abgeglichen
        return missing * 5 <= here && median >= 0 && median < 300.f && ledOk;
    }

    // Anteil passender NPCs um mich - eine Eigenschaft je Schritt
    static bool CTNearShare(const json& post, bool withDead, const char* what, float need,
        bool (*same)(const CTNearPair& p, std::string& ex), std::string& d) {
        std::vector<CTNearPair> pairs;
        int missing = 0;
        const int here = CTNearPairs(post, withDead, pairs, missing);
        if (here < 0) {
            d = "Partner meldet keine NPC-Liste";
            return false;
        }
        if (pairs.empty()) {
            d = CTFmt("-- keine gemeinsamen NPCs in 20 m (hier %d, beim Partner fehlen %d)", here, missing);
            return false;
        }
        int good = 0, logged = 0;
        for (auto& p : pairs) {
            std::string ex;
            if (same(p, ex))
                good++;
            else if (logged++ < 8)
                CTLog("  %s: %s", p.name.c_str(), ex.c_str());
        }
        d = CTFmt("%s: %d von %d gleich (soll mind. %.0f %%)", what, good, (int)pairs.size(), need * 100.f);
        return good >= (int)ceilf(need * (float)pairs.size());
    }

    static bool CTSameHp(const CTNearPair& p, std::string& ex) {
        const int mine = p.npc->GetAttribute(NPC_ATR_HITPOINTS), theirs = p.their->value("hp", -999);
        ex = CTFmt("HP hier %d, Partner %d", mine, theirs);
        return mine == theirs;
    }

    static bool CTSameWm(const CTNearPair& p, std::string& ex) {
        const int mine = p.npc->GetWeaponMode(), theirs = p.their->value("wm", -999);
        ex = CTFmt("Waffenmodus hier %d, Partner %d", mine, theirs);
        return mine == theirs;
    }

    static bool CTSameHead(const CTNearPair& p, std::string& ex) {
        const float mine = GetHeading(p.npc), theirs = p.their->value("h", -1e9f);
        ex = CTFmt("Blickrichtung hier %.0f, Partner %.0f Grad", mine, theirs);
        return theirs > -1e8f && CTAngle(mine, theirs) <= 30.f;
    }

    static bool CTSameAni(const CTNearPair& p, std::string& ex) {
        const json mine = CTAnis(p.npc);
        const json theirs = p.their->value("an", json::array());
        ex = "Animationen hier " + mine.dump() + ", Partner " + theirs.dump();
        for (auto& a : mine)
            if (CTHas(theirs, a.get<std::string>()))
                return true;
        return false;
    }

    static bool CTSameDead(const CTNearPair& p, std::string& ex) {
        const int mine = p.npc->IsDead() ? 1 : 0, theirs = p.their->value("d", -1);
        ex = CTFmt("hier %s, beim Partner %s", mine ? "tot" : "lebt", theirs == 1 ? "tot" : theirs == 0 ? "lebt" : "?");
        return mine == theirs;
    }

    static bool JNearHp(const json&, const json& post, std::string& d) { return CTNearShare(post, false, "Lebenspunkte", 0.9f, CTSameHp, d); }
    static bool JNearWm(const json&, const json& post, std::string& d) { return CTNearShare(post, false, "Waffenmodus", 0.9f, CTSameWm, d); }
    static bool JNearHead(const json&, const json& post, std::string& d) { return CTNearShare(post, false, "Blickrichtung (bis 30 Grad)", 0.8f, CTSameHead, d); }
    static bool JNearAni(const json&, const json& post, std::string& d) { return CTNearShare(post, false, "laufende Animation", 0.6f, CTSameAni, d); }
    static bool JNearDead(const json&, const json& post, std::string& d) { return CTNearShare(post, true, "tot/lebendig", 1.f, CTSameDead, d); }

    // =====================================================================================================================
    // Bereich 6: Gegenstaende und Truhen
    // =====================================================================================================================

    static bool PWorld(std::string& why) {
        if (!CoopRuleWorld()) {
            why = "Abgleich Gegenstaende/Truhen aus";
            return false;
        }
        return true;
    }

    // Truhe: Apfel hinein / heraus, einzeln und drei (Truhen-Haken wie Quickloot und Skripte, E1/E3)
    static bool PChest(std::string& why) {
        if (!PWorld(why))
            return false;
        CT.chest = CTNearestChest(player->GetPositionWorld(), 5000.f);   // v108: 50 m statt 10 m (Tests v107/v108: keine Truhe in der Naehe)
        if (!CT.chest) {
            why = "keine Truhe in 50 m";
            return false;
        }
        CT.chestPos = CT.chest->GetPositionWorld();
        return true;
    }

    static bool PChestAgain(std::string& why) {
        if (!PWorld(why))
            return false;
        if (!CT.chest || FindChest(CT.chestPos) != CT.chest) {
            why = "keine Truhe aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static bool CTChestInsert(int n, std::string& why) {
        oCItem* it = CreateCoopItem(parser->GetIndex(zSTRING(CTApple())));
        if (!it) {
            why = "Apfel nicht erzeugt";
            return false;
        }
        it->amount = n;
        CT.chest->Insert(it);
        it->Release();
        return true;
    }

    // Herausnehmen ueber oCMobContainer::Remove wie Quickloot/Skripte - die GT-Haken sollen es sehen. u98 (P120): Remove legt den
    // Gegenstand in die Listen der Welt; ein echter Aufrufer legt ihn danach ins Inventar, hier niemand - darum gleich aus der Welt
    // (RemoveVob, wie nach RemoveFromInv). Remove(item, n) nicht: es haengt laut Disassembly (G2A 0x00726080, G1 0x00683F40) den
    // ganzen Gegenstand aus und liefert 0 - beim groesseren Stapel darum ganz heraus und den Rest neu hinein.
    static bool CTChestRemove(int n, std::string& why) {
        for (zCListSort<oCItem>* l = CT.chest->containList.next; l; l = l->next)
            if (l->data && CoopItemInst(l->data) == CTApple() && CoopItemAmount(l->data) >= n) {
                oCItem* it = l->data;
                const int rest = CoopItemAmount(it) - n;
                CT.chest->Remove(it);
                ogame->GetGameWorld()->RemoveVob(it);
                if (rest > 0 && !CTChestInsert(rest, why))
                    return false;
                return true;
            }
        why = CTFmt("keine %d Aepfel in der Truhe", n);
        return false;
    }

    static bool AChestIn1(std::string& why) { return CTChestInsert(1, why); }
    static bool AChestOut1(std::string& why) { return CTChestRemove(1, why); }
    static bool AChestIn3(std::string& why) { return CTChestInsert(3, why); }
    static bool AChestOut3(std::string& why) { return CTChestRemove(3, why); }

    static bool CTJudgeChest(const json& pre, const json& post, int delta, std::string& d) {
        const int a = CTInt(pre, "chest", "n"), b = CTInt(post, "chest", "n");
        d = CTFmt("Aepfel in der Truhe beim Partner %d -> %d (soll %+d)", a, b, delta);
        return a >= 0 && b == a + delta;
    }

    static bool JChestIn1(const json& pre, const json& post, std::string& d) { return CTJudgeChest(pre, post, 1, d); }
    static bool JChestOut1(const json& pre, const json& post, std::string& d) { return CTJudgeChest(pre, post, -1, d); }
    static bool JChestIn3(const json& pre, const json& post, std::string& d) { return CTJudgeChest(pre, post, 3, d); }
    static bool JChestOut3(const json& pre, const json& post, std::string& d) { return CTJudgeChest(pre, post, -3, d); }

    static bool JChestAll(const json&, const json& post, std::string& d) {
        if (CTInt(post, "chest", "found") != 1) {
            d = "Partner findet die Truhe nicht";
            return false;
        }
        const std::string p = CTStr(post, "chest", "all"), mine = BestandText(ChestBestand(CT.chest));
        d = "Inhalt beim Partner \"" + p + "\", hier \"" + mine + "\"";
        if (d.size() > 400)
            d = d.substr(0, 400) + "...";
        return p == mine;
    }

    // --- u137 (P134): Mechanismen -------------------------------------------------------------------------------------
    // Ein echter Hebel: Mob mit Ziel, dahinter genau ein Mover, der hin und her kann (2STATE_TOGGLE, beliebig oft), ohne Skript und Gegenstand. Der am
    // weitesten vom Helden - ein Gefaengnistor, das sich dort bewegt, stoert niemanden. OnEndStateChange wie die Engine am Ende der Bewegung; das
    // laeuft durch den Haken und schickt die Benutzung. Vier Schritte: an (Tor geht auf), aus, an (Tor geht zu), aus - danach ist alles wie vorher.
    static oCMobInter* CTMob = NULL;
    static zVEC3 CTMobPos;
    static std::string CTMobZiel;
    static int CTMobScriptsWas = 0, CTMobCamsWas = 0;

    static bool PMobNew(std::string& why) {
        CTMob = NULL;
        if (!CoopRuleWorld() && !CoopRuleQuests()) {
            why = "Abgleich Quests und Gegenstaende/Truhen aus";
            return false;
        }
        oCWorld* w = ogame->GetGameWorld();
        zCArray<zCVob*> mobs;
        w->SearchVobListByBaseClass(oCMobInter::classDef, mobs, NULL);
        float best = -1.f;
        for (int i = 0; i < mobs.GetNumInList(); i++) {
            oCMobInter* m = zDYNAMIC_CAST<oCMobInter>(mobs.GetSafe(i));
            if (!m || m->triggerTarget.IsEmpty() || !m->useWithItem.IsEmpty() || !m->onStateFuncName.IsEmpty() || !m->conditionFunc.IsEmpty() || m->rewind ||
                m->state != 0 || m->npcsCurrent > 0 || !MobSyncRelevant(m) || zDYNAMIC_CAST<oCMobDoor>(m))
                continue;
            zCArray<zCVob*> ziele;
            w->SearchVobListByName(m->triggerTarget, ziele);
            zCMover* mv = ziele.GetNumInList() == 1 ? zDYNAMIC_CAST<zCMover>(ziele.GetSafe(0)) : NULL;
            if (!mv || mv->moverBehavior != zCMover::MB_2STATE_TOGGLE || mv->moverState != zCMover::MOVER_STATE_CLOSED || mv->numCanBeActivated >= 0 || !mv->triggerTarget.IsEmpty())
                continue;
            const float d = CoopDist(m->GetPositionWorld(), player->GetPositionWorld());
            if (d > best) {
                best = d;
                CTMob = m;
                CTMobPos = m->GetPositionWorld();
                CTMobZiel = m->triggerTarget.ToChar();
            }
        }
        if (!CTMob) {
            why = "kein Hebel mit einfachem Tor in dieser Welt";
            return false;
        }
        return true;
    }

    static bool PMobAgain(std::string& why) {
        if (!CTMob || FindMob(CTMobPos) != CTMob) {
            why = "kein Hebel aus dem Schritt davor";
            return false;
        }
        return true;
    }

    static json QMob() { json q; q["mob"]["p"] = CTVec(CTMobPos); q["mob"]["z"] = CTMobZiel; return q; }

    static bool CTMobUse(int from, int to) {
        CTMob->state_target = to;
        CTMob->OnEndStateChange(player, from, to);
        CTMob->state = to;
        return true;
    }

    static bool AMobOn(std::string&) { return CTMobUse(0, 1); }
    static bool AMobOff(std::string&) { return CTMobUse(1, 0); }

    static bool CTJudgeMob(const json& pre, const json& post, int s, std::string& d, int ms1, int ms2) {
        if (CTInt(post, "mob", "found") != 1) {
            d = "Partner findet den Hebel nicht";
            return false;
        }
        const int a = CTInt(pre, "mob", "s"), b = CTInt(post, "mob", "s"), m0 = CTInt(pre, "mob", "ms"), m1 = CTInt(post, "mob", "ms");
        d = CTFmt("%s: beim Partner Hebel %d -> %d, Tor %d -> %d (0 offen, 1 geht auf, 2 zu, 3 geht zu)", CTMobZiel.c_str(), a, b, m0, m1);
        return b == s && (ms1 < 0 || m1 == ms1 || m1 == ms2);
    }

    static bool JMobOn(const json& pre, const json& post, std::string& d) { return CTJudgeMob(pre, post, 1, d, zCMover::MOVER_STATE_OPENING, zCMover::MOVER_STATE_OPEN); }
    static bool JMobOff(const json& pre, const json& post, std::string& d) { return CTJudgeMob(pre, post, 0, d, -1, -1); }
    static bool JMobOn2(const json& pre, const json& post, std::string& d) { return CTJudgeMob(pre, post, 1, d, zCMover::MOVER_STATE_CLOSING, zCMover::MOVER_STATE_CLOSED); }

    // Skript und Kamera in der Kette: am Portal in NEWWORLD (G2A), dessen Skript B_ADDON_PORTAL_ACTIVATED leer ist. Ohne Haken lief hier die
    // Kamerafahrt, sonst geschieht nichts
    static oCTriggerScript* CTChainScript = NULL;
    static zCCSCamera* CTChainCam = NULL;

    static bool PMobChain(std::string& why) {
        oCWorld* w = ogame->GetGameWorld();
        CTChainScript = zDYNAMIC_CAST<oCTriggerScript>(w->SearchVobByName(zSTRING("EVT_ADDON_TROLLPORTAL_TRIGGERSCRIPT_01")));
        CTChainCam = zDYNAMIC_CAST<zCCSCamera>(w->SearchVobByName(zSTRING("EVT_ADDON_TROLLPORTAL_CAMERA_01")));
        if (!CTChainScript || !CTChainCam || !CTChainScript->triggerTarget.IsEmpty()) {
            why = "nur in NEWWORLD (Portal in Khorinis)";
            return false;
        }
        return true;
    }

    static bool AMobChain(std::string&) {
        CTMobScriptsWas = CoopMobSkipScripts;
        CTMobCamsWas = CoopMobSkipCams;
        const long long was = MobChainUntilMs;
        MobChainUntilMs = GetCurrentMs() + 1000;
        CTChainScript->TriggerTarget(NULL);
        CTChainCam->OnTrigger(NULL, NULL);
        CTChainCam->OnUntrigger(NULL, NULL);
        MobChainUntilMs = was;
        return true;
    }

    static bool JMobChain(const json&, const json&, std::string& d) {
        const int s = CoopMobSkipScripts - CTMobScriptsWas, c = CoopMobSkipCams - CTMobCamsWas;
        d = CTFmt("Skript %d-mal, Kamera %d-mal uebergangen (soll 1 und 2)", s, c);
        return s == 1 && c == 2;
    }

    // u138 (P136): gemeinsam speichern, waehrend eine Kamerafahrt laeuft, dann gemeinsam laden. Ohne den Haken auf zCCSCamera::Unarchive stuerzt das Spiel im ersten Bild nach dem Laden ab
    // (zCKBSpline::Interpolate0 aus zCCSCamera::OnTick). Die laengste Fahrt der Welt, damit sie beim Speichern noch laeuft; das Laden beendet sie.
    static std::string CTCamName;
    static int CTCamStoppedWas = 0;
    static bool PSave(std::string& why);
    static bool PAfterLoad(std::string& why);

    static bool PCamSave(std::string& why) {
        CTCamName.clear();
        if (!PSave(why))
            return false;
        zCArray<zCVob*> cams;
        ogame->GetGameWorld()->SearchVobListByClass(zCCSCamera::classDef, cams, NULL);
        float best = 0.f;
        for (int i = 0; i < cams.GetNumInList(); i++) {
            zCCSCamera* c = zDYNAMIC_CAST<zCCSCamera>(cams.GetSafe(i));
            if (c && !c->started && c->posKeys.GetNumInList() >= 2 && c->totalTime > best && !c->GetObjectName().IsEmpty()) {
                best = c->totalTime;
                CTCamName = c->GetObjectName().ToChar();
            }
        }
        if (CTCamName.empty()) {
            why = "keine Kamerafahrt in dieser Welt";
            return false;
        }
        return true;
    }

    static zCCSCamera* CTCamByName() {
        return CTCamName.empty() ? NULL : zDYNAMIC_CAST<zCCSCamera>(ogame->GetGameWorld()->SearchVobByName(zSTRING(CTCamName.c_str())));
    }

    static bool ACamSave(std::string& why) {
        zCCSCamera* c = CTCamByName();
        if (!c) {
            why = "Kamerafahrt nicht gefunden";
            return false;
        }
        CTCamStoppedWas = CoopCamStopped;
        c->GetEM()->OnTrigger(c, player);
        return true;
    }

    static bool JCamSave(const json&, const json&, std::string& d) {
        zCCSCamera* c = CTCamByName();
        d = CTFmt("%s, %.0f s: laeuft %d", CTCamName.c_str(), c ? c->totalTime : 0.f, c ? (int)c->started : -1);
        return c && c->started;
    }

    static bool PCamLoad(std::string& why) {
        if (!PAfterLoad(why))
            return false;
        if (CTCamName.empty()) {
            why = "keine Kamerafahrt beim Speichern";
            return false;
        }
        return true;
    }

    // Der Client uebernimmt beim gemeinsamen Laden die Welt des Hosts: In seinem Lauf steht dort keine laufende Kamerafahrt (die Fahrt fehlt dann, wenn der Host woanders ist);
    // im Lauf des Hosts muss sie beim Laden gestoppt worden sein
    static bool JCamLoad(const json&, const json&, std::string& d) {
        zCCSCamera* c = CTCamByName();
        zCAICamera* ai = zCAICamera::GetCurrent();
        const int stopped = CoopCamStopped - CTCamStoppedWas;
        d = CTFmt("%s: laeuft %d (-1 nicht in dieser Welt); beim Laden gestoppt %d; zCCSCamera::playing %d, Held beweglich %d", CTCamName.c_str(), c ? (int)c->started : -1, stopped,
            (int)zCCSCamera::playing, ai ? (int)ai->playerIsMovable : -1);
        return (!c || !c->started) && !zCCSCamera::playing && ai && ai->playerIsMovable && (!ServerThread || stopped > 0);
    }

    static json QDigMobs() { return CTDigQuery("mobs"); }
    static bool JDigMobs(const json&, const json& post, std::string& d) { return CTDigJudge(post, "alle Mechanismen (Zustand, Schloss)", 1, d); }

    static bool JDigChests(const json&, const json& post, std::string& d) { return CTDigJudge(post, "alle Truhen (Inhalt)", 1, d); }
    static bool JDigLoose(const json&, const json& post, std::string& d) { return CTDigJudge(post, "Gegenstaende am Boden in 20 m (Menge je Art)", 1, d); }

    // Aufheben (E2): naechster freier Gegenstand ohne Besitzer in 20 m (v108, vorher 8 m) - PutInInv wie die Engine beim Aufheben
    // (DisableVob + Inventar, GTs Haken melden ITEM_TAKE/ITEM_GONE); er verschwindet beim Partner
    static bool PPick(std::string& why) {
        if (!PWorld(why))
            return false;
        zCArray<zCVob*> vobs = CollectVobsInRadius(player->GetPositionWorld(), 2000);
        oCItem* best = NULL;
        float bestD = 2001.f;
        for (int i = 0; i < vobs.GetNumInList(); i++) {
            oCItem* it = zDYNAMIC_CAST<oCItem>(vobs.GetSafe(i));
            if (!it || it->owner || it->ownerGuild || CoopItemIsDropped(it) || !CoopItemLiegtFrei(it) || it->GetInstance() == CTRuneInst())
                continue;
            const float d = CoopDist(it->GetPositionWorld(), player->GetPositionWorld());
            if (d < bestD) {
                bestD = d;
                best = it;
            }
        }
        if (!best) {
            why = "kein freier Gegenstand ohne Besitzer in 20 m";
            return false;
        }
        CT.pickInst = CoopItemInst(best);
        CT.pickPos = best->GetPositionWorld();
        return true;
    }

    static bool APick(std::string& why) {
        oCItem* it = NULL;
        if (CTCountItemVobs(CT.pickInst, CT.pickPos, 60.f, &it) <= 0 || !it) {
            why = CT.pickInst + " inzwischen weg";
            return false;
        }
        if (player->GetFocusVob() == it)
            player->ClearFocusVob();
        player->PutInInv(it);
        return true;
    }

    static bool JPick(const json& pre, const json& post, std::string& d) {
        const int a = CTInt(pre, "item"), b = CTInt(post, "item");
        d = CTFmt("%s an der Stelle beim Partner %d -> %d (soll -1)", CT.pickInst.c_str(), a, b);
        if (a == 0)
            d += " - lag beim Partner gar nicht dort";
        return a >= 1 && b == a - 1;
    }

    // Welt-Abgleich neu anfordern (nur Client, WORLD_REQUEST wie nach dem Laden, E4) - danach Boden um mich gleich
    static bool AWorldAgain(std::string&) {
        CoopWorldSyncRequestAgain();
        return true;
    }

    // =====================================================================================================================
    // Bereich 7: Coop-Funktionen
    // =====================================================================================================================

    static bool JRune(const json&, const json& post, std::string& d) {
#if !COOP_READY_Teleport
        CT_MODULE_OFF_JUDGE("Teleport-Rune")
#else
        const int idx = CTInt(post, "rune", "idx"), has = CTInt(post, "rune", "has");
        if (idx < 0) {
            d = "beim Partner fehlt ITRU_COOP_TELEPORT (COOP_ITEMS.D)";
            return false;
        }
        const int mine = (TpRune() >= 0 && player->IsInInv(TpRuneIdx, 1)) ? 1 : 0;
        d = CTFmt("Rune im Inventar: Partner %s, hier %s", has == 1 ? "ja" : "NEIN", mine ? "ja" : "NEIN");
        return has == 1 && mine == 1;
#endif
    }

    static bool JRuneLook(const json&, const json& post, std::string& d) {
#if !COOP_READY_Teleport
        CT_MODULE_OFF_JUDGE("Teleport-Rune")
#else
        const int st = CTInt(post, "rune", "st"), look = CTInt(post, "rune", "look");
        oCItem* it = TpRune() >= 0 ? player->IsInInv(TpRuneIdx, 1) : NULL;
        const bool mine = TpRuneHasOwnLook(it);   // u89 (P113)
        d = CTFmt("eigenes Aussehen: Partner %s (Zustand %d), hier %s (Zustand %d)", look == 1 ? "ja" : "NEIN", st, mine ? "ja" : "NEIN", TpLookState);
        return look == 1 && mine;
#endif
    }

    static bool JMark(const json&, const json& post, std::string& d) {
        const int inw = CTInt(post, "mark", "inw"), hud = CTInt(post, "mark", "hud"), age = CTInt(post, "mark", "age"), mode = CTInt(post, "mark", "mode");
        if (inw != 1) {
            d = "meine Figur ist beim Partner nicht in der Welt";
            return false;
        }
        if (hud == 1) {
            d = "-- beim Partner ist die Anzeige gerade aus (Dialog, Inventar oder Karte)";
            return false;
        }
        d = age < 0 ? std::string("Markierung zeigt mich nie") :
            CTFmt("zuletzt vor %d ms gezeichnet (%s)", age, mode == 0 ? "Name ueber dem Kopf" : "Pfeil am Bildrand");
        return age >= 0 && age < 1500;
    }

    // Zuschau-Fenster (K2) - nur mit dem Coop-GD3D11 (Export GDX_CoopSecondView)
    static bool PWin(std::string& why) {
        if (!CoopCamWindowAvailable()) {
            why = "Renderer ohne Zuschau-Fenster (Original-GD3D11)";
            return false;
        }
        if (CoopCamWindowOn())
            CoopCamStop("Coop-Test: Fenster-Schritt beginnt neu");   // z. B. von der Automatik geoeffnet
        for (auto& kv : PlayerNpcs)
            if (kv.first && kv.first->vobLeafList.GetNum() > 0)
                return true;
        why = "keine Figur eines Mitspielers in der Welt";
        return false;
    }

    static bool AWin(std::string& why) {
        CoopCamToggle();
        if (!CoopCamWindowOn()) {
            why = "Zuschau-Fenster liess sich nicht einschalten";
            return false;
        }
        return true;
    }

    static bool JWin(const json&, const json&, std::string& d) {
        const bool on = CoopCamWindowSent();
        d = on ? "Fenster an (Renderer bekommt die Verfolgerkamera je Bild) - Bild selbst von Hand pruefen" : "Fenster wieder aus";
        return on;
    }

    static bool PWinOff(std::string& why) {
        if (!CoopCamWindowOn()) {
            why = "Zuschau-Fenster war nicht an";
            return false;
        }
        return true;
    }

    static bool AWinOff(std::string& why) {
        for (int i = 0; i < 8 && CoopCamWindowOn(); i++)
            CoopCamToggle();
        if (CoopCamWindowOn()) {
            why = "Zuschau-Fenster liess sich nicht ausschalten";
            return false;
        }
        return true;
    }

    static bool JWinOff(const json&, const json&, std::string& d) {
        const bool off = !CoopCamWindowOn();
        d = off ? "Fenster aus, Kamera und Held unberuehrt" : "Fenster wieder an (Automatik?)";
        return off && !player->IsMovLock();
    }

    // u135 (P132): Zuschau-Fenster mit weit entferntem Partner - die zweite Kamera steht dann weit von der ersten. Der Testlauf stuerzte mit
    // AtmosphericScattering=0 beim automatischen Oeffnen ab (Partner 515 m entfernt), der Schritt "Zuschau-Fenster an" mit dem Partner daneben nie.
    // Der eigene Held geht dafuer an den entferntesten Wegpunkt und danach zurueck.
    static zVEC3 CTFarHome;
    static bool CTFarAway = false;
    static float CTFarDist = 0.f;

    static zCWaypoint* CTFarthestWaypoint() {
        zCWorld* w = ogame->GetGameWorld();
        zCWayNet* net = w ? w->wayNet : NULL;
        if (!net)
            return NULL;
        const zVEC3 me = player->GetPositionWorld();
        zCWaypoint* best = NULL;
        float bestD = 10000.f;   // mindestens 100 m (die Automatik oeffnet ab 45 m)
        for (zCListSort<zCWaypoint>* l = net->wplist.next; l; l = l->next) {
            zCWaypoint* wp = l->data;
            if (!wp)
                continue;
            const float d = CoopDist(wp->GetPositionWorld(), me);
            if (d > bestD) {
                best = wp;
                bestD = d;
            }
        }
        return best;
    }

    static bool PWinFar(std::string& why) {
        if (!PWin(why))
            return false;
        if (!CTFarthestWaypoint()) {
            why = "kein Wegpunkt mindestens 100 m entfernt";
            return false;
        }
        return true;
    }

    static bool AWinFar(std::string& why) {
        zCWaypoint* wp = CTFarthestWaypoint();
        if (!wp) {
            why = "kein Wegpunkt mindestens 100 m entfernt";
            return false;
        }
        CTFarHome = player->GetPositionWorld();
        CTFarDist = CoopDist(wp->GetPositionWorld(), CTFarHome);
        CTFarAway = true;
        player->SetPositionWorld(wp->GetPositionWorld());
        if (!CoopCamWindowOn())
            CoopCamToggle();
        return true;
    }

    static bool JWinFar(const json&, const json&, std::string& d) {
        const bool on = CoopCamWindowSent();
        d = CTFmt("Fenster %s, Held %.0f m vom Partner entfernt", on ? "an" : "AUS", CTFarDist / 100.f);
        return on;
    }

    static void CWinFar() {
        for (int i = 0; i < 8 && CoopCamWindowOn(); i++)
            CoopCamToggle();
        if (CTFarAway && player)
            player->SetPositionWorld(CTFarHome);
        CTFarAway = false;
    }

    // u135 (P132): im Testlauf fiel das Oeffnen des Fensters mit der gemeinsamen Sicherung beim Verbinden zusammen - darum speichern, solange
    // es offen ist. Wird der Schritt uebersprungen, laeuft kein cleanup: dann hier zurueck
    static bool PSaveFar(std::string& why) {
        if (!CoopRuleSaves()) {
            why = "Spielstaende getrennt (Einstellungen > COOP > Abgleich)";
            CWinFar();
            return false;
        }
        return true;
    }

    // =====================================================================================================================

    // =====================================================================================================================
    // Bereich 8 (u30): Speichern und Laden - Nutzer: "wir sollten den Tests noch ein paar Sachen hinzufuegen wie Speichern und
    // Laden". Am Ende des Laufs, weil das Laden die Welt neu aufbaut. Gespeichert wird in den naechsten reservierten Slot
    // (coopSaveSlots, wie die Auto-Sicherung), geladen genau diese Sicherung - am Spielstand aendert das nichts, was nicht eben
    // gespeichert wurde. Host laedt wie aus der Liste im Esc-Menue, Client als Ladewunsch an den Host. Danach: eigener Held wie beim
    // Speichern, Figuren gegenseitig da, beide in derselben Welt (auch wenn sie vorher in verschiedenen waren - der Stand des Hosts
    // gilt).
    // =====================================================================================================================

    static void CTHeroSnapOf(CTHeroSnap& s) {
        s.lvl = player->level;
        s.exp = (int)player->experience_points;
        s.str = player->GetAttribute(NPC_ATR_STRENGTH);
        s.dex = player->GetAttribute(NPC_ATR_DEXTERITY);
        s.hpMax = player->GetAttribute(NPC_ATR_HITPOINTSMAX);
        s.world = CoopWorldName();
        const zVEC3 p = player->GetPositionWorld();
        for (int i = 0; i < 3; i++)
            s.pos[i] = p.n[i];
    }

    // u31: Werte des Partner-Helden aus der Antwort auf "save"
    static void CTHeroSnapFrom(const json& post, CTHeroSnap& s) {
        s.lvl = CTInt(post, "save", "lvl");
        s.exp = CTInt(post, "save", "exp");
        s.str = CTInt(post, "save", "str");
        s.dex = CTInt(post, "save", "dex");
        s.hpMax = CTInt(post, "save", "hpmax");
        s.world = CTStr(post, "save", "world");
        if (post.contains("save") && post["save"].contains("p") && post["save"]["p"].is_array() && post["save"]["p"].size() == 3) {
            const zVEC3 p = CTVecOf(post["save"]["p"]);
            for (int i = 0; i < 3; i++)
                s.pos[i] = p.n[i];
        }
    }

    static float CTSnapDist(const CTHeroSnap& a, const CTHeroSnap& b) {
        const float dx = a.pos[0] - b.pos[0], dy = a.pos[1] - b.pos[1], dz = a.pos[2] - b.pos[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }

    // u31: beim Speichern in verschiedenen Welten - dann laedt jeder seinen eigenen Stand (CoopSaveSync.cpp, P71)
    static bool CTSavedApart() {
        return !CT.heroAtSave.world.empty() && !CT.partnerAtSave.world.empty() && CT.heroAtSave.world != CT.partnerAtSave.world;
    }

    static bool CTHeroSnapEqual(const CTHeroSnap& a, const CTHeroSnap& b) {
        return a.lvl == b.lvl && a.exp == b.exp && a.str == b.str && a.dex == b.dex && a.hpMax == b.hpMax;
    }

    static std::string CTHeroSnapText(const CTHeroSnap& s) {
        return CTFmt("Stufe %d, EP %d, Staerke %d, Geschick %d, LP max %d", s.lvl, s.exp, s.str, s.dex, s.hpMax);
    }

    // Ladevorgang seit dem letzten Bild: Zeiger in die alte Welt vergessen (vom Test angelegte Gegenstaende, Truhe); die Merker fuer
    // Zuruecknehmen gelten nicht mehr - der Held kommt aus dem Spielstand
    static void CTForgetWorld() {
        CT.weapon = CT.ranged = CT.armor = CT.ammo = NULL;
        CT.chest = NULL;
        CT.overlayOn = false;
        CT.headTurned = false;
        CT.varSet = false;
        CT.heroVarSet = false;
        CT.repSet = false;
        CT.guildSet = false;
        CT.talOld = -1;
    }

    // u105 (P127): Auto-Sicherung "alle 3 min" (CoopDifficulty.cpp, CoopSaveSync.cpp) - der Host stellt sie fuer den Test auf 3 min
    // (ohne Gothic.ini, laeuft nach 40 s ab) und setzt die letzte Sicherung auf 2:58 min zurueck: nach rund 2 s - genau bei 3:00 -
    // sichert er gemeinsam, der Partner speichert mit (gleiche Kennung im selben Slot); beide sehen die Regel 3 min. Testet der
    // Client, stellt der Host das auf seine Frage vorher um. Danach gilt wieder die Einstellung aus dem Menue.
    static int CTAutoRunde = 0, CTAutoZahl0 = 0;
    static long long CTAutoLosMs = 0;

    static bool PAuto(std::string& why) {
        if (!CoopRuleSaves()) {
            why = "Spielstaende getrennt (Einstellungen > COOP > Abgleich)";
            return false;
        }
        if (ServerThread && CoopSaveSyncPeers() <= 0) {
            why = "kein Mitspieler verbunden";
            return false;
        }
        CTAutoRunde++;
        CTAutoLosMs = 0;
        return true;
    }

    static json QAuto() {
        json a;
        a["id"] = CTFmt("%s-%d", CT.tag.c_str(), CTAutoRunde);
        if (!ServerThread)
            a["los"] = 1;   // Client testet: der Host stellt bei der ersten Frage um
        else {
            int zahl = 0, slot = -1;
            long long f = 0;
            bool frei = true;
            CoopSaveSyncTestAutoInfo(zahl, slot, f, frei);
            if (CTAutoLosMs && zahl > CTAutoZahl0)
                a["slot"] = slot;   // Host testet: Kennung des Partners in diesem Slot
        }
        json q;
        q["auto"] = a;
        return q;
    }

    static bool AAuto(std::string&) {
        if (!ServerThread)
            return true;   // der Host hat schon auf die Frage vorher umgestellt
        int slot = -1;
        long long f = 0;
        bool frei = true;
        CoopSaveSyncTestAutoInfo(CTAutoZahl0, slot, f, frei);
        CoopRuleAutoSaveTest(3, 40000);
        CoopSaveSyncTestAutoAgo(3 * 60000 - 2000);
        CTAutoLosMs = CurrentMs;
        return true;
    }

    static bool JAuto(const json& pre, const json& post, std::string& d) {
        int zahl = 0, slot = -1;
        long long f = 0;
        bool frei = true;
        CoopSaveSyncTestAutoInfo(zahl, slot, f, frei);
        const int hier = CoopRuleAutoSaveMinutes(), dort = CTInt(post, "auto", "min");
        const bool freiDort = CTInt(post, "auto", "frei") == 1;
        int neu, nach, s;
        std::string kHost, kClient;
        if (ServerThread) {
            neu = zahl - CTAutoZahl0;
            nach = CTAutoLosMs && f >= CTAutoLosMs ? (int)(f - CTAutoLosMs) : -1;
            s = neu > 0 ? slot : -1;
            kHost = s >= 0 ? CoopSaveSyncSlotSid(s) : std::string();
            kClient = CTStr(post, "auto", "sid");
        }
        else {
            neu = CTInt(post, "auto", "zahl") - CTInt(pre, "auto", "zahl");
            nach = CTInt(post, "auto", "nach");
            s = neu > 0 ? CTInt(post, "auto", "slot") : -1;
            kHost = CTStr(post, "auto", "sid");
            kClient = s >= 0 ? CoopSaveSyncSlotSid(s) : std::string();
        }
        d = CTFmt("Regel hier %d min, beim Partner %d min (soll je 3); Auto-Sicherungen des Hosts +%d (soll 1), faellig %d ms nach dem "
            "Umstellen (soll 1500-6000: bei 3:00), Slot %d: Kennung Host %s, Client %s; frei hier %d, dort %d", hier, dort, neu, nach, s,
            kHost.empty() ? "-" : kHost.c_str(), kClient.empty() ? "-" : kClient.c_str(), frei ? 1 : 0, freiDort ? 1 : 0);
        return hier == 3 && dort == 3 && neu == 1 && nach >= 1500 && nach <= 6000 && !kHost.empty() && kHost == kClient && frei && freiDort;
    }

    static void CAuto() {
        if (ServerThread)
            CoopRuleAutoSaveTest(0, 0);   // wieder wie im Menue (testet der Client, laeuft es beim Host nach 40 s ab)
    }

    static json QSave() {
        json q;
        q["save"]["slot"] = CT.saveSlot;
        q["save"]["sid"] = CT.saveSlot >= 0 ? CoopSaveSyncSlotSid(CT.saveSlot) : std::string();
        return q;
    }

    static json QLoad() { json q; q["load"] = 1; return q; }

    static bool PSave(std::string& why) {
        if (!CoopRuleSaves()) {
            why = "Spielstaende getrennt (Einstellungen > COOP > Abgleich)";
            return false;
        }
        return true;
    }

    static bool ASave(std::string& why) {
        CTHeroSnapOf(CT.heroAtSave);
        const int slot = CoopSaveSyncTestSave(why);
        if (slot < 0)
            return false;
        CT.saveSlot = slot;
        CT.saveSid.clear();
        CT.bkOk = CT.bkHidden = false;   // u55
        CT.sidBefore = CoopSaveSyncSlotSid(slot);   // gespeichert wird erst am Ende des Bilds
        return true;
    }

    static bool JSave(const json&, const json& post, std::string& d) {
        const std::string mine = CT.saveSlot >= 0 ? CoopSaveSyncSlotSid(CT.saveSlot) : std::string();
        const std::string theirs = CTStr(post, "save", "sid");
        const int hero = CTInt(post, "save", "hero");
        d = CTFmt("Slot %d: Kennung hier %s, beim Partner %s, sein Held dazu %s; Welt hier %s, beim Partner %s", CT.saveSlot,
            mine.empty() ? "-" : mine.c_str(), theirs.empty() ? "-" : theirs.c_str(), hero == 1 ? "ja" : "nein", CoopWorldName().c_str(),
            CTStr(post, "save", "world").c_str());
        const bool fresh = !mine.empty() && mine != CT.sidBefore;
        if (!fresh && !mine.empty())
            d += " - noch die alte Sicherung im Slot";
        const bool ok = fresh && theirs == mine && hero == 1;
        if (ok) {
            CT.saveSid = mine;
            CTHeroSnapFrom(post, CT.partnerAtSave);
        }
        return ok;
    }

    static bool PLoad(std::string& why) {
        if (CT.saveSid.empty()) {
            why = "keine gemeinsame Testsicherung (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool ALoad(std::string& why) {
        CT.loadedMs = 0;
        CT.expectLoad = true;
        if (!CoopSaveSyncTestLoad(CT.saveSlot, why)) {
            CT.expectLoad = false;
            return false;
        }
        CTSay("Coop-Test: lade die Testsicherung gemeinsam (Slot %d, %s) - der Lauf geht danach weiter", CT.saveSlot, CT.saveSid.c_str());
        return true;
    }

    static bool JLoad(const json& pre, const json& post, std::string& d) {
        const int n0 = CTInt(pre, "load", "n"), n1 = CTInt(post, "load", "n");
        const std::string theirs = CTStr(post, "load", "sid"), mine = CoopSaveSyncLastLoadSid();
        const bool meLoaded = CT.loadedMs > CT.actMs;
        d = CTFmt("hier %s (%s), beim Partner %s (%s) - soll %s", meLoaded ? "geladen" : "NICHT geladen", mine.empty() ? "-" : mine.c_str(),
            n1 > n0 ? "geladen" : "nicht geladen", theirs.empty() ? "-" : theirs.c_str(), CT.saveSid.c_str());
        return meLoaded && n0 != -999 && n1 > n0 && theirs == CT.saveSid && mine == CT.saveSid;
    }

    static void CLoad() { CT.expectLoad = false; }   // kam der Ladevorgang nicht, gilt ein spaeterer nicht mehr als erwartet

    static bool PAfterLoad(std::string& why) {
        if (CT.saveSid.empty() || !CT.loadedMs) {
            why = "nicht gemeinsam geladen (Schritt davor)";
            return false;
        }
        return true;
    }

    // eigener Held wie beim Speichern, der des Partners ebenso (der Client behaelt seinen Helden auch beim Uebernehmen der Host-Welt);
    // u31 dazu die Lage (bis 5 m - der Client steht in derselben Welt ggf. neben dem Host)
    static bool JHeroKept(const json&, const json& post, std::string& d) {
        CTHeroSnap now, theirs;
        CTHeroSnapOf(now);
        CTHeroSnapFrom(post, theirs);
        const float dMe = CTSnapDist(now, CT.heroAtSave), dThem = CTSnapDist(theirs, CT.partnerAtSave);
        const bool mineOk = CTHeroSnapEqual(now, CT.heroAtSave) && dMe < 500.f;
        const bool theirsOk = CTHeroSnapEqual(theirs, CT.partnerAtSave) && dThem < 500.f;
        d = CTFmt("eigener Held %s (%s, %.1f m von der Lage beim Speichern), Held des Partners %s (%s, %.1f m)",
            mineOk ? "wie beim Speichern" : "ANDERS", mineOk ? CTHeroSnapText(now).c_str() : (CTHeroSnapText(CT.heroAtSave) + " -> " + CTHeroSnapText(now)).c_str(),
            dMe / 100.f, theirsOk ? "wie beim Speichern" : "ANDERS",
            theirsOk ? CTHeroSnapText(theirs).c_str() : (CTHeroSnapText(CT.partnerAtSave) + " -> " + CTHeroSnapText(theirs)).c_str(), dThem / 100.f);
        return mineOk && theirsOk;
    }

    // u31: Figuren wie beim Speichern - in derselben Welt gegenseitig da, nach getrenntem Speichern beide nicht (jeder in seiner Welt)
    static bool JFigsAfterLoad(const json& pre, const json& post, std::string& d) {
        if (!CTSavedApart())
            return JFigs(pre, post, d);
        const bool ok = JSepFigs(pre, post, d);
        d += " (beim Speichern in verschiedenen Welten)";
        return ok;
    }

    // u31: jeder in der Welt, in der er gespeichert hat - in derselben bzw. jeder in seiner (bis u30 holte das Laden alle in die Welt
    // des Hosts)
    static bool JWorldAfterLoad(const json& pre, const json& post, std::string& d) {
        const std::string w = CTStr(post, "save", "world"), mine = CoopWorldName();
        d = CTFmt("Partner %s, hier %s (beim Speichern: hier %s, Partner %s)", w.c_str(), mine.c_str(), CT.heroAtSave.world.c_str(),
            CT.partnerAtSave.world.c_str());
        return !mine.empty() && mine == CT.heroAtSave.world && w == CT.partnerAtSave.world;
    }

    // =====================================================================================================================
    // u39: Welten im Testlauf - Nutzer (Test u38: Host im Minental, Partner in Khorinis, 70 von 110 Schritten "--"): "die ganzen
    // Tests wurden uebersprungen, das sollte aber nicht so sein". Ist der Partner beim Start in einer anderen Welt, holt ihn der Test
    // in die eigene Welt (er wechselt wie beim gemeinsamen Weltwechsel und tritt neben mich, Paket SELFTEST "go"); dann laufen alle
    // Schritte. Am Ende (Bereich 9) schickt ihn der Test in eine andere Welt - die, aus der er kam, sonst die zweite Hauptwelt - und
    // prueft dort die Trennung, den Weltprozess samt Zuschau-Fenster (u38) und gemeinsames Speichern/Laden in verschiedenen Welten
    // mit dem Weltstand-Tausch (P81). Danach holt er ihn noch einmal her (in dessen Weltdatei steht mein Held - er darf danach nicht
    // in der Welt sein) und stellt die Ausgangslage wieder her.
    // =====================================================================================================================

    static oCNpc* CTPartnerFig() {
        auto it = PlayerNameToNpc.find(string(CT.partner.c_str()));
        return it != PlayerNameToNpc.end() ? it->second : NULL;
    }

    static void CTSend(const char* f, const json& extra);   // weiter unten (Ablauf)

    // tp: statt Weltwechsel die Teleport-Rune ueber Welten (CoopTeleportAcross, Ziel ich)
    static void CTSendGo(const std::string& world, bool withPos, bool keep, bool tp = false) {
        json x;
        x["run"] = CT.tag;
        x["to"] = CT.partner;
        x["from"] = DialogMe();
        x["w"] = world;
        if (withPos) {
            x["p"] = CTVec(player->GetPositionWorld());
            x["a"] = CTVec(player->GetAtVectorWorld());
        }
        x["keep"] = keep ? 1 : 0;
        x["tp"] = tp ? 1 : 0;
        CTSend("go", x);
    }

    static bool CTHomeKept = false;   // beim ersten Herholen merkt sich der Partner seine Welt und Lage

    static bool AFetch(std::string& why) {
        if (CT.partner.empty()) {
            why = "Partner unbekannt";
            return false;
        }
        if (!CT.otherWorld) {
            CTLog("Partner ist schon in meiner Welt (%s)", CoopWorldName().c_str());
            return true;
        }
        const bool keep = CT.startedApart && !CTHomeKept;
        CTHomeKept = CTHomeKept || keep;
        CTSendGo(CoopWorldName(), true, keep);
        CTSay("Coop-Test: %s kommt in meine Welt (%s) - er wechselt die Welt, das dauert etwas", CT.partner.c_str(),
            CoopWorldShow(CoopWorldName()).c_str());
        return true;
    }

    // in derselben Welt, seine Figur hier, sein Weltwechsel fertig, kein zweiter Held (aus einer getauschten Weltdatei) hier oder dort
    static bool JFetch(const json&, const json& post, std::string& d) {
        const std::string w = CTStr(post, "base", "world"), mine = CoopWorldName();
        oCNpc* f = CTPartnerFig();
        const bool fig = CTInWorld(f);
        const float dist = fig ? CoopDist(f->GetPositionWorld(), player->GetPositionWorld()) : -1.f;
        const int going = CTInt(post, "base", "going"), stray = CTInt(post, "base", "stray"), strayHere = CTStrayHeroes();
        d = CTFmt("Partner in %s, hier %s; seine Figur hier %s%s; zweiter Held beim Partner %d, hier %d", w.c_str(), mine.c_str(),
            fig ? "da" : "fehlt", fig ? CTFmt(" (%.1f m)", dist / 100.f).c_str() : "", stray, strayHere);
        const bool ok = !w.empty() && CoopWcBase(w) == CoopWcBase(mine) && fig && going == 0 && stray == 0 && strayHere == 0;
        if (ok) {
            CT.otherWorld = false;
            CT.otherText.clear();
        }
        return ok;
    }

    // zweite Hauptwelt der Engine (G2: Khorinis <-> Minental, G1: Kolonie <-> Freie Mine). u134: auch Othello 4.x hat FREEMINE.ZEN
    // (WorldsOthello4.0.vdf) - bis dahin fielen dort alle 43 Schritte ueber Welten als "keine zweite Welt bekannt" aus
    static std::string CTSecondWorld(const std::string& mine) {
        const std::string b = CoopWcBase(mine);
#if ENGINE >= Engine_G2
        return b == "NEWWORLD.ZEN" ? "OLDWORLD\\OLDWORLD.ZEN" : "NEWWORLD\\NEWWORLD.ZEN";
#else
        return b == "WORLD.ZEN" ? "FREEMINE.ZEN" : "WORLD.ZEN";
#endif
    }

    static bool ASendAway(std::string& why) {
        if (CT.partner.empty()) {
            why = "Partner unbekannt";
            return false;
        }
        if (CT.otherWorld) {
            CTLog("Partner ist schon in einer anderen Welt (%s)", CT.otherText.c_str());
            return true;
        }
        std::string target;   // leer: zurueck in seine Welt an seine Stelle
        if (!CT.startedApart) {
            target = CTSecondWorld(CoopWorldName());
            if (target.empty()) {
                why = "keine zweite Welt bekannt (Mod)";
                return false;
            }
        }
        CTSendGo(target, false, false);
        CTSay("Coop-Test: %s geht %s - danach Trennung, Weltprozess und Speichern in verschiedenen Welten", CT.partner.c_str(),
            target.empty() ? "zurueck in seine Welt" : ("nach " + CoopWorldShow(target)).c_str());
        return true;
    }

    static bool JAway(const json&, const json& post, std::string& d) {
        const int mine = CoopWorldKey(), theirs = CTInt(post, "base", "wk"), going = CTInt(post, "base", "going");
        const std::string w = CTStr(post, "base", "world");
        const bool fig = CTInWorld(CTPartnerFig());
        d = CTFmt("Partner in %s, hier %s; seine Figur hier %s", w.c_str(), CoopWorldName().c_str(), fig ? "noch DA" : "weg");
        const bool ok = mine != 0 && theirs > 0 && theirs != mine && !fig && going == 0;
        if (ok) {
            CT.otherWorld = true;
            CT.otherText = w;
        }
        return ok;
    }

    // Teleport-Rune ueber Welten (u39, N2): der Partner teleportiert sich zu mir - Weltwechsel in meine Welt, dann neben mich. In seiner
    // Kopie meiner Welt steht seit dem Weltstand-Tausch mein Held - er darf danach nicht in der Welt sein (JFetch: zweiter Held)
    static bool PTpHere(std::string& why) {
        if (!CT.otherWorld) {
            why = "Partner nicht in einer anderen Welt (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool ATpHere(std::string&) {
        CTSendGo(CoopWorldName(), false, false, true);
        CTSay("Coop-Test: %s teleportiert sich ueber die Welten zu mir (Rune)", CT.partner.c_str());
        return true;
    }

    static bool JTpHere(const json& pre, const json& post, std::string& d) {
        const bool ok = JFetch(pre, post, d);
        oCNpc* f = CTPartnerFig();
        const float dist = CTInWorld(f) ? CoopDist(f->GetPositionWorld(), player->GetPositionWorld()) : -1.f;
        if (ok && dist > 1000.f)
            d += " - NICHT neben mir";
        return ok && dist >= 0.f && dist <= 1000.f;
    }

    // Ausgangslage: anfangs getrennt -> zurueck in seine Welt an seine Stelle; anfangs zusammen -> er ist (wieder) hier
    static bool ARestore(std::string&) {
        if (CT.startedApart) {
            if (CT.otherWorld)
                return true;
            CTSendGo(std::string(), false, false);
            CTSay("Coop-Test: %s geht zurueck in seine Welt (wie beim Start)", CT.partner.c_str());
            return true;
        }
        if (!CT.otherWorld)
            return true;
        CTSendGo(CoopWorldName(), true, false);
        CTSay("Coop-Test: %s kommt zurueck in meine Welt (wie beim Start)", CT.partner.c_str());
        return true;
    }

    static bool JRestore(const json& pre, const json& post, std::string& d) {
        if (CT.startedApart)
            return JAway(pre, post, d);
        return JFetch(pre, post, d);
    }

    // --- u47 (P88): Betreten - der Stand dessen gilt, der dort ist bzw. zuletzt dort war (CoopWorlds.cpp CoopEnterGate) ----------
    // Nutzer: "beim Betreten soll der Stand dessen gelten, der dort war bzw. zuletzt dort war ... nimm Testschritte fuers Betreten in
    // den Welten-Testlauf auf". Der Partner markiert seine Welt (Fleischwanze tot, Apfel am Boden), ich betrete sie direkt am
    // markierten Wegpunkt (die Sperre holt vorher seinen Stand) und finde beides; zurueck in meine Welt an meine Stelle; ich markiere
    // meine Welt, der Partner kommt per Teleport (Schritt danach) und findet meine Markierung. Laeuft der Test beim Host, betritt der
    // Host die Welt des Clients (bis u46 ueberschrieb dabei sein Stand den des Clients) und umgekehrt.
    struct CTEnterState {
        bool partnerMark = false;
        std::string world, wp;      // Markierung des Partners
        zVEC3 pos;
        std::string homeWorld;      // meine Welt und Lage vor dem Betreten
        zVEC3 homePos, homeAt;
        bool homeSet = false, homePlaced = false;
        std::string entryVar;       // u140 (P139): Eintritts-Merker, den der Partner in seiner Welt gesetzt hat
    };
    static CTEnterState CTEnter;

    // u140 (P139): Probe fuer "Eintritts-Merker reisen mit der Weltdatei" - der Merker von Kapitel 6 meiner Welt auf 1. Die Skripte dazu sind leer und
    // laufen erst ab Kapitel 6: Wer diese Welt jetzt betritt, kann den Merker nur mit der Datei bekommen. G1: der Merker der Mine, sonst keiner
    static std::string CTEntryProbe() {
        std::string name;
        for (auto& v : CoopWorldEntryVarsGet(WxWorldTag(CoopWorldName()))) {
            const std::string n = v.at(0).get<std::string>();
            if (name.empty() || n[n.size() - 1] == '6')
                name = n;
        }
        if (zCPar_Symbol* s = name.empty() ? NULL : parser->GetSymbol(zSTRING(name.c_str())))
            s->SetValue(1, 0);
        return name;
    }

    static int CTEntryVarNow(const std::string& name) {
        int v = -1;
        if (zCPar_Symbol* s = name.empty() || !parser ? NULL : parser->GetSymbol(zSTRING(name.c_str())))
            s->GetValue(v, 0);
        return v;
    }

    static void CTEntryProbeOff(const std::string& name) {
        if (name.empty() || name[name.size() - 1] != '6')
            return;
        if (zCPar_Symbol* s = parser->GetSymbol(zSTRING(name.c_str())))
            s->SetValue(0, 0);
    }

    static void CTEntryProbeOffBoth(const std::string& name) {
        if (name.empty())
            return;
        CTEntryProbeOff(name);
        json x;
        x["run"] = CT.tag;
        x["to"] = CT.partner;
        x["from"] = DialogMe();
        x["evoff"] = name;
        CTSend("go", x);
    }

    // u141 (P141): In derselben Welt gilt der Merker fuer beide
    static std::string CTEntrySame;

    static json QEntrySame() { json q; q["var"] = json::array({ CTEntrySame }); return q; }

    static bool PEntrySame(std::string& why) {
        CTEntrySame.clear();
        for (auto& v : CoopWorldEntryVarsGet(WxWorldTag(CoopWorldName()))) {
            const std::string n = v.at(0).get<std::string>();
            if (n[n.size() - 1] == '6')
                CTEntrySame = n;
        }
        if (CTEntrySame.empty()) {
            why = "keine Eintritts-Merker fuer Kapitel 6 in dieser Welt";
            return false;
        }
        return PStory(why);
    }

    static bool AEntrySame(std::string&) {
        if (zCPar_Symbol* s = parser->GetSymbol(zSTRING(CTEntrySame.c_str())))
            s->SetValue(1, 0);
        return true;
    }

    static bool JEntrySame(const json&, const json& post, std::string& d) {
        const int v = post.contains("var") ? post["var"].value(CTEntrySame, -999) : -999;
        d = CTFmt("%s beim Partner %d (soll 1)", CTEntrySame.c_str(), v);
        return v == 1;
    }

    static void CEntrySame() { CTEntryProbeOffBoth(CTEntrySame); }

    static json QEnterMark() { json q; q["emark"] = 1; q["base"] = 1; return q; }   // "mark" = Mitspieler-Markierung
    static json QMarkChk() {
        json q;
        q["base"] = 1;
        q["markchk"]["p"] = CTVec(CTMarkMine.pos);
        return q;
    }

    static bool PMarkPartner(std::string& why) {
        CTEnter = CTEnterState();
        if (!CT.otherWorld) {
            why = "Partner nicht in einer anderen Welt (Schritt davor)";
            return false;
        }
        if (!DialogExternal("WLD_INSERTNPC") || !DialogExternal("WLD_INSERTITEM")) {
            why = "WLD_INSERTNPC/WLD_INSERTITEM fehlt";
            return false;
        }
        return true;
    }

    static bool AMarkPartner(std::string&) {
        json x;
        x["run"] = CT.tag;
        x["to"] = CT.partner;
        x["from"] = DialogMe();
        x["w"] = std::string();
        x["mark"] = 1;
        CTSend("go", x);
        CTSay("Coop-Test: %s markiert seine Welt (Fleischwanze tot, Apfel am Boden) - danach betrete ich sie", CT.partner.c_str());
        return true;
    }

    static bool JMarkPartner(const json&, const json& post, std::string& d) {
        const int have = CTInt(post, "emark", "have"), dead = CTInt(post, "emark", "dead"), apples = CTInt(post, "emark", "apples");
        const int going = CTInt(post, "base", "going");
        const std::string w = CTStr(post, "emark", "w"), wp = CTStr(post, "emark", "wp");
        d = CTFmt("beim Partner in %s an %s: tote Fleischwanzen %d, Aepfel %d%s", w.empty() ? "-" : w.c_str(), wp.empty() ? "-" : wp.c_str(),
            dead, apples, going == 1 ? " (markiert noch)" : "");
        bool ok = have == 1 && dead >= 1 && apples >= 1 && going == 0 && !w.empty() && !wp.empty();
        if (ok) {
            try {
                CTEnter.pos = CTVecOf(post.at("emark").at("p"));
            }
            catch (...) {
                ok = false;
            }
        }
        if (ok) {
            CTEnter.partnerMark = true;
            CTEnter.world = w;
            CTEnter.wp = wp;
            CTEnter.entryVar = CTStr(post, "emark", "ev");   // u140 (P139)
            if (CTEntryVarNow(CTEnter.entryVar) == 1) {      // war noch nie dort, der Merker steht hier schon: nicht bewertbar, also zuruecksetzen
                CTLog("Eintritts-Merker %s steht hier schon auf 1 - fuer den Test auf 0", CTEnter.entryVar.c_str());
                CTEntryProbeOff(CTEnter.entryVar);
            }
        }
        return ok;
    }

    static bool PEnter(std::string& why) {
        if (!CTEnter.partnerMark) {
            why = "keine Markierung des Partners (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool AEnter(std::string&) {
        CTEnter.homeWorld = CoopWorldName();
        CTEnter.homePos = player->GetPositionWorld();
        CTEnter.homeAt = player->GetAtVectorWorld();
        CTEnter.homeSet = true;
        CTEnter.homePlaced = false;
        CT.loadedMs = 0;
        CT.expectLoad = true;
        CTSay("Coop-Test: ich betrete die Welt von %s (%s, am Wegpunkt %s) - erst wird sein Stand geholt, dann gewechselt",
            CT.partner.c_str(), CoopWorldShow(CTEnter.world).c_str(), CTEnter.wp.c_str());
        CoopWcGo(CTEnter.world, CTEnter.wp);   // Startpunkt = markierter Wegpunkt (die Markierung liegt gleich in der Naehe)
        return true;
    }

    // hier geladen, in seiner Welt, seine Markierung hier, kein zweiter Held (seiner aus der geholten Weltdatei)
    static bool JEnter(const json&, const json& post, std::string& d) {
        const bool loaded = CT.loadedMs > CT.actMs;
        const bool there = CoopWcBase(CoopWorldName()) == CoopWcBase(CTEnter.world);
        int dead = 0, apples = 0;
        if (loaded && there)
            CTCountMark(CTEnter.pos, dead, apples);
        const int stray = CTStrayHeroes();
        d = CTFmt("%s, hier %s; seine Markierung hier: tote Fleischwanzen %d, Aepfel %d (soll je >= 1); zweiter Held hier %d",
            loaded ? "geladen" : "NICHT geladen", CoopWorldName().c_str(), dead, apples, stray);
        const bool ok = loaded && there && dead >= 1 && apples >= 1 && stray == 0;
        if (loaded && there) {
            CT.otherWorld = false;   // jetzt in derselben Welt
            CT.otherText.clear();
        }
        return ok;
    }

    // u140 (P139): Eintritts-Merker kamen mit der Weltdatei - vor dem Einzug, sonst liefe B_Enter_* auf der Welt des Partners noch einmal
    static bool JEntryVar(const json&, const json&, std::string& d) {
        if (CTEnter.entryVar.empty()) {
            d = "-- nicht bewertbar: diese Welt hat keine Eintritts-Merker";
            return false;
        }
        const int v = CTEntryVarNow(CTEnter.entryVar);
        d = CTFmt("%s hier %d (soll 1, gesetzt nur beim Partner)", CTEnter.entryVar.c_str(), v);
        return v == 1;
    }

    // u140 (P140): festgefahrener Spielstand nachgestellt - Abschluss-Gespraech des Sumpfdrachen gehoert, Drache unsterblich. Danach alles wie vorher
    static bool CTDragonOk = false;
    static int CTDragonBefore = 0, CTDragonAfter = 0;

    static oCInfo* CTDragonInfo() {
        const int idx = parser ? parser->GetIndex(zSTRING("DIA_DRAGON_SWAMP_EXIT")) : -1;
        return idx >= 0 && ogame->GetInfoManager() ? ogame->GetInfoManager()->GetInformation(idx) : NULL;
    }

    static bool PDragonHeal(std::string& why) {
        if (!CTDragonInfo() || !DialogNpcByName("DRAGON_SWAMP-1")) {
            why = "kein Sumpfdrache in dieser Welt";
            return false;
        }
        return true;
    }

    static bool ADragonHeal(std::string&) {
        oCInfo* info = CTDragonInfo();
        oCNpc* d = DialogNpcByName("DRAGON_SWAMP-1");
        const int told = info->told, flags = d->variousFlags;
        info->told = TRUE;
        d->variousFlags = flags | NPC_FLAG_IMMORTAL;
        CTDragonBefore = d->variousFlags;
        const int healed = DlgDragonsHeal();
        CTDragonAfter = d->variousFlags;
        CTDragonOk = healed == 1 && !(CTDragonAfter & NPC_FLAG_IMMORTAL);
        info->told = told;
        d->variousFlags = flags;
        return true;
    }

    static bool JDragonHeal(const json&, const json&, std::string& d) {
        d = CTFmt("DRAGON_SWAMP-1: Schutz %d -> %d", CTDragonBefore, CTDragonAfter);
        return CTDragonOk;
    }

    // u142 (P142): Gegenstand einer Nachricht ist schon frei - ein Puffer, der aussieht wie neu belegter Speicher ("{\"by" statt vtable).
    // Beide Wege der Engine: Nachricht laeuft schon (dort stuerzten G1 und Othello ab) und Nachricht beginnt
    static int CTMsgVobDead = 0, CTMsgVobLeft = 0, CTMsgVobRet = 0;

    static bool PMsgVob(std::string& why) {
        if (!player || player->interactItem || player->GetInteractMob()) {
            why = "Held benutzt gerade etwas";
            return false;
        }
        return true;
    }

    static int CTMsgVobRun(BOOL running) {
        struct { void* vt; int ref; char rest[0x60]; } alt = { (void*)0x7962227B, 1, { 0 } };
        oCMsgManipulate* msg = new oCMsgManipulate(oCMsgManipulate::EV_USEITEMTOSTATE, (zCVob*)&alt, 0);
        msg->SetInUse(running);
        const int ret = player->EV_UseItemToState(msg);
        if (msg->targetVob)
            CTMsgVobLeft++;
        msg->targetVob = NULL;
        msg->Release();
        return ret;
    }

    static bool AMsgVob(std::string&) {
        const int before = CoopMsgVobDeadCount, target = player->interactItemTargetState, current = player->interactItemCurrentState, body = player->GetBodyState();
        CTMsgVobLeft = 0;
        player->interactItemTargetState = 0;
        CTMsgVobRet = CTMsgVobRun(TRUE);
        CTMsgVobRet += CTMsgVobRun(FALSE);
        player->interactItemTargetState = target;
        player->interactItemCurrentState = current;
        player->SetBodyState(body);
        CTMsgVobDead = CoopMsgVobDeadCount - before;
        return true;
    }

    static bool JMsgVob(const json&, const json&, std::string& d) {
        d = CTFmt("%d alte Zeiger erkannt, %d stehen geblieben, Nachrichten fertig %d von 2", CTMsgVobDead, CTMsgVobLeft, CTMsgVobRet);
        return CTMsgVobDead == 2 && !CTMsgVobLeft && CTMsgVobRet == 2 && !player->interactItem;
    }

    static bool PBack(std::string& why) {
        if (!CTEnter.homeSet) {
            why = "nicht weg (Schritt davor)";
            return false;
        }
        if (CoopWcBase(CoopWorldName()) == CoopWcBase(CTEnter.homeWorld)) {
            why = "schon in meiner Welt";
            return false;
        }
        return true;
    }

    static bool ABack(std::string&) {
        CT.loadedMs = 0;
        CT.expectLoad = true;
        CTSay("Coop-Test: zurueck in meine Welt (%s)", CoopWorldShow(CTEnter.homeWorld).c_str());
        CTEntryProbeOffBoth(CTEnter.entryVar);   // u140 (P139): die Probe zuruecknehmen, hier und beim Partner
        if (CTVarSym()) {   // u140 (P139): im selben Moment wie der Wechsel - nur die Meldung nach dem Weltwechsel bringt den Wert zum Partner
            CT.varValue = 1000 + (int)((GetCurrentMs() / 1000) % 8000);
            CTVarSym()->SetValue(CT.varValue, 0);
            CT.varSet = true;
        }
        CoopWcGo(CTEnter.homeWorld, "");
        return true;
    }

    static bool JBack(const json& pre, const json& post, std::string& d) {
        const bool loaded = CT.loadedMs > CT.actMs;
        const bool home = CoopWcBase(CoopWorldName()) == CoopWcBase(CTEnter.homeWorld);
        if (loaded && home && !CTEnter.homePlaced) {   // an meine alte Stelle (Gothic setzt mich an den Start der Welt)
            CTEnter.homePlaced = true;
            player->SetCollDet(FALSE);
            player->SetPositionWorld(CTEnter.homePos);
            player->SetCollDet(TRUE);
            player->SetHeadingAtWorld(CTEnter.homeAt);
            PendingMoveReset = true;
        }
        const int theirs = CTInt(post, "base", "wk");
        d = CTFmt("%s, hier %s (soll %s), Partner %s", loaded ? "geladen" : "NICHT geladen", CoopWorldName().c_str(),
            CTEnter.homeWorld.c_str(), CTStr(post, "base", "world").c_str());
        const bool ok = loaded && home && theirs > 0 && theirs != CoopWorldKey();
        if (ok) {
            CT.otherWorld = true;   // wieder getrennt (fuer "Teleport zu mir")
            CT.otherText = CTStr(post, "base", "world");
        }
        return ok;
    }

    static bool PMarkMine(std::string& why) {
        CTMarkMine = CTMark();
        if (!CT.otherWorld) {
            why = "Partner nicht in einer anderen Welt";
            return false;
        }
        return CTMarkStart(CTMarkMine, why);
    }

    static bool JMarkMine(const json&, const json&, std::string& d) {
        const int r = CTMarkFinish(CTMarkMine);
        int dead = 0, apples = 0;
        if (r > 0)
            CTCountMark(CTMarkMine.pos, dead, apples);
        d = CTFmt("Markierung hier an %s: tote Fleischwanzen %d, Aepfel %d%s", CTMarkMine.wp.c_str(), dead, apples,
            r < 0 ? " - eingesetzte Fleischwanze nicht gefunden" : r == 0 ? " - noch nicht" : "");
        return r > 0 && dead >= 1 && apples >= 1;
    }

    // nach "Teleport zu mir ueber Welten": der Partner hat meine Welt betreten - meine Markierung muss bei ihm sein
    static bool PMarkSeen(std::string& why) {
        if (!CTMarkMine.have) {
            why = "keine eigene Markierung (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool JMarkSeen(const json&, const json& post, std::string& d) {
        const int dead = CTInt(post, "markchk", "dead"), apples = CTInt(post, "markchk", "apples");
        const std::string w = CTStr(post, "markchk", "w");
        d = CTFmt("beim Partner in %s: meine Markierung an %s - tote Fleischwanzen %d, Aepfel %d (soll je >= 1)", w.c_str(),
            CTMarkMine.wp.c_str(), dead, apples);
        return CoopWcBase(w) == CoopWcBase(CoopWorldName()) && dead >= 1 && apples >= 1;
    }

    // Weltprozess (u38): bereit mit Bild, sobald der Partner woanders ist (Vorstart, Auftrag, Laden - bis 150 s)
    static bool PWp(std::string& why) {
#if ENGINE == Engine_G2A || ENGINE == Engine_G1   // u42: auch G1
        return true;
#else
        why = "Weltprozess nur in Gothic 1 und Gothic 2 NotR";
        return false;
#endif
    }

    static bool JWpReady(const json&, const json&, std::string& d) {
        unsigned int h = 0;
        std::string nick, world, state;
        const bool ok = CoopWpImage(h, nick, world, state);
        d = "Weltprozess: " + state + (ok ? CTFmt(" - %s in %s, Bild %08X", nick.c_str(), world.c_str(), h) : std::string());
        return ok;
    }

    static bool PWinWp(std::string& why) {
        if (!CoopCamWindowAvailable()) {
            why = "Renderer ohne Zuschau-Fenster (Original-GD3D11)";
            return false;
        }
        if (CoopCamWindowOn())
            CoopCamStop("Coop-Test: Fenster-Schritt beginnt neu");   // z. B. von der Automatik geoeffnet
        return PWp(why);
    }

    static bool AWinWp(std::string&) {
        // u135: die Automatik kann das Fenster seit PWinWp schon wieder geoeffnet haben (Othello-Client unter Last: 386 s geoeffnet, 387 s
        // hier ausgeschaltet, Schritt FEHLER) - dann nicht umschalten
        if (!CoopCamWindowOn())
            CoopCamToggle();   // kein Mitspieler in der Welt -> Bild des Weltprozesses
        return true;
    }

    static bool JWinWp(const json&, const json&, std::string& d) {
        const int r = CoopCamWorldProcessResult();
        d = r == -9 ? std::string("Fenster zeigt NICHT den Weltprozess") : r == 2 ? std::string("Fenster zeigt das Bild des Weltprozesses (Renderer meldet 2) - Bild selbst von Hand pruefen")
            : CTFmt("Renderer meldet %d (1 wartet auf das erste Bild, -1 nicht zu oeffnen)", r);
        return r == 2;
    }

    // gemeinsam speichern in verschiedenen Welten (Bereich 9)
    static bool JSaveApart(const json& pre, const json& post, std::string& d) {
        const bool ok = JSave(pre, post, d);
        const std::string w = CTStr(post, "save", "world");
        const bool apart = !w.empty() && CoopWcBase(w) != CoopWcBase(CoopWorldName());
        if (!apart)
            d += " - NICHT in verschiedenen Welten";
        return ok && apart;
    }

    // u55 (P103): Story-Video wie aus einem Skript (G2 Logo2.bik, G1 logo1.bik), hier nach 1,5 s wie mit Esc beendet - der Partner
    // spielt es unter derselben Nummer und muss es auch beenden (Nutzer: "Außerdem werden Videos nciht geskippt wenn einer sie Skippt")
    static json QVid() {
        json q;
        q["vid"] = 1;
        return q;
    }

    static bool PVid(std::string& why) {
        if (!CoopRuleQuests()) {
            why = "Abgleich \"Quests\" aus - Videos laufen nur hier";
            return false;
        }
        return true;
    }

    static bool AVid(std::string& why) {
        long no = 0;
        if (!CoopDialogTestVideo(ENGINE == Engine_G1 ? "LOGO1.BIK" : "LOGO2.BIK", 1500, no, why))
            return false;
        CT.vidNo = no;
        return true;
    }

    static bool JVid(const json&, const json& post, std::string& d) {
        long begun = 0, remote = 0, sent = 0;
        CoopStoryVideoState(begun, remote, sent);
        const long pb = CTInt(post, "vid", "begun"), ps = CTInt(post, "vid", "stopped");
        d = CTFmt("Video Nr. %ld: hier %s, beim Partner %s", CT.vidNo, sent == CT.vidNo ? "nach 1,5 s abgebrochen und gemeldet" : "NICHT als abgebrochen gemeldet",
            ps == CT.vidNo ? "auch beendet" : pb == CT.vidNo ? "gestartet, aber NICHT beendet" : "(noch) nicht gestartet");
        return CT.vidNo != 0 && sent == CT.vidNo && ps == CT.vidNo;
    }

    // u55 (P102): Sicherung beim Partner - nach dem gemeinsamen Speichern liegt bei jedem der Stand des anderen (<Kennung>__von_<PC>).
    // Im Lauf des Hosts legt der Client danach seine eigene Kopie beiseite (verlorene Saves, nur im Testordner) und muss sie beim
    // gemeinsamen Laden aus der Sicherung beim Host zurueckholen - sonst nimmt er die Welt des Hosts statt seiner eigenen (P71), und
    // "Nach dem Laden (getrennt)" schlaegt fehl.
    static json QBk() {
        json q;
        q["bk"]["sid"] = CT.saveSid;
        q["bk"]["pc"] = CoopSaveSyncMyPc();
        return q;
    }

    static json QBkHide() {
        json q = QBk();
        q["bk"]["hide"] = 1;
        return q;
    }

    static bool PBk(std::string& why) {
        if (CT.saveSid.empty()) {
            why = "keine gemeinsame Testsicherung (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool JBk(const json&, const json& post, std::string& d) {
        const std::string theirPc = CTStr(post, "bk", "pc");
        const bool here = CoopSaveSyncHasBackupOf(CT.saveSid, theirPc);
        const bool there = CTInt(post, "bk", "have") == 1;
        d = CTFmt("Kennung %s: hier der Stand des Partners (PC %s) %s, beim Partner meiner %s", CT.saveSid.c_str(),
            theirPc.empty() ? "?" : theirPc.c_str(), here ? "da" : "FEHLT", there ? "da" : "FEHLT");
        CT.bkOk = here && there;
        return CT.bkOk;
    }

    static bool PBkHide(std::string& why) {
        if (!PBk(why))
            return false;
        if (!CT.bkOk) {
            why = "Sicherung beim Partner fehlt (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool JBkHide(const json& pre, const json& post, std::string& d) {
        const bool gone = CTInt(post, "bk", "gone") == 1;
        std::string what = CTStr(pre, "bk", "what");   // beiseitegelegt wird schon bei der ersten Frage
        if (what.empty())
            what = CTStr(post, "bk", "what");
        d = CTFmt("eigene Kopie beim Partner %s (%s)", gone ? "beiseitegelegt" : "NICHT beiseitegelegt", what.c_str());
        CT.bkHidden = gone;
        return gone;
    }

    static bool PBkRestored(std::string& why) {
        if (!PAfterLoad(why))
            return false;
        if (!CT.bkHidden) {
            why = "Kopie beim Partner nicht beiseitegelegt (Schritt davor)";
            return false;
        }
        return true;
    }

    static bool JBkRestored(const json&, const json& post, std::string& d) {
        const bool ok = CTInt(post, "bk", "restored") == 1;
        d = ok ? "Partner hat seine eigene Kopie aus der Sicherung hier zurueckgeholt" : "Partner hat seine Kopie NICHT zurueckgeholt";
        return ok;
    }

    // Weltstand-Tausch (P81): CRC der Weltdateien beider Welten im Testslot bzw. in Saves\current
    static json QWx() {
        json q;
        q["wx"]["slot"] = CT.saveSlot;
        q["wx"]["w"] = json::array({ CT.heroAtSave.world, CT.partnerAtSave.world });
        return q;
    }

    static long long CTWxCrc(const json& post, const std::string& w, const char* k) {
        try {
            return post.at("wx").at("w").at(w).at(k).get<long long>();
        }
        catch (...) {
            return -2;
        }
    }

    // where "s": Slot, "c": Saves\current - die Datei der Welt des Partners hier = seine, die Datei meiner Welt beim Partner = meine
    static bool CTWxSame(const json& post, const char* where, std::string& d) {
        const std::string mw = CT.heroAtSave.world, pw = CT.partnerAtSave.world;
        const int slot = where[0] == 's' ? CT.saveSlot : -1;
        unsigned c1 = 0, c2 = 0;
        long long s1 = 0, s2 = 0;
        const long long hereTheirs = CoopSaveSyncWorldFileCrc(slot, pw, c1, s1) ? (long long)c1 : -1;
        const long long hereMine = CoopSaveSyncWorldFileCrc(slot, mw, c2, s2) ? (long long)c2 : -1;
        const long long thereTheirs = CTWxCrc(post, pw, where), thereMine = CTWxCrc(post, mw, where);
        const int pend = CTInt(post, "wx", "pend"), pendHere = CoopSaveSyncWxPending();
        d = CTFmt("%s: %s hier %08llX, beim Partner %08llX; %s hier %08llX, beim Partner %08llX; offene Abholungen hier %d, beim Partner %d",
            where[0] == 's' ? CTFmt("Slot %d", CT.saveSlot).c_str() : "Saves\\current", CoopWorldShow(pw).c_str(), hereTheirs, thereTheirs,
            CoopWorldShow(mw).c_str(), hereMine, thereMine, pendHere, pend);
        return hereTheirs >= 0 && hereTheirs == thereTheirs && hereMine >= 0 && hereMine == thereMine && pend == 0 && pendHere == 0;
    }

    static bool JWxSlot(const json&, const json& post, std::string& d) {
        return CTWxSame(post, "s", d);
    }

    static bool JWxCurrent(const json&, const json& post, std::string& d) {
        return CTWxSame(post, "c", d);
    }

    static bool PApart(std::string& why) {
        if (!CTSavedApart()) {
            why = "nicht in verschiedenen Welten gespeichert (Schritt davor)";
            return false;
        }
        return PSave(why);
    }

    static const CoopTestDef CoopTests[] = {
        // 1 Verbindung und Grundstand
        { "Partner antwortet (Rundlauf)",        "L1",      100, 0,           NULL,        QBase,      ANone,       JRtt,        NULL },
        { "Fassung gleich",                      "H10",     100, 0,           NULL,        QBase,      ANone,       JVer,        NULL },
        { "Story-Video: Ueberspringen gilt fuer beide", "P103", 2500, CTF_LONG, PVid,      QVid,       AVid,        JVid,        NULL },   // u55
        { "Partner in meine Welt holen",         "P82",    3000, CTF_PARTNERWAIT, NULL,    QBase,      AFetch,      JFetch,      NULL },   // u39
        { "Welt gleich",                         "E4",      100, 0,           NULL,        QBase,      ANone,       JWorld,      NULL },
        { "Weltschluessel beim Partner",         "P65",     100, 0,           NULL,        QBase,      ANone,       JWorldKey,   NULL },   // u29
        { "Wer fuehrt die NPCs des Clients",     "P66",     100, 0,           NULL,        QBase,      ANone,       JWhoLeads,   NULL },
        { "Spieltag und Uhrzeit gleich",         "GT",      100, CTF_LONG,    NULL,        QBase,      ANone,       JDayTime,    NULL },   // u74: bis 20 s (Uhr alle 10 s)
        { "Uhrzeit +30 min",                     "GT",     2000, CTF_HOST,    NULL,        QBase,      ATime,       JTime,       NULL },
        { "Abgleich-Regeln gleich",              "H21",     100, 0,           NULL,        QBase,      ANone,       JRules,      NULL },
        { "Schwierigkeitsgrad gleich",           "H16",     100, 0,           NULL,        QBase,      ANone,       JDiff,       NULL },
        { "Engine-Aenderungen wie beim Partner", "H1/G6",   300, CTF_ONCE,           NULL,        QAudit,     ANone,       JAudit,      NULL },
        { "Gemeinsamer Durchlauf",               "H31",     100, 0,           NULL,        QBase,      ANone,       JRun,        NULL },
        { "Gemeinsames Speichern moeglich",      "H4",      100, 0,           NULL,        QBase,      ANone,       JSaveReady,  NULL },
        { "Netz ohne Rueckstau",                 "L1/L4",   100, 0,           NULL,        QBase,      ANone,       JQueue,      NULL },
        { "Absturzschutz aktiv",                 "A18/A20", 100, 0,           NULL,        QBase,      ANone,       JCrash,      NULL },
        { "Log-Versand",                         "G4",     3500, CTF_LONG,           NULL,        QLog,       ALogMark,    JLogMark,    NULL },
        // 2 Eigene Figur beim Partner
        { "Figuren gegenseitig in der Welt",     "GT",      100, 0,           NULL,        QBase,      ANone,       JFigs,       NULL },
        { "Lage meiner Figur",                   "GT",      300, 0,           NULL,        QFig,       ANone,       JFigPos,     NULL },
        { "Blickrichtung +90 Grad",              "L3",     1200, 0,           NULL,        QFig,       ATurn,       JHead,       NULL },
        { "Blickrichtung zurueck",               "L3",     1200, 0,           PTurnBack,   QFig,       ATurnBack,   JHead,       NULL },
        { "Name meiner Figur",                   "H8",      100, 0,           NULL,        QFig,       ANone,       JName,       NULL },
        { "Name aendern (wie im Menue)",         "H8",     2500, 0,           PName,       QFig,       AName,       JNameNew,    NULL },
        { "Name zurueck",                        "H8",     2500, 0,           PNameBack,   QFig,       ANameBack,   JNameBack,   NULL },
        { "Aussehen meiner Figur",               "H8",      100, 0,           NULL,        QFig,       ANone,       JLook,       NULL },
        { "Gesicht aendern (wie im Menue)",      "H8",     2500, 0,           PFace,       QFig,       AFace,       JFace,       NULL },
        { "Gesicht zurueck",                     "H8",     2500, 0,           PFaceBack,   QFig,       AFaceBack,   JFaceBack,   NULL },
        { "Lebenspunkte -1",                     "GT",     1500, 0,           NULL,        QFig,       AHp,         JHp,         CHp },
        { "Max-Lebenspunkte +5",                 "GT",     1500, 0,           NULL,        QFig,       AHpMax,      JHpMax,      CHpMax },
        { "Faustkampf-Modus",                    "GT",     1500, 0,           NULL,        QFig,       AFist,       JFist,       NULL },
        { "Waffe weg",                           "GT",     1500, 0,           NULL,        QFig,       ANoWeapon,   JNoWeapon,   NULL },
        { "Nahkampfwaffe anlegen",               "GT",     1500, 0,           NULL,        QFig,       AMelee,      JMelee,      NULL },
        { "Nahkampfwaffe ziehen",                "GT",     1500, 0,           PMeleeDraw,  QFig,       AMeleeDraw,  JMeleeDraw,  NULL },
        { "Nahkampfwaffe wegstecken",            "GT",     1500, 0,           NULL,        QFig,       ANoWeapon,   JNoWeapon,   NULL },
        { "Nahkampfwaffe ablegen",               "GT",     1500, 0,           NULL,        QFig,       AMeleeOff,   JMeleeOff,   NULL },
        { "Fernkampfwaffe anlegen",              "GT",     1500, 0,           NULL,        QFig,       ARanged,     JRanged,     NULL },
        { "Fernkampfwaffe ziehen",               "GT",     1500, 0,           PRangedDraw, QFig,       ARangedDraw, JRangedDraw, NULL },
        { "Fernkampfwaffe wegstecken",           "GT",     1500, 0,           NULL,        QFig,       ANoWeapon,   JNoWeapon,   NULL },
        { "Fernkampfwaffe ablegen",              "GT",     1500, 0,           NULL,        QFig,       ARangedOff,  JRangedOff,  NULL },
        { "Ruestung anlegen",                    "GT",     1500, 0,           NULL,        QFig,       AArmor,      JArmor,      NULL },
        { "Ruestung ablegen",                    "GT",     1500, 0,           NULL,        QFig,       AArmorOff,   JArmorOff,   NULL },
        { "Talent Einhand",                      "GT",     1500, 0,           NULL,        QFig,       ATalent,     JTalent,     CTalent },
        { "Schutz gegen Klingen +5",             "GT",     1500, 0,           NULL,        QFig,       AProt,       JProt,       CProt },
        { "Overlay an",                          "B4",     1500, 0,           POverlay,    QFig,       AOverlay,    JOverlay,    NULL },
        { "Overlay ab",                          "B4",     1500, 0,           POverlayOff, QFig,       AOverlayOff, JOverlayOff, NULL },
        { "Geste (Animation)",                   "L5",      300, 0,           NULL,        QFig,       AGesture,    JGesture,    NULL },
        { "Staerke bleibt beim eigenen Helden",  "D10",    2000, CTF_ONCE,           NULL,        QHero,      AStr,        JStr,        CStr },
        { "EP ohne Skript bleiben lokal",        "D11",    2000, CTF_ONCE,           NULL,        QHero,      AExpLocal,   JExpLocal,   CExpLocal },
        // 3 Story und Tagebuch
        { "Tagebuch: Thema + Eintrag (Dialog)",  "D4/D8",  2000, 0,           PDiary,      QTopic,     ATopic,      JTopic,      NULL },
        { "Themenstatus erfolgreich",            "D7",     2500, 0,           PDiary,      QTopic,     AStatusDone, JStatusDone, NULL },
        { "Themenstatus laufend",                "D7",     2500, 0,           PDiary,      QTopic,     AStatusRun,  JStatusRun,  NULL },
        { "Eintrag ausserhalb eines Dialogs",    "D7",     3000, 0,           PDiary,      QTopic,     AEntryTick,  JEntryTick,  NULL },
        { "Quest-EP +10 (Dialog)",               "D11",    2000, 0,           PStory,      QExp,       AQuestXp,    JQuestXp,    NULL },
        { "Skript-Variable (Story-Takt)",        "D7",     2500, 0,           PVar,        QVarTest,   AVarTick,    JVar,        NULL },
        { "Skript-Variable (Dialog)",            "D4",     1500, 0,           PVar,        QVarTest,   AVarDialog,  JVar,        NULL },
        { "Skript-Variable zurueck",             "D7",     2500, 0,           PVar,        QVarTest,   AVarBack,    JVar,        NULL },
        { "Heldenvariable bleibt lokal",         "D10",    2500, CTF_ONCE,           PHeroVar,    QVarHero,   AHeroVar,    JHeroVar,    CHeroVar },
        { "Ruf +1 mit Meldung",                  "D17",    2500, CTF_OTHELLO, PRep,        QVarRep,    ARep,        JRep,        NULL },
        { "Ruf zurueck",                         "D17",    2500, CTF_OTHELLO, PRepBack,    QVarRep,    ARepBack,    JRepBack,    NULL },
        { "Gildenhaltung Troll/Golem",           "D9",     2500, 0,           PGuild,      QGuild,     AGuild,      JGuild,      NULL },
        { "Gildenhaltung zurueck",               "D9",     2500, 0,           PGuildBack,  QGuild,     AGuildBack,  JGuildBack,  NULL },
        { "Gildenhaltung mit Gilde -1",          "P133",    300, CTF_ONCE,    PGuildNeg,   QBase,      AGuildNeg,   JGuildNeg,   NULL },      // u136
        // u86 (P110): Gilden je Held (Variante C) - Beitritt, Gilden-Quest mit Belohnung und erfuelltem Thema, fremde Gilde
        { "Gilden: Partner in Testgilde",        "P110",   1500, 0,           PGld,        QGldSet,    ANone,       JGldSet,     NULL },
        { "Gilden: Quest gleicher Gilde",        "P110",   2500, 0,           PGld,        QGld,       AGldSame,    JGldSame,    NULL },
        { "Gilden: Quest anderer Gilde",         "P110",   2500, CTF_ONCE,    PGld,        QGld,       AGldOther,   JGldOther,   NULL },
        { "Gilden: Aufnahme bleibt beim Helden", "P110",   2500, CTF_ONCE,    PGld,        QGld,       AGldJoin,    JGldJoin,    NULL },
        { "Gilden: kein Story-Takt danach",      "P110",   3000, CTF_ONCE,    PGld,        QGld,       AGldTick,    JGldTick,    NULL },
        { "Gilden: zurueck",                     "P110",   2500, 0,           PGld,        QGldBack,   AGldBack,    JGldBack,    NULL },
        // u96 (P118): Missions-Gegenstaende spiegeln
        { "Missions-Gegenstand gespiegelt",      "P118",   2500, 0,           PMi,         QMi,        AMiGive,     JMiGive,     NULL },
        { "Missions-Gegenstand gekennzeichnet",  "P126",    800, 0,           PMiMark,     QMi,        ANone,       JMiMark,     NULL },   // u104
        { "Missions-Gegenstand abgegeben",       "P118",   2500, 0,           PMi,         QMi,        AMiTake,     JMiTake,     NULL },
        // u97 (P119): gemeinsame Coop-Truhe im Inventar
        { "Coop-Truhe: Inhalt beim Partner",     "P119",   2500, 0,           PTruhe,      QTruhe,     ATruheRein,  JTruheRein,  NULL },
        { "Coop-Truhe: benutzen oeffnet sie",    "P119",   3500, CTF_LONG,    PTruheAuf,   QTruhe,     ATruheAuf,   JTruheAuf,   NULL },
        // u99 (P121): fuer beide gleichzeitig offen
        { "Coop-Truhe: beide offen",             "P121",   2500, CTF_LONG,    PTruhe,      QTruheBeide, ATruheBeide, JTruheBeide, NULL },
        // u103 (P125): zaehlt bei Quests (Npc_HasItems/Npc_RemoveInvItems fuer den eigenen Helden)
        { "Coop-Truhe: zaehlt bei Quests",       "P125",   2500, CTF_LONG,    PTruheQuest, QTruhe,     ATruheQuest, JTruheQuest, NULL },
        { "Coop-Truhe: beide nehmen zugleich",   "P121",   3500, CTF_LONG,    PTruheGleich, QTruheGleich, ATruheGleich, JTruheGleich, NULL },
        { "Coop-Truhe: beide zu, gleicher Inhalt", "P121", 2500, CTF_LONG,    PTruhe,      QTruheZu,   ATruheZu,    JTruheZu,    NULL },
        { "Kapitel gleich",                      "D16",     100, 0,           NULL,        QVarChapter, ANone,      JChapter,    NULL },
        { "Alle Skript-Variablen gleich",        "D7",      300, CTF_ONCE,           PStory,      QDigVars,   ANone,       JDigVars,    NULL },
        { "Ganzes Tagebuch gleich",              "D4/D7",   300, CTF_ONCE,           PDiary,      QDigLog,    ANone,       JDigLog,     NULL },
        { "Gildentabelle gleich",                "D9",      300, CTF_ONCE,           PStory,      QDigGuilds, ANone,       JDigGuilds,  NULL },
        { "Tagesablauf/Haltung aller NPCs",      "D9",      300, CTF_ONCE,           PStory,      QDigNpcs,   ANone,       JDigNpcs,    NULL },
        // 4 Test-NPCs und Welt-Ereignisse
        { "Test-NPC einfuegen (Molerat)",        "D13",    3500, 0,           PMolSpawn,   QCountMol,  AMolSpawn,   JMolSpawn,   NULL },
        { "Test-NPC beim Partner gefuehrt",      "I1/I5",  2500, 0,           PMol,        QMol,       ANone,       JMolSync,    NULL },
        { "Test-NPC: Lage",                      "I1",      300, 0,           PMol,        QMol,       ANone,       JMolPos,     NULL },
        { "Test-NPC: Blickrichtung",             "L3",      300, 0,           PMol,        QMol,       ANone,       JMolHead,    NULL },
        { "Test-NPC: Leben (Schwierigkeit)",     "H16b",    300, 0,           PMol,        QMol,       ANone,       JMolHpMax,   NULL },
        { "Test-NPC treffen",                    "C1",     2500, 0,           PMol,        QMol,       AMolHit,     JMolHit,     NULL },
        { "Test-NPC toeten + EP-Regel",          "C1/I2",  3000, 0,           PMol,        QMolExp,    AMolKill,    JMolKill,    NULL },
        { "Test-NPC bleibt tot (5 s)",           "I3",     5000, CTF_ONCE,           PMolDead,    QMol,       ANone,       JMolStillDead, NULL },
        { "Zweiter Test-NPC (Fleischwanze)",     "D13",    3500, 0,           PBugSpawn,   QCountBug,  ABugSpawn,   JBugSpawn,   NULL },
        { "Fleischwanze Haltung freundlich",     "D9",     2500, 0,           PBugAlive,   QBug,       ABugFriendly, JBugFriendly, NULL },
        { "Fleischwanze Haltung neutral",        "D9",     2500, 0,           PBugAlive,   QBug,       ABugNeutral, JBugNeutral, NULL },
        { "Fleischwanze unsterblich (Dialog)",   "P138",   1500, 0,           PBugAlive,   QBug,       ABugImmortal, JBugImmortal, NULL },   // u139
        { "Fleischwanze Skriptfelder (Takt)",    "P138",   2500, 0,           PBugAlive,   QBug,       ABugFieldsTick, JBugFieldsTick, NULL },
        { "Fleischwanze Skriptfelder zurueck",   "P138",   2500, 0,           PBugAlive,   QBug,       ABugFieldsBack, JBugFieldsBack, NULL },
        { "Fleischwanze toeten",                 "C1",     3000, 0,           PBugAlive,   QBug,       ABugKill,    JBugKill,    NULL },
        { "Gegenstand der Nachricht schon frei", "P142",    100, 0,           PMsgVob,     QBase,      AMsgVob,     JMsgVob,     NULL },   // u142
        { "Tote NPCs gleich",                    "I3",      300, CTF_ONCE,           NULL,        QDigDead,   ANone,       JDigDead,    NULL },
        { "Trigger an den Partner",              "D13",    2000, 0,           PTrig,       QTrig,      ATrig,       JTrig,       NULL },
        { "Untrigger an den Partner",            "D13",    2000, 0,           PTrig,       QTrig,      AUntrig,     JUntrig,     NULL },
        { "Skript: Wld_InsertItem ueber den Host", "D13d", 2500, 0,           PInsertItem, QItems,     AInsertItem, JInsertItem, NULL },   // u27
        { "Kampf-Uebergabe einig",               "C18",     100, 0,           NULL,        QLead,      ANone,       JLead,       NULL },
        // 5 NPCs um mich
        { "Verfolgungspunkt beim Partner",       "I1",     1500, 0,           NULL,        QFar,       AFar,        JFar,        NULL },
        { "NPCs um mich beim Partner",           "I5",      300, 0,           NULL,        QNear,      ANone,       JNear,       NULL },
        { "Lebenspunkte der NPCs um mich",       "GT",      100, 0,           NULL,        QNear,      ANone,       JNearHp,     NULL },
        { "Waffenmodus der NPCs um mich",        "GT",      100, 0,           NULL,        QNear,      ANone,       JNearWm,     NULL },
        { "Blickrichtung der NPCs um mich",      "L3",      100, 0,           NULL,        QNear,      ANone,       JNearHead,   NULL },
        { "Animationen der NPCs um mich",        "L5",      100, 0,           NULL,        QNear,      ANone,       JNearAni,    NULL },
        { "Tot oder lebendig um mich",           "I3",      100, 0,           NULL,        QNear,      ANone,       JNearDead,   NULL },
        // 6 Gegenstaende und Truhen
        { "Truhe: Apfel hinein",                 "E1/E3",  2500, 0,           PChest,      QChest,     AChestIn1,   JChestIn1,   NULL },
        { "Truhe: Apfel heraus",                 "E1/E3",  2500, 0,           PChestAgain, QChest,     AChestOut1,  JChestOut1,  NULL },
        { "Truhe: drei Aepfel hinein",           "E1/E3",  2500, 0,           PChestAgain, QChest,     AChestIn3,   JChestIn3,   NULL },
        { "Truhe: drei Aepfel heraus",           "E1/E3",  2500, 0,           PChestAgain, QChest,     AChestOut3,  JChestOut3,  NULL },
        { "Truhe: ganzer Inhalt gleich",         "E1",      300, 0,           PChestAgain, QChest,     ANone,       JChestAll,   NULL },
        { "Alle Truhen der Welt gleich",         "E4",      300, CTF_ONCE,           PWorld,      QDigChests, ANone,       JDigChests,  NULL },
        { "Mechanismus: Hebel an, Tor geht auf", "P134",   2500, 0,           PMobNew,     QMob,       AMobOn,      JMobOn,      NULL },      // u137
        { "Mechanismus: Hebel aus",              "P134",   2500, 0,           PMobAgain,   QMob,       AMobOff,     JMobOff,     NULL },
        { "Mechanismus: Hebel an, Tor geht zu",  "P134",   2500, 0,           PMobAgain,   QMob,       AMobOn,      JMobOn2,     NULL },
        { "Mechanismus: Hebel wieder aus",       "P134",   2500, 0,           PMobAgain,   QMob,       AMobOff,     JMobOff,     NULL },
        { "Mechanismus: Skript und Kamera nur beim Benutzer", "P134", 100, CTF_ONCE, PMobChain, QBase,  AMobChain,   JMobChain,   NULL },
        { "Alle Mechanismen der Welt gleich",    "P134",    300, CTF_ONCE,    PWorld,      QDigMobs,   ANone,       JDigMobs,    NULL },
        { "Gegenstaende am Boden um mich",       "E4",      300, CTF_ONCE,           PWorld,      QDigLoose,  ANone,       JDigLoose,   NULL },
        { "Aufheben: verschwindet beim Partner", "E2",     2500, 0,           PPick,       QPick,      APick,       JPick,       NULL },
        { "Welt-Abgleich neu anfordern",         "E4/H1",  6000, CTF_CLIENT | CTF_ONCE,  PWorld,      QDigLoose,  AWorldAgain, JDigLoose,   NULL },
        // 7 Coop-Funktionen
        { "Teleport-Rune beim Partner",          "N1",      100, 0,           NULL,        QRune,      ANone,       JRune,       NULL },
        { "Rune mit eigenem Aussehen",           "N1c",     100, 0,           NULL,        QRune,      ANone,       JRuneLook,   NULL },
        { "Mitspieler-Markierung zeigt mich",    "O1/O2",   100, 0,           NULL,        QMark,      ANone,       JMark,       NULL },
        { "Zuschau-Fenster an",                  "K2",     1500, 0,           PWin,        QBase,      AWin,        JWin,        NULL },
        { "Zuschau-Fenster aus",                 "K2",     1000, 0,           PWinOff,     QBase,      AWinOff,     JWinOff,     NULL },
        { "Zuschau-Fenster, Partner weit weg",   "P132",   5000, CTF_LONG,    PWinFar,     QBase,      AWinFar,     JWinFar,     NULL },      // u135
        { "Gemeinsam speichern, Fenster offen",  "P132",   4000, CTF_LONG,    PSaveFar,    QSave,      ASave,       JSave,       CWinFar },   // u135
        // 8 Speichern und Laden (u30) - am Ende: das Laden baut die Welt neu auf
        { "Auto-Sicherung alle 3 min",           "P127",   3000, CTF_LONG,    PAuto,       QAuto,      AAuto,       JAuto,       CAuto },   // u105
        { "Kamerafahrt laeuft beim Speichern",   "P136",    500, CTF_ONCE,    PCamSave,    QBase,      ACamSave,    JCamSave,    NULL },   // u138
        { "Gemeinsam speichern",                 "P69",    4000, CTF_LONG,    PSave,       QSave,      ASave,       JSave,       NULL },
        { "Gemeinsam laden",                     "P69",    8000, CTF_LOAD,    PLoad,       QLoad,      ALoad,       JLoad,       CLoad },
        { "Nach dem Laden: Kamerafahrt gestoppt", "P136",   300, CTF_LONG,    PCamLoad,    QBase,      ANone,       JCamLoad,    NULL },   // u138
        { "Nach dem Laden: Held erhalten",       "P69",     300, CTF_LONG,    PAfterLoad,  QSave,      ANone,       JHeroKept,   NULL },
        { "Nach dem Laden: Figuren wie beim Speichern",          "P69",     300, CTF_LONG,    PAfterLoad,  QBase,      ANone,       JFigsAfterLoad, NULL },
        { "Nach dem Laden: Welten wie beim Speichern",        "P69",     100, CTF_LONG,    PAfterLoad,  QSave,      ANone,       JWorldAfterLoad, NULL },
        // 9 Getrennte Welten und Weltprozess (u39) - der Partner geht in eine andere Welt, am Ende Ausgangslage
        { "Partner in eine andere Welt schicken", "P82",   3000, CTF_PARTNERWAIT, NULL,    QBase,      ASendAway,   JAway,       NULL },
        { "Getrennte Welten erkannt",            "P65",     100, CTF_SEPWORLD, NULL,       QBase,      ANone,       JSepWorlds,  NULL },   // u30
        { "Keine Figur in der anderen Welt",     "P65",     100, CTF_SEPWORLD, NULL,       QBase,      ANone,       JSepFigs,    NULL },
        { "Keine NPCs aus der anderen Welt",     "P65",     100, CTF_SEPWORLD, NULL,       QBase,      ANone,       JSepNpcs,    NULL },
        { "Wer fuehrt die NPCs (getrennt)",      "P66",     100, CTF_SEPWORLD, NULL,       QBase,      ANone,       JWhoLeads,   NULL },
        // u40: Abgleich ueber Welten - Nutzer (Test u39): "Wir haben jedoch immer noch nicht die Synchronisation ueber Welten hinweg
        // getestet". Was fuer alle Welten gilt (Uhrzeit, Story, Tagebuch, Skript-Variablen, Gilden, Kapitel, Quest-EP) muss auch
        // ankommen, wenn der Partner woanders ist; der eigene Held bleibt eigen. Dieselben Handlungen wie Bereich 1-3, eigene Kennung.
        { "Spieltag und Uhrzeit (ueber Welten)", "GT",      100, CTF_SEPWORLD, NULL,       QBase,      ANone,       JDayTime,    NULL },
        { "Uhrzeit +30 min (ueber Welten)",      "GT",     2000, CTF_SEPWORLD | CTF_HOST, NULL, QBase, ATime,       JTime,       NULL },
        { "Staerke bleibt eigen (ueber Welten)", "D10",    2000, CTF_SEPWORLD | CTF_ONCE, NULL, QHero, AStr,        JStr,        CStr },
        { "EP ohne Skript lokal (ueber Welten)", "D11",    2000, CTF_SEPWORLD | CTF_ONCE, NULL, QHero, AExpLocal,   JExpLocal,   CExpLocal },
        { "Tagebuch-Eintrag Dialog (ueber Welten)", "D4/D8", 2000, CTF_SEPWORLD, PDiary,  QTopic,     ATopic,      JTopic,      NULL },
        { "Themenstatus erfolgreich (ueber Welten)", "D7", 2500, CTF_SEPWORLD, PDiary,     QTopic,     AStatusDone, JStatusDone, NULL },
        { "Themenstatus laufend (ueber Welten)", "D7",     2500, CTF_SEPWORLD, PDiary,     QTopic,     AStatusRun,  JStatusRun,  NULL },
        { "Eintrag ohne Dialog (ueber Welten)",  "D7",     3000, CTF_SEPWORLD, PDiary,     QTopic,     AEntryTick,  JEntryTick,  NULL },
        { "Quest-EP +10 (ueber Welten)",         "D11",    2000, CTF_SEPWORLD, PStory,     QExp,       AQuestXp,    JQuestXp,    NULL },
        { "Skript-Variable Takt (ueber Welten)", "D7",     2500, CTF_SEPWORLD, PVar,       QVarTest,   AVarTick,    JVar,        NULL },
        { "Skript-Variable Dialog (ueber Welten)", "D4",   1500, CTF_SEPWORLD, PVar,       QVarTest,   AVarDialog,  JVar,        NULL },
        { "Skript-Variable zurueck (ueber Welten)", "D7",  2500, CTF_SEPWORLD, PVar,       QVarTest,   AVarBack,    JVar,        NULL },
        { "Heldenvariable lokal (ueber Welten)", "D10",    2500, CTF_SEPWORLD | CTF_ONCE, PHeroVar, QVarHero, AHeroVar, JHeroVar,   CHeroVar },
        { "Gildenhaltung (ueber Welten)",        "D9",     2500, CTF_SEPWORLD, PGuild,     QGuild,     AGuild,      JGuild,      NULL },
        { "Gildenhaltung zurueck (ueber Welten)", "D9",    2500, CTF_SEPWORLD, PGuildBack, QGuild,     AGuildBack,  JGuildBack,  NULL },
        { "Kapitel gleich (ueber Welten)",       "D16",     100, CTF_SEPWORLD, NULL,       QVarChapter, ANone,      JChapter,    NULL },
        { "Alle Skript-Variablen (ueber Welten)", "D7",     300, CTF_SEPWORLD | CTF_ONCE, PStory, QDigVars, ANone,     JDigVars,    NULL },
        { "Ganzes Tagebuch (ueber Welten)",      "D4/D7",   300, CTF_SEPWORLD | CTF_ONCE, PDiary, QDigLog, ANone,      JDigLog,     NULL },
        { "Gildentabelle (ueber Welten)",        "D9",      300, CTF_SEPWORLD | CTF_ONCE, PStory, QDigGuilds, ANone,   JDigGuilds,  NULL },
        { "Weltprozess bereit",                  "P80",    2000, CTF_SEPWORLD | CTF_PARTNERWAIT, PWp, QBase, ANone,      JWpReady,    NULL },
        { "Zuschau-Fenster zeigt den Weltprozess", "P80",  1500, CTF_SEPWORLD | CTF_LONG, PWinWp, QBase,   AWinWp,      JWinWp,      NULL },
        { "Zuschau-Fenster (Weltprozess) aus",   "P80",    1000, CTF_SEPWORLD,  PWinOff,    QBase,      AWinOff,     JWinOff,     NULL },
        { "Kamerafahrt beim Speichern (getrennte Welten)", "P136", 500, CTF_SEPWORLD | CTF_ONCE, PCamSave, QBase, ACamSave, JCamSave, NULL },   // u138
        { "Gemeinsam speichern (getrennte Welten)", "P69", 4000, CTF_SEPWORLD | CTF_LONG, PSave, QSave,    ASave,       JSaveApart,  NULL },
        { "Weltstand des Partners im Spielstand", "P81",   2000, CTF_SEPWORLD | CTF_PARTNERWAIT, PApart, QWx, ANone,     JWxSlot,     NULL },
        { "Sicherung beim Partner (getrennte Welten)", "P102", 2000, CTF_SEPWORLD | CTF_PARTNERWAIT, PBk, QBk, ANone,    JBk,         NULL },   // u55
        { "Sicherung: Partner verliert seine Kopie (Test)", "P102", 300, CTF_SEPWORLD | CTF_HOST | CTF_ONCE, PBkHide, QBkHide, ANone, JBkHide, NULL },
        { "Gemeinsam laden (getrennte Welten)",  "P71",    8000, CTF_SEPWORLD | CTF_LOAD, PLoad, QLoad,    ALoad,       JLoad,       CLoad },
        { "Nach dem Laden (getrennt): Kamerafahrt gestoppt", "P136", 300, CTF_SEPWORLD | CTF_LONG, PCamLoad, QBase, ANone, JCamLoad,  NULL },   // u138
        { "Nach dem Laden (getrennt): Held erhalten", "P71", 300, CTF_SEPWORLD | CTF_LONG, PAfterLoad, QSave, ANone,    JHeroKept,   NULL },
        { "Nach dem Laden: Partner holt seine Kopie zurueck", "P102", 300, CTF_SEPWORLD | CTF_HOST | CTF_LONG, PBkRestored, QBk, ANone, JBkRestored, NULL },
        { "Nach dem Laden (getrennt): keine Figuren", "P71", 300, CTF_SEPWORLD | CTF_LONG, PAfterLoad, QBase, ANone,    JFigsAfterLoad, NULL },
        { "Nach dem Laden (getrennt): Welten wie beim Speichern", "P71", 100, CTF_SEPWORLD | CTF_LONG, PAfterLoad, QSave, ANone, JWorldAfterLoad, NULL },
        { "Nach dem Laden: Weltstand in Saves\\current", "P81", 300, CTF_SEPWORLD | CTF_LONG, PAfterLoad, QWx, ANone,  JWxCurrent,  NULL },
        // u47 (P88): Betreten - der Stand dessen gilt, der dort ist bzw. zuletzt dort war
        { "Betreten: Partner markiert seine Welt", "P88",  2000, CTF_SEPWORLD | CTF_LONG, PMarkPartner, QEnterMark, AMarkPartner, JMarkPartner, NULL },
        { "Betreten: ich komme in seine Welt",    "P88",   4000, CTF_LOAD | CTF_PARTNERWAIT, PEnter, QBase,  AEnter,      JEnter,      CLoad },
        { "Betreten: Eintritts-Merker kamen mit", "P139",   100, 0,           PEnter,      QBase,      ANone,       JEntryVar,   NULL },   // u140
        { "Drache aus altem Spielstand heilt",    "P140",   100, 0,           PDragonHeal, QBase,      ADragonHeal, JDragonHeal, NULL },
        { "Betreten: zurueck in meine Welt",      "P88",   3000, CTF_LOAD | CTF_PARTNERWAIT, PBack,  QBase,  ABack,       JBack,       CLoad },
        { "Betreten: Variable aus dem Weltwechsel", "P139", 4500, CTF_SEPWORLD | CTF_LONG, PVar, QVarTest,  ANone,       JVar,        NULL },   // u140
        { "Betreten: Variable zurueck",           "P139",  2500, CTF_SEPWORLD, PVar,       QVarTest,   AVarBack,    JVar,        NULL },
        { "Betreten: ich markiere meine Welt",    "P88",   1500, CTF_SEPWORLD | CTF_LONG, PMarkMine, QBase,   ANone,       JMarkMine,   NULL },
        { "Teleport zu mir ueber Welten",        "N2",     3000, CTF_PARTNERWAIT, PTpHere, QBase,      ATpHere,     JTpHere,     NULL },
        { "Betreten: Partner sieht meinen Stand", "P88",    500, CTF_LONG,    PMarkSeen,   QMarkChk,   ANone,       JMarkSeen,   NULL },
        { "Eintritts-Merker in derselben Welt",   "P141",  2500, 0,           PEntrySame,  QEntrySame, AEntrySame,  JEntrySame,  CEntrySame },   // u141
        { "Ausgangslage wiederherstellen",       "P82",    3000, CTF_PARTNERWAIT, NULL,    QBase,      ARestore,    JRestore,    NULL },
    };
    static const size_t CoopTestCount = sizeof(CoopTests) / sizeof(CoopTests[0]);

    static const CoopTestBlock CoopTestBlocks[] = {
        { "Partner antwortet (Rundlauf)",        "Verbindung und Grundstand" },
        { "Figuren gegenseitig in der Welt",     "Eigene Figur beim Partner" },
        { "Tagebuch: Thema + Eintrag (Dialog)",  "Story und Tagebuch" },
        { "Test-NPC einfuegen (Molerat)",        "Test-NPCs und Welt-Ereignisse" },
        { "Verfolgungspunkt beim Partner",       "NPCs um mich" },
        { "Truhe: Apfel hinein",                 "Gegenstaende und Truhen" },
        { "Teleport-Rune beim Partner",          "Coop-Funktionen" },
        { "Auto-Sicherung alle 3 min",           "Speichern und Laden" },   // u30; u105 mit der Auto-Sicherung vorn
        { "Partner in eine andere Welt schicken", "Getrennte Welten und Weltprozess" },   // u39
    };
    static const int CoopTestBlockCount = sizeof(CoopTestBlocks) / sizeof(CoopTestBlocks[0]);

    // u30: erster Schritt eines Bereichs (Nummer aus dem Namen; bis u29 feste Nummern - u29 fuegte einen Schritt ein, seitdem lag
    // jeder Bereich um einen Schritt verschoben: Ueberschriften und Bereichszahlen am Ende stimmten nicht)
    static size_t CTBlockFirst(int b) {
        static size_t first[16];
        static bool done = false;
        if (!done) {
            done = true;
            for (int i = 0; i < CoopTestBlockCount && i < 16; i++) {
                first[i] = CoopTestCount;
                for (size_t j = 0; j < CoopTestCount; j++)
                    if (!strcmp(CoopTests[j].name, CoopTestBlocks[i].firstStep)) {
                        first[i] = j;
                        break;
                    }
            }
        }
        return b >= 0 && b < CoopTestBlockCount && b < 16 ? first[b] : CoopTestCount;
    }

    // u30 (P65): Schritte, die auch gelten, wenn der Partner in einer anderen Welt ist - sie brauchen weder seine Figur noch NPCs,
    // Truhen oder Gegenstaende derselben Welt (Grundstand, Heldenwerte, Story/Tagebuch/Gilden, Rune im Inventar). Alle anderen
    // melden dann "--". Die Schritte mit CTF_SEPWORLD laufen nur in diesem Fall.
    static const char* const CTAnyWorldSteps[] = {
        "Partner antwortet (Rundlauf)", "Fassung gleich", "Wer fuehrt die NPCs des Clients", "Spieltag und Uhrzeit gleich",
        "Uhrzeit +30 min", "Abgleich-Regeln gleich", "Schwierigkeitsgrad gleich", "Engine-Aenderungen wie beim Partner",
        "Gemeinsamer Durchlauf", "Gemeinsames Speichern moeglich", "Netz ohne Rueckstau", "Absturzschutz aktiv", "Log-Versand",
        "Staerke bleibt beim eigenen Helden", "EP ohne Skript bleiben lokal",
        "Tagebuch: Thema + Eintrag (Dialog)", "Themenstatus erfolgreich", "Themenstatus laufend", "Eintrag ausserhalb eines Dialogs",
        "Quest-EP +10 (Dialog)", "Skript-Variable (Story-Takt)", "Skript-Variable (Dialog)", "Skript-Variable zurueck",
        "Heldenvariable bleibt lokal", "Ruf +1 mit Meldung", "Ruf zurueck", "Gildenhaltung Troll/Golem", "Gildenhaltung zurueck",
        "Gildenhaltung mit Gilde -1",   // u136 (P133)
        "Gilden: Partner in Testgilde", "Gilden: Quest gleicher Gilde", "Gilden: Quest anderer Gilde",   // u86 (P110)
        "Gilden: Aufnahme bleibt beim Helden", "Gilden: kein Story-Takt danach", "Gilden: zurueck",
        "Missions-Gegenstand gespiegelt", "Missions-Gegenstand gekennzeichnet", "Missions-Gegenstand abgegeben",   // u96 (P118), u104
        "Coop-Truhe: Inhalt beim Partner", "Coop-Truhe: benutzen oeffnet sie", "Coop-Truhe: beide offen",   // u97/u99: ohne Welt
        "Coop-Truhe: zaehlt bei Quests", "Coop-Truhe: beide nehmen zugleich", "Coop-Truhe: beide zu, gleicher Inhalt",   // u103
        "Kapitel gleich", "Alle Skript-Variablen gleich", "Ganzes Tagebuch gleich", "Gildentabelle gleich",
        "Teleport-Rune beim Partner", "Rune mit eigenem Aussehen",
        // Speichern und Laden: gerade bei getrennten Welten wichtig - danach sind beide in der Welt des Hosts
        "Auto-Sicherung alle 3 min",   // u105
        "Kamerafahrt laeuft beim Speichern", "Nach dem Laden: Kamerafahrt gestoppt",   // u138 (P136)
        "Gemeinsam speichern", "Gemeinsam laden", "Nach dem Laden: Held erhalten", "Nach dem Laden: Figuren wie beim Speichern",
        "Nach dem Laden: Welten wie beim Speichern",
        // u39: Welten fuer den Test wechseln
        "Partner in meine Welt holen", "Partner in eine andere Welt schicken", "Teleport zu mir ueber Welten", "Ausgangslage wiederherstellen",
        "Story-Video: Ueberspringen gilt fuer beide",   // u55: das Video laeuft beim Partner in jeder Welt
        // u47 (P88): Betreten - ich wechsle in die Welt des Partners und zurueck
        "Betreten: ich komme in seine Welt", "Betreten: Eintritts-Merker kamen mit", "Drache aus altem Spielstand heilt", "Betreten: zurueck in meine Welt", "Betreten: Partner sieht meinen Stand",
    };

    static bool CTAnyWorld(const CoopTestDef& t) {
        if (t.flags & CTF_SEPWORLD)
            return true;
        for (const char* n : CTAnyWorldSteps)
            if (!strcmp(n, t.name))
                return true;
        return false;
    }

    // Anzahl Schritte, die bei getrennten Welten "--" melden; Tippfehler in der Liste ins Log
    static int CTOtherWorldSkipCount() {
        for (const char* n : CTAnyWorldSteps) {
            bool found = false;
            for (size_t i = 0; i < CoopTestCount && !found; i++)
                found = !strcmp(CoopTests[i].name, n);
            if (!found)
                CTLog("Schritte fuer getrennte Welten: \"%s\" ist kein Schritt (Tippfehler?)", n);
        }
        int n = 0;
        for (size_t i = 0; i < CoopTestCount; i++)
            if (!CTAnyWorld(CoopTests[i]))
                n++;
        return n;
    }

    // u12 (Gothic 2): Schritte, deren Modul in dieser Engine noch nicht freigegeben ist (CoopCore.cpp, G2-FEHLT.md), melden "--" mit dem
    // Modulnamen statt FEHLER. Zuordnung ueber den Schrittnamen; der Start meldet Eintraege ohne passenden Schritt. Allgemein - in G1
    // sind alle Module frei, dort aendert sich nichts.
    struct CTModNeed {
        const char* step;
        bool ready;
        const char* mod;
    };
    static const CTModNeed CTModNeeds[] = {
        { "Abgleich-Regeln gleich",              COOP_READY_Difficulty != 0, "Schwierigkeit/Regeln" },
        { "Schwierigkeitsgrad gleich",           COOP_READY_Difficulty != 0, "Schwierigkeit/Regeln" },
        { "Engine-Aenderungen wie beim Partner", COOP_READY_Audit != 0,      "Selbstpruefung" },
        { "Gemeinsamer Durchlauf",               COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Gemeinsames Speichern moeglich",      COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Skript: Wld_InsertItem ueber den Host", COOP_READY_World != 0 && COOP_READY_Dialog != 0, "Welt-Abgleich" },   // u27: Host-Ereignis im Dialog-Modul
        { "Kampf-Uebergabe einig",               COOP_READY_CombatLead != 0, "Kampf-Uebergabe" },
        { "Truhe: Apfel hinein",                 COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Truhe: Apfel heraus",                 COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Truhe: drei Aepfel hinein",           COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Truhe: drei Aepfel heraus",           COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Truhe: ganzer Inhalt gleich",         COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Alle Truhen der Welt gleich",         COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Gegenstaende am Boden um mich",       COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Aufheben: verschwindet beim Partner", COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Welt-Abgleich neu anfordern",         COOP_READY_World != 0,      "Welt-Abgleich" },
        { "Teleport-Rune beim Partner",          COOP_READY_Teleport != 0,   "Teleport-Rune" },
        { "Rune mit eigenem Aussehen",           COOP_READY_Teleport != 0 && (ENGINE == Engine_G1 || ENGINE == Engine_G2A), "Rune-Modell" },   // u17 nur G1, u21 auch G2A
        { "Mitspieler-Markierung zeigt mich",    COOP_READY_Marker != 0,     "Markierung" },
        { "Auto-Sicherung alle 3 min",           COOP_READY_Save != 0 && COOP_READY_Difficulty != 0, "Speichern/Laden" },   // u105
        { "Kamerafahrt laeuft beim Speichern",   COOP_READY_Save != 0 && COOP_READY_World != 0, "Speichern/Laden" },   // u138
        { "Nach dem Laden: Kamerafahrt gestoppt", COOP_READY_Save != 0 && COOP_READY_World != 0, "Speichern/Laden" },
        { "Kamerafahrt beim Speichern (getrennte Welten)", COOP_READY_Save != 0 && COOP_READY_World != 0, "Speichern/Laden" },
        { "Nach dem Laden (getrennt): Kamerafahrt gestoppt", COOP_READY_Save != 0 && COOP_READY_World != 0, "Speichern/Laden" },
        { "Gemeinsam speichern",                 COOP_READY_Save != 0,       "Speichern/Laden" },   // u30
        { "Gemeinsam laden",                     COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden: Held erhalten",       COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden: Figuren wie beim Speichern",          COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden: Welten wie beim Speichern",        COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Gemeinsam speichern (getrennte Welten)",           COOP_READY_Save != 0,       "Speichern/Laden" },   // u39
        { "Weltstand des Partners im Spielstand",             COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Sicherung beim Partner (getrennte Welten)",        COOP_READY_Save != 0,       "Speichern/Laden" },   // u55
        { "Sicherung: Partner verliert seine Kopie (Test)",   COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden: Partner holt seine Kopie zurueck", COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Gemeinsam laden (getrennte Welten)",               COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden (getrennt): Held erhalten",         COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden (getrennt): keine Figuren",         COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Nach dem Laden (getrennt): Welten wie beim Speichern", COOP_READY_Save != 0,   "Speichern/Laden" },
        { "Nach dem Laden: Weltstand in Saves\\current",      COOP_READY_Save != 0,       "Speichern/Laden" },
        { "Zuschau-Fenster zeigt den Weltprozess",            COOP_READY_Camera != 0,     "Zuschau-Fenster" },
        { "Zuschau-Fenster (Weltprozess) aus",                COOP_READY_Camera != 0,     "Zuschau-Fenster" },
        { "Teleport zu mir ueber Welten",                     COOP_READY_Teleport != 0,   "Teleport-Rune" },
    };

    static const char* CTModuleOff(const CoopTestDef& t) {
        for (const CTModNeed& m : CTModNeeds)
            if (!m.ready && !strcmp(m.step, t.name))
                return m.mod;
        return NULL;
    }

    static int CTModuleOffCount() {
        int n = 0;
        for (const CTModNeed& m : CTModNeeds) {
            bool found = false;
            for (size_t i = 0; i < CoopTestCount && !found; i++)
                found = !strcmp(CoopTests[i].name, m.step);
            if (!found)
                CTLog("Zuordnung Schritt -> Modul: \"%s\" ist kein Schritt (Tippfehler?)", m.step);
            else if (!m.ready)
                n++;
        }
        return n;
    }

    // u44 (D12i): Welten-Testlauf - Nutzer (Test u41): "außerdem wollte ich ja die synchronisation zwischen zwei welten testen,
    // die ersten 100 tests etwa sind überflüssig". Laeuft: Bereich 1 bis "Partner in meine Welt holen" (Verbindung, Fassung, der
    // Partner kommt her) und Bereich 9 ganz (er geht weg, Abgleich ueber Welten, Weltprozess, Speichern/Laden, Teleport, Ausgangslage).
    // Shift+F11 = Welten-Testlauf, Strg+Shift+F11 = alle 144 Schritte (die bestandenen bleiben fuer spaetere Pruefungen erreichbar).
    // u50: Pruefrunde fuer lange Laeufe - nur Schritte ohne Handlung, die den Stand beider Seiten vergleichen
    static bool CTIsCheckStep(size_t i) {
        static const char* names[] = {
            "Partner antwortet (Rundlauf)", "Fassung gleich", "Welt gleich", "Weltschluessel beim Partner", "Wer fuehrt die NPCs des Clients",
            "Spieltag und Uhrzeit gleich", "Abgleich-Regeln gleich", "Schwierigkeitsgrad gleich", "Gemeinsamer Durchlauf", "Netz ohne Rueckstau",
            "Figuren gegenseitig in der Welt", "Lage meiner Figur", "Name meiner Figur", "Aussehen meiner Figur", "Kapitel gleich",
            "Alle Skript-Variablen gleich", "Ganzes Tagebuch gleich", "Gildentabelle gleich", "Tagesablauf/Haltung aller NPCs",
            "Tote NPCs gleich", "Verfolgungspunkt beim Partner", "NPCs um mich beim Partner", "Lebenspunkte der NPCs um mich",
            "Waffenmodus der NPCs um mich", "Blickrichtung der NPCs um mich", "Animationen der NPCs um mich", "Tot oder lebendig um mich",
            "Alle Truhen der Welt gleich", "Gegenstaende am Boden um mich", "Mitspieler-Markierung zeigt mich",
            "Getrennte Welten erkannt", "Keine Figur in der anderen Welt", "Keine NPCs aus der anderen Welt", "Wer fuehrt die NPCs (getrennt)",
            "Spieltag und Uhrzeit (ueber Welten)", "Kapitel gleich (ueber Welten)", "Alle Skript-Variablen (ueber Welten)",
            "Ganzes Tagebuch (ueber Welten)", "Gildentabelle (ueber Welten)", "Weltprozess bereit" };
        bool listed = false;
        for (const char* n : names)
            if (!strcmp(CoopTests[i].name, n))
                listed = true;
        if (!listed)
            return false;
        // u50 (Dauertest Lauf 2: bei getrennten Welten liefen "Welt gleich" und "Figuren gegenseitig in der Welt" mit und schlugen
        // fehl): getrennt nur die Schritte fuer getrennte Welten, sonst nur die anderen; Rundlauf und Fassung immer
        const bool always = !strcmp(CoopTests[i].name, "Partner antwortet (Rundlauf)") || !strcmp(CoopTests[i].name, "Fassung gleich");
        const bool sep = (CoopTests[i].flags & CTF_SEPWORLD) != 0;
        return always || sep == CT.otherWorld;
    }

    static bool CTInRun(size_t i) {
        if (CT.checksOnly)
            return CTIsCheckStep(i);
        if (!CT.worldsOnly && !CT.guildsOnly)
            return true;
        static size_t fetch = CoopTestCount;
        if (fetch == CoopTestCount)
            for (size_t j = 0; j < CoopTestCount; j++)
                if (!strcmp(CoopTests[j].name, "Partner in meine Welt holen")) {
                    fetch = j;
                    break;
                }
        if (CT.guildsOnly)   // u86: bis zum Herholen, Regeln gleich, die Gilden-Schritte, am Ende die Ausgangslage
            return i <= fetch || !strcmp(CoopTests[i].name, "Abgleich-Regeln gleich") || !strncmp(CoopTests[i].name, "Gilden: ", 8) ||
                !strncmp(CoopTests[i].name, "Missions-Gegenstand", 19) || !strncmp(CoopTests[i].name, "Coop-Truhe", 10) ||
                !strncmp(CoopTests[i].name, "Truhe: ", 7) || !strcmp(CoopTests[i].name, "Alle Truhen der Welt gleich") ||
                !strncmp(CoopTests[i].name, "Mechanismus: ", 13) || !strcmp(CoopTests[i].name, "Alle Mechanismen der Welt gleich") ||   // u137 (P134)
                !strcmp(CoopTests[i].name, "Auto-Sicherung alle 3 min") ||   // u105 (P127)
                !strcmp(CoopTests[i].name, "Zuschau-Fenster an") || !strcmp(CoopTests[i].name, "Zuschau-Fenster aus") ||   // u135 (P132): zweite Kamera
                !strcmp(CoopTests[i].name, "Zuschau-Fenster, Partner weit weg") || !strcmp(CoopTests[i].name, "Gemeinsam speichern, Fenster offen") ||
                !strcmp(CoopTests[i].name, "Gildenhaltung mit Gilde -1") ||   // u136 (P133)
                !strcmp(CoopTests[i].name, "Tagesablauf/Haltung aller NPCs") || !strcmp(CoopTests[i].name, "Zweiter Test-NPC (Fleischwanze)") ||   // u139 (P138)
                !strncmp(CoopTests[i].name, "Fleischwanze ", 13) || !strcmp(CoopTests[i].name, "Gegenstand der Nachricht schon frei") ||   // u142 (P142)
                i + 1 == CoopTestCount;   // u96 (P118), u97 (P119), u98 (P120: Welt-Truhen - der Partner nimmt ohne Remove heraus) mit
        return i <= fetch || i >= CTBlockFirst(CoopTestBlockCount - 1);
    }

    // u45: Nummer im laufenden Lauf (Welten-Testlauf 1..39 statt der festen Nummer von 144 - Nutzer: "es waren 144 tests wie letztes
    // mal"; der Lauf war der neue, die Anzeige "001/144", "109/144" sah aber wie der alte aus)
    static unsigned CTRunPos(size_t idx) {
        unsigned n = 0;
        for (size_t i = 0; i <= idx && i < CoopTestCount; i++)
            if (CTInRun(i))
                n++;
        return n;
    }

    static unsigned CTRunTotal() {
        return CTRunPos(CoopTestCount - 1);
    }

    static int CTBlockOf(size_t idx) {
        int b = 0;
        for (int i = 0; i < CoopTestBlockCount; i++)
            if (idx >= CTBlockFirst(i))
                b = i;
        return b;
    }

    // --- Ablauf --------------------------------------------------------------------------------------------------------

    static void CTSend(const char* f, const json& extra) {
        json j = extra;
        j["type"] = SYNC_SELFTEST;
        j["id"] = MyselfId.ToChar();
        j["f"] = f;
        CoopTraceCount(TRACE_SENT, SYNC_SELFTEST);
        ReadyToSendJsons.enqueue(j);
    }

    static void CTAsk(const char* ph) {
        json x;
        x["run"] = CT.tag;
        x["n"] = (int)CT.idx;
        x["ph"] = ph;
        x["from"] = DialogMe();
        x["q"] = CoopTests[CT.idx].query();
        CT.askMs = GetCurrentMs();
        if (!strcmp(ph, "post"))
            CT.myHeadAsk = GetHeading(player);
        CTSend("q", x);
    }

    static void CTCount(int& total, int* perBlock) {
        total++;
        perBlock[CTBlockOf(CT.idx)]++;
    }

    static void CTResult(const char* what, const std::string& d) {
        const CoopTestDef& t = CoopTests[CT.idx];
        const char* oth = (t.flags & CTF_OTHELLO) ? ", nur Othello" : "";
        const std::string again = CT.retries ? CTFmt(" [%d. Nachfrage, %.1f s nach der Handlung]", CT.retries, (CurrentMs - CT.actMs) / 1000.f) : std::string();
        // u45: Welten-Testlauf zaehlt 1..39 (Log zusaetzlich mit der festen Schrittnummer)
        const bool part = CT.worldsOnly || CT.guildsOnly;   // u86: auch der Gilden-Testlauf
        const unsigned pos = part ? CTRunPos(CT.idx) : (unsigned)(CT.idx + 1);
        const unsigned total = part ? CTRunTotal() : (unsigned)CoopTestCount;
        const char* label = CT.guildsOnly ? "Gilden-Test" : CT.worldsOnly ? "Welten-Test" : "Coop-Test";
        const std::string fixed = part ? CTFmt(" [Schritt %u]", (unsigned)(CT.idx + 1)) : std::string();
        CTLog("%02u/%u %s  %s (%s%s): %s%s%s", pos, total, what, t.name, t.ref, oth, d.c_str(), again.c_str(), fixed.c_str());
        // Chat: OK kurz, alles andere mit Begruendung (100 Zeilen - die Einzelheiten stehen im Log)
        char buf[400];
        if (!strcmp(what, "OK"))
            _snprintf(buf, sizeof(buf), "%s %02u/%u OK  %s", label, pos, total, t.name);
        else
            _snprintf(buf, sizeof(buf), "%s %02u/%u %s  %s: %s", label, pos, total, what, t.name, d.c_str());
        buf[sizeof(buf) - 1] = 0;
        ChatLog(string(buf));
    }

    // alles zuruecknehmen, was ein Schritt am eigenen Helden oder an der Welt veraendert hat und noch offen ist (Abbruch)
    static void CTRestoreAll() {
        if (!player)
            return;
        if (CT.headTurned) {
            player->RotateWorldY(-90.f);
            CT.headTurned = false;
        }
        if (CT.nameChanged && zoptions) {
            IniSetStr("coopName", CT.nameOld);
            CT.nameChanged = false;
        }
        if (CT.faceChanged && zoptions) {
            IniSetInt("coopFace", CT.faceIniOld);
            CT.faceChanged = false;
        }
        if (CT.overlayOn) {
            player->RemoveOverlay(zSTRING(CT.overlay.c_str()));
            CT.overlayOn = false;
        }
        if ((CT.weapon || CT.ranged) && player->GetWeaponMode() != NPC_WEAPON_NONE)
            player->SetWeaponMode2(NPC_WEAPON_NONE);
        CTDropOwn(CT.weapon);
        CTDropOwn(CT.ranged);
        CTDropOwn(CT.armor);
        CTDropAmmo();
        if (CT.varSet) {
            if (zCPar_Symbol* s = CTVarSym())
                s->SetValue(0, 0);
            CT.varSet = false;
        }
        if (CT.gldValue) {   // u86: Abbruch mitten in den Gilden-Schritten
            if (zCPar_Symbol* s = CTVarSym()) {
                DlgGuildVars.erase(s);
                s->SetValue(0, 0);
            }
            CT.gldValue = 0;
        }
        CHeroVar();
        if (CT.repSet) {
            if (zCPar_Symbol* s = parser->GetSymbol(zSTRING("REPUTATIONOC")))
                s->SetValue(CT.repOld, 0);
            CT.repSet = false;
        }
        if (CT.guildSet) {
            std::string w;
            AGuildBack(w);
        }
        if (CT.talOld >= 0)
            CTalent();
    }

    static void CTFinish(bool aborted) {
        if (aborted && CT.idx < CoopTestCount && (CT.phase == CT_WAIT || CT.phase == CT_POST) && CoopTests[CT.idx].cleanup)
            CoopTests[CT.idx].cleanup();
        CTRestoreAll();
        CT.on = false;
        if (CoopCamWindowOn())
            CoopCamStop("Coop-Test beendet");
        CTSay("Coop-Testlauf %s %s: %d OK, %d FEHLER, %d uebersprungen, %d bekannte Luecke", CT.tag.c_str(),
            aborted ? "abgebrochen" : "fertig", CT.ok, CT.fail, CT.skip, CT.gap);
        {   // u49: dem Partner das Ende melden (CoopAutoTest.cpp startet danach dessen Lauf; aeltere Fassungen ignorieren "end")
            json x;
            x["run"] = CT.tag;
            x["from"] = DialogMe();
            x["t"] = CTFmt("%s %s: %d OK, %d FEHLER, %d uebersprungen, %d bekannte Luecke", CT.tag.c_str(), aborted ? "abgebrochen" : "fertig",
                CT.ok, CT.fail, CT.skip, CT.gap);
            CTSend("end", x);
        }
        for (int b = 0; b < CoopTestBlockCount; b++) {
            const size_t first = CTBlockFirst(b);
            const size_t end = b + 1 < CoopTestBlockCount ? CTBlockFirst(b + 1) : CoopTestCount;
            unsigned n = 0;   // u43: im Welten-Testlauf nur die Schritte, die laufen
            for (size_t i = first; i < end; i++)
                if (CTInRun(i))
                    n++;
            if (!n)
                continue;
            CTSay("  %d %s: %d/%u OK, %d FEHLER, %d --, %d bekannt", b + 1, CoopTestBlocks[b].title, CT.bOk[b], n,
                CT.bFail[b], CT.bSkip[b], CT.bGap[b]);
        }
        for (auto& f : CT.fails)
            CTLog("  FEHLER: %s", f.c_str());
        CTSay("Von Hand pruefen: Zielwechsel (C3), Wiederbeleben F4 (C5), Dialoge (D3-D6), Fallenlassen (E8), Kapitel-Bildschirm (D16), "
            "Teleport wirken (N1, N2 ueber Welten), Kartenpfeile (O4), Weltwechsel (P73 gemeinsam: einer geht durch und bestaetigt, der "
            "andere kommt mit), Bild des Weltprozesses im Zuschau-Fenster ansehen (P80)");
    }

    static void CTStart(bool worldsOnly, bool checksOnly, bool guildsOnly = false);

    void CoopSelfTestToggle(bool worldsOnly) {
        if (CT.on) {
            CTFinish(true);
            return;
        }
        CTStart(worldsOnly, false);
    }

    // u86 (P110): Gilden-Testlauf (CoopAutoTest.cpp, -cooptest:host:<Slot>:gilden)
    void CoopSelfTestGuildRun() {
        if (!CT.on)
            CTStart(false, false, true);
    }

    // u50: Pruefrunde fuer lange Laeufe - true = laeuft
    bool CoopSelfTestPartnerSettled();
    static bool CTFigureInWorld();

    bool CoopSelfTestCheckRun() {
        if (CT.on || !CoopSelfTestPartnerSettled())   // u50: nicht mitten in einem Weltwechsel oder Ladevorgang
            return false;
        CTStart(false, true);
        return CT.on;
    }

    bool CoopSelfTestRunning() {
        return CT.on;
    }

    static void CTStart(bool worldsOnly, bool checksOnly, bool guildsOnly) {
        if (!parser || zCParser::GetParser() != parser || !player || !ogame) {
            CTSay("Coop-Test: Spiel nicht bereit");
            return;
        }
        // u30 (Test u29: zweimal "kein Mitspieler verbunden", obwohl verbunden - der Partner war im Minental, seine Figur steht
        // seit P65 nur in derselben Welt): ist er in einer anderen Welt, laufen die weltunabhaengigen Schritte und die Pruefung der
        // Trennung, der Rest meldet "--"
        const std::vector<std::string> away = CoopWorldsElsewhere();
        bool otherWorld = PlayerNpcs.empty() && !away.empty();
        if (checksOnly) {   // u50: Pruefrunde nach Weltkennung und echter Figur (nicht nach der Spielerliste)
            otherWorld = !away.empty();
            if (!otherWorld && !CTFigureInWorld()) {
                CTLog("Pruefrunde: Partner weder sicher hier noch sicher in einer anderen Welt - nicht gestartet");
                return;
            }
        }
        if (PlayerNpcs.empty() && !otherWorld) {
            CTSay((ServerThread || ClientThread) ? "Coop-Test: keine Figur eines Mitspielers da (verbunden, aber noch im Menue oder beim Laden?)"
                : "Coop-Test: kein Mitspieler verbunden");
            return;
        }
        // v108 (Test v107: beide Laeufe liefen gleichzeitig und stoerten sich - Spawns, Wartezeiten, eigene Handlungen)
        if (GetCurrentMs() - CTPartnerQueryMs < 10000) {
            CTSay("Coop-Test: der Partner testet gerade (Lauf %s) - erst nach dessen Ende starten", CTPartnerRun.c_str());
            return;
        }
        SYSTEMTIME st;
        GetLocalTime(&st);
        char tag[64];
        _snprintf(tag, sizeof(tag), "T%02d%02d%02d-%s", st.wHour, st.wMinute, st.wSecond, MyselfId.ToChar());
        tag[sizeof(tag) - 1] = 0;
        CT = CoopTestRun();
        CT.on = true;
        CT.tag = tag;
        CT.otherWorld = otherWorld;
        CT.startedApart = otherWorld;   // u39
        CT.worldsOnly = worldsOnly;     // u43
        CT.checksOnly = checksOnly;     // u50
        CT.guildsOnly = guildsOnly;     // u86
        CTHomeKept = false;
        if (otherWorld)
            CT.otherText = away[0];
        CT.loadsSeen = CoopWorldLoads;   // u30
        int ms = 0;
        unsigned steps = 0;
        for (size_t i = 0; i < CoopTestCount; i++)
            if (CTInRun(i)) {
                ms += CoopTests[i].waitMs + 600;
                steps++;
            }
        if (guildsOnly)   // u86: nur Herholen und Zurueckschicken
            ms += otherWorld ? 2 * 25000 : 0;
        else
            ms += (otherWorld ? 4 : 3) * 25000 + 40000;   // u39: Weltwechsel des Partners (je etwa 25 s), Weltprozess laden
        if (checksOnly) {
            CTSay("Pruefrunde %s: %u Vergleiche mit dem Partner (%s)", CT.tag.c_str(), steps, otherWorld ? "getrennte Welten" : "gleiche Welt");
            return;
        }
        if (guildsOnly)
            CTSay("Gilden-Testlauf %s startet: %u Schritte (Verbindung, Partner herholen, dann Gilden je Held P110: Belohnung und erfuellte "
                "Quest gleicher Gilde, Quest anderer Gilde, Aufnahme, Story-Takt), etwa %d min - Shift+F11 bricht ab (nur Test-Spielstand!)",
                CT.tag.c_str(), steps, (ms + 30000) / 60000);
        else if (worldsOnly)
            CTSay("Welten-Testlauf %s startet: %u Schritte (Verbindung, Partner herholen, dann Bereich 9: getrennte Welten, Abgleich ueber "
                "Welten, Weltprozess, Speichern/Laden, Teleport), etwa %d min - Shift+F11 bricht ab, Strg+Shift+F11 = alle 144 Schritte "
                "(nur Test-Spielstand!)", CT.tag.c_str(), steps, (ms + 30000) / 60000);
        else
            CTSay("Coop-Testlauf %s startet: alle %u Schritte in %d Bereichen, etwa %d min - Shift+F11 bricht ab (nur Test-Spielstand!)",
                CT.tag.c_str(), (unsigned)CoopTestCount, CoopTestBlockCount, (ms + 30000) / 60000);
        CTOtherWorldSkipCount();   // prueft nur noch die Namensliste (Tippfehler ins Log)
        if (otherWorld) {
            CTSay("Partner in einer anderen Welt (%s, hier %s): der Test holt ihn gleich her (Schritt 3), am Ende geht er zurueck",
                CT.otherText.c_str(), CoopWorldName().c_str());
        }
        else {
            const float pd = CTPartnerDist();
            CTSay("Abstand zur Figur des Partners: %.0f m (%s; Reichweite %d m)", pd / 100.f,
                pd > (float)BROADCAST_DISTANCE ? "fern - Verfolgungspunkt wird mitgeprueft" : "nah", BROADCAST_DISTANCE / 100);
        }
        if (const int off = CTModuleOffCount())   // u12 (G2A)
            CTSay("%d Schritte melden \"--\": ihr Modul ist in dieser Engine noch aus (docs\\G2-FEHLT.md)", off);
    }

    bool CoopTeleportAcross(const std::string& to, std::string& why);   // u39: CoopTeleport.cpp

    // u39: Antwortseite - Weltwechsel bzw. Teleport ueber Welten auf Wunsch des Testenden (Paket "go", CTGoOrder)
    static void CTGoPulse() {
        if (!CTGo.on || !player || !ogame || IsLoadingLevel)
            return;
        if (CTGo.phase == 0) {
            const bool busy = zCMenu::GetActive() != NULL || IsPlayerTalkingWithAnybody();
            if (busy && CurrentMs < CTGo.since + 30000)
                return;   // Menue/Dialog erst zu Ende (hoechstens 30 s)
            if (CTGo.mark) {   // u47 (P88): eigene Welt markieren - einsetzen, im naechsten Bild toeten
                std::string why;
                if (!CTMarkMade.insertedMs) {
                    if (!CTMarkStart(CTMarkMade, why)) {
                        CTLog("Testlauf %s: Markierung NICHT moeglich: %s", CTGo.run.c_str(), why.c_str());
                        CTGo.on = false;
                    }
                    else
                        CTMarkMade.entryVar = CTEntryProbe();   // u140 (P139)
                    return;
                }
                const int r = CTMarkFinish(CTMarkMade);
                if (r != 0) {
                    CTLog("Testlauf %s: Markierung %s", CTGo.run.c_str(), r > 0 ? "fertig (Fleischwanze tot, Apfel am Boden)" :
                        "gescheitert - die eingesetzte Fleischwanze nicht gefunden");
                    CTGo.on = false;
                }
                return;
            }
            if (CTGo.keep) {
                CTHomeSet = true;
                CTHomeWorld = CoopWorldName();
                CTHomePos = player->GetPositionWorld();
                CTHomeAt = player->GetAtVectorWorld();
                CTLog("Testlauf %s: merke meine Welt %s und Lage (fuer die Rueckkehr)", CTGo.run.c_str(), CTHomeWorld.c_str());
            }
            if (CTGo.tp) {   // Teleport-Rune ueber Welten - der Rest laeuft in CoopTeleport.cpp
                std::string why;
                const bool ok = CoopTeleportAcross(CTGo.from, why);
                CTLog("Testlauf %s: Teleport ueber Welten zu %s %s%s", CTGo.run.c_str(), CTGo.from.c_str(), ok ? "gestartet" : "NICHT moeglich: ",
                    ok ? "" : why.c_str());
                CTGo.on = false;
                return;
            }
            CTGo.target = CTGo.world;
            if (CTGo.target.empty()) {
                if (!CTHomeSet) {
                    CTLog("Testlauf %s: zurueck - keine gemerkte Welt", CTGo.run.c_str());
                    CTGo.on = false;
                    return;
                }
                CTGo.target = CTHomeWorld;
                CTGo.pos = CTHomePos;
                CTGo.at = CTHomeAt;
                CTGo.havePos = true;
                CTGo.exact = true;
            }
            if (CoopWcInTarget(CTGo.target)) {
                CTGo.phase = 2;
                CTGo.loadedMs = CurrentMs - 1500;
                return;
            }
            CTGo.loadsAt = CoopWorldLoads;
            CTGo.since = CurrentMs;
            CTGo.phase = 1;
            CTLog("Testlauf %s: wechsle nach %s (Wunsch von %s)", CTGo.run.c_str(), CTGo.target.c_str(), CTGo.from.c_str());
            CoopWcGo(CTGo.target, "");   // wie beim gemeinsamen Weltwechsel, ohne Abfrage; Startpunkt der Welt
            return;
        }
        if (CTGo.phase == 1) {
            if (CoopWorldLoads != CTGo.loadsAt && CoopWcInTarget(CTGo.target)) {
                CTGo.phase = 2;
                CTGo.loadedMs = CurrentMs;
            }
            else if (CurrentMs > CTGo.since + 120000) {
                CTLog("Testlauf %s: Weltwechsel nach %s nach 120 s nicht fertig - aufgegeben", CTGo.run.c_str(), CTGo.target.c_str());
                CTGo.on = false;
            }
            return;
        }
        if (CurrentMs < CTGo.loadedMs + 1500)
            return;   // erst ankommen lassen
        std::string how = "Startpunkt der Welt";
        if (CTGo.havePos && CTGo.exact) {
            player->SetCollDet(FALSE);
            player->SetPositionWorld(CTGo.pos);
            player->SetCollDet(TRUE);
            player->SetHeadingAtWorld(CTGo.at);
            PendingMoveReset = true;
            how = "gemerkte Lage";
        }
        else if (CTGo.havePos) {
            PendingMoveReset = true;
            if (!CoopPlaceBesideHost(CTGo.pos, CTGo.at, how, "den Testenden"))
                PendingMoveReset = false;
        }
        CTLog("Testlauf %s: in %s angekommen (%s)", CTGo.run.c_str(), CoopWorldName().c_str(), how.c_str());
        CTGo.on = false;
    }

    // u50 (Dauertest u50, Lauf 1: Pruefrunde nach einem Weltwechsel des Partners - "Getrennte Welten erkannt" lief, obwohl beide in
    // Khorinis waren; die gemeldete Welt war beim Start noch die alte): wo der Partner ist, muss seit 15 s gleich sein
    static std::string CTPartnerWhere;
    static long long CTPartnerWhereMs = 0;
    static int CTPartnerWhereLoads = -1;

    // u50: Figur eines Mitspielers wirklich in meiner Welt (Dauertest Lauf 2: PlayerNpcs hatte noch die Figur, die nach einem
    // Weltwechsel nicht mehr in der Welt stand - die Pruefrunde verglich dann "gleiche Welt" mit einem Partner im Minental)
    static bool CTFigureInWorld() {
        for (auto& kv : PlayerNpcs)
            if (kv.first && kv.first->GetHomeWorld() && kv.first->vobLeafList.GetNum() > 0)
                return true;
        return false;
    }

    static void CTTrackPartner() {
        std::string w = "-";
        const std::vector<std::string> away = CoopWorldsElsewhere();
        if (!away.empty())
            w = "weg:" + away[0];
        else if (CTFigureInWorld())
            w = "hier";
        if (w != CTPartnerWhere || CoopWorldLoads != CTPartnerWhereLoads) {
            CTPartnerWhere = w;
            CTPartnerWhereLoads = CoopWorldLoads;
            CTPartnerWhereMs = GetCurrentMs();
        }
    }

    bool CoopSelfTestPartnerSettled() {
        return CTPartnerWhere != "-" && GetCurrentMs() - CTPartnerWhereMs > 15000;
    }

    void CoopSelfTestPulse() {
        CTGoPulse();   // u39: auch ohne eigenen Lauf (Antwortseite)
        CTTrackPartner();   // u50
        if (!CT.on)
            return;
        // u30: eigener Ladevorgang seit dem letzten Bild (der Takt laeuft waehrend des Ladens nicht - IsLoadingLevel unten sah ihn
        // nie). Bis u29 lief der Lauf danach mit Zeigern in die alte Welt weiter (vom Test angelegte Gegenstaende, Truhe). Jetzt:
        // vergessen; erwartet nur im Schritt "Gemeinsam laden", sonst Abbruch.
        if (CT.loadsSeen != CoopWorldLoads) {
            CT.loadsSeen = CoopWorldLoads;
            CTForgetWorld();
            if (!CT.expectLoad) {
                CTSay("Coop-Test: ein Ladevorgang mitten im Testlauf (Partner oder Weltwechsel) - abgebrochen");
                CT.phase = CT_GAP;   // ohne Zuruecknehmen des laufenden Schritts (die Welt ist eine andere)
                CTFinish(true);
                return;
            }
            CT.expectLoad = false;
            CT.loadedMs = CurrentMs;
            CTLog("Testsicherung geladen - weiter mit dem Lauf");
        }
        if (!player || !ogame || IsLoadingLevel) {
            CTFinish(true);
            return;
        }
        try {
            const CoopTestDef& t = CoopTests[CT.idx < CoopTestCount ? CT.idx : 0];
            switch (CT.phase) {
            case CT_START:
            {
                // u86: nach uebersprungenen Schritten erst im naechsten Bild weiter - t ist oben gebunden; bis u85 galten fuer den
                // ersten Schritt danach Vorbereitung und Merkmale des ersten uebersprungenen (allgemein)
                bool skipped = false;
                while (CT.idx < CoopTestCount && !CTInRun(CT.idx)) {   // u43: Welten-Testlauf ueberspringt Bereiche 1 (Rest) bis 8
                    CT.idx++;
                    skipped = true;
                }
                if (CT.idx >= CoopTestCount) {
                    CTFinish(false);
                    return;
                }
                if (skipped)
                    break;
                for (int b = 0; b < CoopTestBlockCount; b++) {
                    if (CTBlockFirst(b) != CT.idx)
                        continue;
                    if (CT.worldsOnly || CT.guildsOnly)   // u45: Welten-Testlauf - zwei Teile statt "Bereich 1/9", "Bereich 9/9"
                        CTSay("-- Teil %d/2: %s --", b == 0 ? 1 : 2, CoopTestBlocks[b].title);
                    else
                        CTSay("-- Bereich %d/%d: %s --", b + 1, CoopTestBlockCount, CoopTestBlocks[b].title);
                }
                if (CT.guildsOnly && !strncmp(t.name, "Gilden: ", 8) && CT.idx > 0 && strncmp(CoopTests[CT.idx - 1].name, "Gilden: ", 8))
                    CTSay("-- Teil 2/2: Gilden je Held (P110) --");
                std::string why;
                const char* off = CTModuleOff(t);   // u12: Modul in dieser Engine noch aus
                const std::string offText = off ? std::string("Modul ") + off + " in dieser Engine noch aus" : std::string();
                const std::string awayText = "Partner in einer anderen Welt (" + CT.otherText + ")";   // u30
                const char* only = (t.flags & CTF_HOST) && !ServerThread ? "nur am Host" :
                    (t.flags & CTF_CLIENT) && ServerThread ? "nur am Client" :
                    (t.flags & CTF_OTHELLO) && !CTOthello() ? "nur mit Othello" :
                    (t.flags & CTF_SEPWORLD) && !CT.otherWorld ? "nur bei getrennten Welten" :
                    CT.otherWorld && !CTAnyWorld(t) ? awayText.c_str() : off ? offText.c_str() : NULL;
                if (only || (t.prep && !t.prep(why))) {
                    CTCount(CT.skip, CT.bSkip);
                    CTResult("--", only ? std::string(only) : "uebersprungen: " + why);
                    CT.phase = CT_GAP;
                    CT.deadline = CurrentMs + 150;
                    break;
                }
                CT.havePre = CT.havePost = false;
                CT.retries = 0;
                CT.myHeadAsk = -1.f;
                CTAsk("pre");
                CT.phase = CT_PRE;
                CT.deadline = CurrentMs + 6000;
                break;
            }
            case CT_PRE:
                if (CT.havePre) {
                    std::string why;
                    CTLog("Schritt %03u: %s", (unsigned)(CT.idx + 1), t.name);   // vor der Handlung - zeigt bei Absturz den Schritt
                    if (!t.act(why)) {
                        CTCount(CT.skip, CT.bSkip);
                        CTResult("--", "uebersprungen: " + why);
                        CT.phase = CT_GAP;
                        CT.deadline = CurrentMs + 150;
                        break;
                    }
                    CT.actMs = CurrentMs;
                    CT.phase = CT_WAIT;
                    CT.deadline = CurrentMs + t.waitMs;
                }
                else if (CurrentMs > CT.deadline) {
                    CTCount(CT.fail, CT.bFail);
                    CT.fails.push_back(std::string(t.name) + ": Partner antwortet nicht");
                    CTResult("FEHLER", "Partner antwortet nicht (vorher)");
                    CT.phase = CT_GAP;
                    CT.deadline = CurrentMs + 300;
                }
                break;
            case CT_WAIT:
            {
                // u30: ein Ladeschritt fragt erst 4 s nach dem eigenen Ladevorgang (Figuren, Uebernahme beim Partner), spaetestens
                // nach 90 s; der Partner darf dann bis 20 s fuer die Antwort brauchen (er laedt evtl. noch)
                const bool load = (t.flags & CTF_LOAD) != 0;
                const bool ready = CurrentMs >= CT.deadline &&
                    (!load || (CT.loadedMs > CT.actMs && CurrentMs >= CT.loadedMs + 4000) || CurrentMs > CT.actMs + 90000);
                if (ready) {
                    CTAsk("post");
                    CT.phase = CT_POST;
                    CT.deadline = CurrentMs + (load ? 20000 : 6000);
                }
                break;
            }
            case CT_POST:
                if (CT.havePost || CurrentMs > CT.deadline) {
                    std::string d;
                    const bool ok = CT.havePost && t.judge(CT.pre, CT.post, d);
                    if (!CT.havePost)
                        d = "Partner antwortet nicht (nachher)";
                    // v108: "noch nicht" - bis 8 s (CTF_LONG 20 s) nach der Handlung alle 0,8 s noch einmal fragen
                    // u39: CTF_PARTNERWAIT bis 150 s alle 1,5 s, auch ohne Antwort (der Partner laedt gerade eine Welt)
                    const bool waitLong = (t.flags & CTF_PARTNERWAIT) != 0;
                    if (!ok && (CT.havePost || waitLong) && d.compare(0, 3, "-- ") != 0 && !(t.flags & (CTF_ONCE | CTF_GAP)) &&
                        CurrentMs - CT.actMs < (waitLong ? 150000 : (t.flags & CTF_LOAD) ? 90000 : (t.flags & CTF_LONG) ? 20000 : 8000)) {
                        if (CT.retries++ == 0)
                            CTLog("Schritt %03u: noch nicht (%s) - frage noch einmal", (unsigned)(CT.idx + 1), d.c_str());
                        else if (waitLong && CT.retries % 12 == 0)
                            CTSay("Coop-Test %03u wartet noch (%.0f s): %s", (unsigned)(CT.idx + 1), (CurrentMs - CT.actMs) / 1000.f, d.c_str());
                        CT.havePost = false;
                        CT.phase = CT_WAIT;
                        CT.deadline = CurrentMs + (waitLong ? 1500 : 800);
                        break;
                    }
                    if (ok) {
                        CTCount(CT.ok, CT.bOk);
                        CTResult(t.flags & CTF_GAP ? "OK (Luecke geschlossen?)" : "OK", d);
                    }
                    else if (d.compare(0, 3, "-- ") == 0) {   // Urteil "nicht bewertbar"
                        CTCount(CT.skip, CT.bSkip);
                        CTResult("--", d.substr(3));
                    }
                    else if (t.flags & CTF_GAP) {
                        CTCount(CT.gap, CT.bGap);
                        CTResult("BEKANNT", d + " - bekannte Luecke");
                    }
                    else {
                        CTCount(CT.fail, CT.bFail);
                        CT.fails.push_back(std::string(t.name) + ": " + d);
                        CTResult("FEHLER", d);
                    }
                    if (t.cleanup)
                        t.cleanup();
                    CT.phase = CT_GAP;
                    CT.deadline = CurrentMs + 300;
                }
                break;
            case CT_GAP:
                if (CurrentMs >= CT.deadline) {
                    CT.idx++;
                    CT.phase = CT_START;
                }
                break;
            }
        }
        catch (std::exception& ex) {
            CoopErrorLog("Coop-Test", ex.what(), CT.idx < CoopTestCount ? CoopTests[CT.idx].name : "-");
            CTCount(CT.fail, CT.bFail);
            CT.fails.push_back(std::string(CT.idx < CoopTestCount ? CoopTests[CT.idx].name : "-") + ": Ausnahme " + ex.what());
            if (CT.idx < CoopTestCount && (CT.phase == CT_WAIT || CT.phase == CT_POST) && CoopTests[CT.idx].cleanup)
                CoopTests[CT.idx].cleanup();
            CT.phase = CT_GAP;
            CT.deadline = CurrentMs + 500;
        }
    }

    bool CoopSelfTestHandles(int type) {
        return type == SYNC_SELFTEST;
    }

    void CoopSelfTestReceive(json& e, const std::string& id) {
        try {
            const std::string f = e.value("f", std::string());
            if (f == "q") {
                const std::string from = e.value("from", id);
                if (from == DialogMe())
                    return;
                CTPartnerQueryMs = GetCurrentMs();   // v108: Lauf des Partners aktiv
                CTPartnerRun = e.value("run", std::string());
                json a;
                a["run"] = e["run"];
                a["n"] = e["n"];
                a["ph"] = e["ph"];
                a["to"] = from;
                a["by"] = DialogMe();
                a["o"] = CoopTestObserve(e["q"], from);
                CTSend("a", a);
                if (e["ph"] == "pre" && e["n"].get<int>() == 0)
                    CTLog("Testlauf %s von %s - ich melde, was ich sehe", e["run"].get<std::string>().c_str(), from.c_str());
            }
            else if (f == "end") {   // u49: Lauf des Partners zu Ende
                const std::string from = e.value("from", id);
                if (from == DialogMe())
                    return;
                CTPartnerEndMs = GetCurrentMs();
                CTPartnerEndText = e.value("t", std::string());
                CTLog("Testlauf von %s zu Ende: %s", from.c_str(), CTPartnerEndText.c_str());
                CTGldRestore("Lauf des Partners zu Ende");   // u86: auch nach einem Abbruch nicht in der Testgilde bleiben
            }
            else if (f == "go") {   // u39: Weltwechsel bzw. Teleport fuer den Testlauf des Partners
                if (e.value("to", std::string()) != DialogMe())
                    return;
                if (e.contains("evoff") && e["evoff"].is_string()) {   // u140 (P139): Probe zuruecknehmen
                    CTEntryProbeOff(e["evoff"].get<std::string>());
                    return;
                }
                CTGo = CTGoOrder();
                CTGo.on = true;
                CTGo.run = e.value("run", std::string());
                CTGo.from = e.value("from", id);
                CTGo.world = e.value("w", std::string());
                CTGo.keep = e.value("keep", 0) != 0;
                CTGo.tp = e.value("tp", 0) != 0;
                CTGo.mark = e.value("mark", 0) != 0;   // u47
                if (e.contains("p") && e["p"].is_array() && e["p"].size() == 3) {
                    CTGo.pos = CTVecOf(e["p"]);
                    CTGo.at = (e.contains("a") && e["a"].is_array() && e["a"].size() == 3) ? CTVecOf(e["a"]) : zVEC3(0.f, 0.f, 1.f);
                    CTGo.havePos = true;
                }
                CTGo.since = GetCurrentMs();
                if (CTGo.mark)
                    CTMarkMade = CTMark();
                CTLog("Testlauf %s von %s: %s", CTGo.run.c_str(), CTGo.from.c_str(), CTGo.mark ? "meine Welt markieren (Betreten)" :
                    CTGo.tp ? "Teleport ueber Welten zu ihm" :
                    CTGo.world.empty() ? "zurueck in meine gemerkte Welt" : ("Weltwechsel nach " + CTGo.world).c_str());
            }
            else if (f == "a") {
                if (!CT.on || e.value("to", std::string()) != DialogMe() || e.value("run", std::string()) != CT.tag
                    || e.value("n", -1) != (int)CT.idx)
                    return;
                if (e["ph"] == "pre" && CT.phase == CT_PRE && !CT.havePre) {
                    CT.pre = e["o"];
                    CT.havePre = true;
                    CT.partner = e.value("by", CT.partner);
                }
                else if (e["ph"] == "post" && CT.phase == CT_POST && !CT.havePost) {
                    CT.post = e["o"];
                    CT.havePost = true;
                    CT.rttMs = (int)(GetCurrentMs() - CT.askMs);
                    CT.partner = e.value("by", CT.partner);
                }
            }
        }
        catch (std::exception& ex) {
            CoopErrorLog("Coop-Test", ex.what(), id.c_str());
        }
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)
    void CoopSelfTestToggle(bool) {}
    void CoopSelfTestGuildRun() {}   // u86
    bool CoopSelfTestCheckRun() { return false; }   // u50
    bool CoopSelfTestPartnerSettled() { return false; }
    bool CoopSelfTestRunning() { return false; }
    void CoopSelfTestPulse() {}
    bool CoopSelfTestHandles(int) { return false; }
    void CoopSelfTestReceive(json&, const std::string&) {}
#endif
}
