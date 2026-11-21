namespace GOTHIC_ENGINE {
    string ReviveState = "READY";
    oCNpc* ReviveNpc;
    long long ReviveStartTime = 0;

    // u282 (P363-P365, Nutzer 18.11.: "Man kann nicht sterben, wenn man eigentlich sterben wuerde liegt man so lange auf dem Boden, bis der
    // Kampf beendet ist oder der Partner einen aufhebt. Sollte der Partner auch 'sterben', dann sterben tatsaechlich beide und man muss
    // neuladen. Ausserdem soll der letzte Spielstand bzw. Sicherung geladen werden. Wenn ESC gedrueckt wird, dann wird nicht neugeladen ...
    // Das Speichern soll nicht moeglich sein, so lange einer auf dem Boden liegt. Die Zeit fuer das Aufheben des Partners soll auf 4 s
    // verkuerzt werden und man soll ganz normal aufstehen wie wenn man KO ist."):
    // - P363 Wird der eigene Held im Coop von einem Gegner getoetet (oCNpc::DoDie mit Taeter, Hooks.cpp), faellt er stattdessen bewusstlos um
    //   (DropUnconscious, Leben 1) und nimmt keinen Schaden mehr (OnDamage-Haken). Gothics/Othellos ZS_Unconscious endet, sobald
    //   Npc_GetStateTime > HAI_TIME_UNCONSCIOUS (20 s): solange er liegen bleibt, setzt GT die Zeit je Bild auf 0, zum Aufstehen auf 9999 -
    //   ZS_Unconscious_End steht dann mit AI_StandUp ganz normal wie nach einem K.o. auf. Nicht im Wasser (Ertrinken bleibt Tod); ohne
    //   Taeter (Sturz, Skript) bleibt es Tod (Wiederbeleben wie bisher); nicht verwandelt, nicht in der Pause, nur mit Mitspieler.
    //   Aufstehen, wenn der Kampf vorbei ist, entscheidet der Host (er fuehrt die NPCs): 3 s lang kein NPC im Kampf mit einem Coop-Spieler
    //   im Umkreis von 20 m, fruehestens 5 s nach dem Umfallen; beim Client schickt er "auf". Speichern geht nicht, solange einer liegt.
    // - P364 Liegen bzw. sind alle Coop-Spieler tot, sterben alle wirklich; der Host laedt nach 10 s (u291, P377: 5 s) den neuesten gemeinsamen Stand des
    //   Durchlaufs. Oeffnet einer vorher ein Menue (Esc), laedt GT nicht automatisch - er waehlt selbst einen Stand.
    // - P365 Aufheben durch den Partner in 4 s (bis u281 10 s), danach steht der Held normal auf (mindestens 30 % Leben).
    // Meldungen SYNC_LIEGEN: op "liegt"/"steht"/"esc" (jeder ueber sich), "auf" (Host an einen Client), "tot" (Host an alle).
    static bool CoopLiegtHier = false;                           // eigener Held liegt statt tot
    static bool CoopLiegtAuf = false;                            // Aufstehen angeordnet
    static bool CoopLiegenEcht = false;                          // GT laesst den eigenen Helden wirklich sterben
    static bool CoopUntenGemeldet = false;                       // den Mitspielern gemeldet: liege bzw. bin tot
    static long long CoopLiegtSeit = 0;
    static std::map<std::string, long long> CoopLiegtPartner;    // Mitspieler (Kennung) liegt bzw. ist tot seit
    static std::map<std::string, long long> CoopKampfFreiSeit;   // Host: seit wann um ihn kein Kampf mehr ist
    static std::map<std::string, long long> CoopAufGesendet;     // Host: "auf" an einen Client geschickt
    static long long CoopAlleTotMs = 0;                          // alle besiegt seit (Host laedt nach 10 s)
    static bool CoopAlleTotEsc = false;                          // ein Menue wurde geoeffnet - nicht automatisch laden
    static bool CoopKampfGeloggt = false;                        // u291 (P376): in dieser Liegen-Phase schon geloggt, wer kaempft
    static oCItem* CoopKoMelee = NULL;                           // u291 (P380): beim K.o. ausgeruestete Nahkampfwaffe
    static oCItem* CoopKoFern = NULL;                            // u291 (P380): beim K.o. ausgeruestete Fernkampfwaffe
    static long long CoopKoMerkMs = 0;                           // u296 (P405): wann zuletzt Waffen gemerkt wurden
    static int CoopKoMeleeInst = -1;
    static int CoopKoFernInst = -1;

    static void LiegenLog(const char* fmt, ...) {
        char buf[600];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Liegen: %s", DiagSeconds(), DiagRole(), buf);
    }

    // u291 (P380, Nutzer 18.11.: "Waffe sollte ausgeruestet bleiben, auch wenn man KO geht"): beim K.o. steckt GT die Waffe weg
    // (Hooks.cpp, P353). Hier merkt sich GT, was ausgeruestet war, und ruestet es nach dem Aufstehen wieder aus, falls es nicht mehr
    // ausgeruestet ist. Die Waffe wird nicht gezogen.
    void CoopKoWaffenMerken() {
        if (!player)
            return;
        CoopKoMerkMs = CurrentMs;   // u296 (P405)
        oCItem* nah = player->GetEquippedMeleeWeapon();
        oCItem* fern = player->GetEquippedRangedWeapon();
        if (nah && CoopInInventory(player, nah)) {
            CoopKoMelee = nah;
            CoopKoMeleeInst = nah->GetInstance();
        }
        if (fern && CoopInInventory(player, fern)) {
            CoopKoFern = fern;
            CoopKoFernInst = fern->GetInstance();
        }
        LiegenLog("K.o.: ausgeruestet merken - Nahkampf %s, Fernkampf %s (P380)", CoopKoMelee ? "ja" : "nein", CoopKoFern ? "ja" : "nein");
    }

    static void CoopKoWaffenPruefen() {
        if (!CoopKoMelee && !CoopKoFern)
            return;
        if (player) {
            oCItem* waffen[2] = { CoopKoMelee, CoopKoFern };
            const int inst[2] = { CoopKoMeleeInst, CoopKoFernInst };
            for (int i = 0; i < 2; i++) {
                oCItem* it = waffen[i];
                if (!it || !CoopInInventory(player, it) || it->GetInstance() != inst[i])
                    continue;
                oCItem* aktuell = i == 0 ? player->GetEquippedMeleeWeapon() : player->GetEquippedRangedWeapon();
                if (aktuell == it)
                    continue;
                player->EquipItem(it);
                LiegenLog("Waffe wieder ausgeruestet (P380): %s", it->GetInstanceName().ToChar());
            }
        }
        CoopKoMelee = CoopKoFern = NULL;
        CoopKoMeleeInst = CoopKoFernInst = -1;
    }

    static void CoopLiegenSenden(const char* op, const std::string& to = std::string()) {
        json j;
        j["type"] = SYNC_LIEGEN;
        j["id"] = MyselfId.ToChar();
        j["op"] = op;
        if (!to.empty())
            j["to"] = to;
        ReadyToSendJsons.enqueue(j);
    }

    static bool CoopLiegenMitspielerDa() {
        if (!(ServerThread || ClientThread))
            return false;
        for (auto& p : PlayerNpcs)
            if (p.first && p.first != player)
                return true;
        return false;
    }

    bool CoopLiegtEigen() {
        return CoopLiegtHier;
    }

    // Figur eines Mitspielers, der liegt bzw. tot ist (seine Meldung "liegt")
    bool CoopLiegtFigur(oCNpc* n) {
        return n && CoopLiegtPartner.count(n->GetObjectName().ToChar()) > 0;
    }

    static void CoopLiegenAufstehen(const char* warum, bool aufgehoben) {
        if (!CoopLiegtHier || !player)
            return;
        CoopLiegtAuf = true;
        if (aufgehoben) {
            const int soll = player->GetAttribute(NPC_ATR_HITPOINTSMAX) * 30 / 100;
            if (player->GetAttribute(NPC_ATR_HITPOINTS) < soll)
                player->SetAttribute(NPC_ATR_HITPOINTS, soll);
        }
        player->state.SetStateTime(9999);
        LiegenLog("eigener Held steht auf (%s)", warum);
    }

    // Hooks.cpp (oCNpc::DoDie am eigenen Helden): true = statt Tod hingelegt bzw. liegt schon
    bool CoopLiegenStattTod(oCNpc* killer) {
        if (!player || CoopLiegenEcht || IsCoopPaused || CoopHeroSwapped() || !CoopLiegenMitspielerDa())
            return false;
        if (CoopLiegtHier) {
            player->SetAttribute(NPC_ATR_HITPOINTS, 1);
            return true;   // liegt schon - kein zweiter Tod
        }
        if (!killer || killer == player)
            return false;   // Sturz, Skript: Tod wie bisher
        const int bs = player->GetBodyState();
        if (bs == BS_SWIM || bs == BS_DIVE)
            return false;   // Ertrinken bleibt Tod
        // u291 (P378, Nutzer 18.11.: "die Todesanimation soll kommen, aber auch nur wenn beide wirklich sterben wuerden"): liegen schon
        // alle anderen Coop-Spieler (oder sind tot), ist das der letzte Tod - Gothics normales DoDie laeuft mit Todesanimation
        bool andere = false, alleAndereUnten = true;
        for (auto& p : PlayerNpcs) {
            oCNpc* n = p.first;
            if (!n || n == player)
                continue;
            andere = true;
            if (!(CoopLiegtFigur(n) || n->IsDead()))
                alleAndereUnten = false;
        }
        if (andere && alleAndereUnten) {
            LiegenLog("eigener Held stirbt wirklich - alle Mitspieler liegen schon (P378)");
            return false;
        }
        player->SetAttribute(NPC_ATR_HITPOINTS, 1);
        CoopLiegtHier = true;
        CoopLiegtAuf = false;
        CoopKampfGeloggt = false;
        CoopLiegtSeit = CurrentMs;
        player->DropUnconscious(0.0f, killer);
        ChatLog(string((std::string(CoopTr("Du liegst am Boden - dein Mitspieler kann dich aufheben (", "You are down - your partner can pick you up (")) +
            CoopKeyText(CK_REVIVE) + CoopTr("). Ist der Kampf vorbei, stehst du wieder auf.", "). When the fight is over, you get back up.")).c_str()),
            zCOLOR(255, 160, 120, 255));
        LiegenLog("eigener Held liegt statt zu sterben (Taeter %s, Zustand %s)", killer->GetObjectName().ToChar(), player->state.curState.name.ToChar());
        return true;
    }

    // RemoteNpc.cpp (SYNC_REVIVED an mich, waehrend ich liege)
    void CoopLiegenWiederbelebt(oCNpc* helfer) {
        if (!CoopLiegtHier)
            return;
        CoopLiegenAufstehen("aufgehoben", true);
        ChatLog(string((std::string(helfer ? helfer->GetObjectName().ToChar() : "?") + CoopTr(" hat dich aufgehoben.", " picked you up.")).c_str()),
            zCOLOR(160, 230, 160, 255));
    }

    static void CoopLiegenEchtSterben() {
        CoopLiegtHier = false;
        CoopLiegtAuf = false;
        if (!player || player->IsDead())
            return;
        CoopLiegenEcht = true;
        // u291 (P378, Nutzer 18.11.: "kommt eine gebuggte Animation"): wie Gothics Todesstoss (oCNpc::EV_AttackFinish) erst die
        // Uebergangsanimation vom Liegen in den Tod, dann DoDie
        zCModel* model = player->GetModel();
        const char* ani = NULL;
        if (model) {
            if (model->IsAnimationActive(zSTRING("S_WOUNDED")))
                ani = "T_WOUNDED_2_DEAD";
            else if (model->IsAnimationActive(zSTRING("S_WOUNDEDB")))
                ani = "T_WOUNDEDB_2_DEADB";
        }
        if (ani)
            model->StartAni(zSTRING(ani), 0);
        player->SetAttribute(NPC_ATR_HITPOINTS, 0);
        player->DoDie(NULL);
        if (ani && model && !model->IsAnimationActive(zSTRING(ani)))
            model->StartAni(zSTRING(ani), 0);
        LiegenLog("echter Tod mit Animation %s (P378)", ani ? ani : "keine (Held liegt nicht in S_WOUNDED/S_WOUNDEDB)");
        CoopLiegenEcht = false;
    }

    static void CoopLiegenAlleBesiegt() {
        CoopAlleTotMs = CurrentMs;
        CoopAlleTotEsc = false;
        if (ServerThread)
            CoopLiegenSenden("tot");
        CoopLiegenEchtSterben();
        const char* text = ServerThread
            ? CoopTr("Ihr seid alle besiegt. In 5 s wird der letzte gemeinsame Stand geladen - Esc: anderen Stand w\xE4hlen.",
                     "You have all been defeated. The last shared save loads in 5 s - Esc: choose another save.")
            : CoopTr("Ihr seid alle besiegt. Der Host l\xE4" "dt gleich den letzten gemeinsamen Stand - Esc: anderen Stand w\xE4hlen.",
                     "You have all been defeated. The host loads the last shared save shortly - Esc: choose another save.");
        zCOLOR farbe(255, 120, 100, 255);
        if (screen)
            screen->PrintTimedCX(2400, zSTRING(text), 5000.0f, &farbe);
        ChatLog(string(text), farbe);
        LiegenLog("alle Coop-Spieler besiegt - alle sterben%s", ServerThread ? ", Laden in 5 s (Esc: kein automatisches Laden)" : "");
    }

    // Host: ein NPC im Kampf mit einem Coop-Spieler im Umkreis von 20 m?
    static bool CoopKampfUm(oCNpc* opfer) {
        oCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        if (!w || !opfer)
            return false;
        static const char* const kampf[] = { "ZS_ATTACK", "ZS_MM_ATTACK", "ZS_MM_ATTACKMAGE", "ZS_ORC_ATTACK", "ZS_BERZERK", "ZS_MM_HUNT",
            "ZS_ASSESSENEMY", "ZS_MM_ASSESSENEMY", "ZS_MM_THREATENENEMY", "ZS_GOTOFIGHT" };
        const zVEC3 p = opfer->GetPositionWorld();
        for (auto* l = CoopWorldNpcListFirst(w); l; l = l->next) {
            oCNpc* n = l->data;
            if (!n || n == player || PlayerNpcs.count(n) || n->IsDead() || n->IsUnconscious())
                continue;
            if ((n->GetPositionWorld() - p).Length() > 2000.0f)
                continue;
            oCNpc* e = n->enemy;
            if (!e || !(e == player || PlayerNpcs.count(e)))
                continue;
            const char* st = n->state.curState.name.ToChar();
            for (const char* k : kampf)
                if (_stricmp(st, k) == 0)
                    return true;
        }
        return false;
    }

    // u292 (P388, Othello-Test u291 19.11., Host 167-192 s): Othellos ZS_Attack laesst den NPC (Silas) mit leerem Gegner im Zustand
    // ZS_ATTACK stehen, sein letztes Ziel (Skriptwert AIV_LASTTARGET) bleibt der Held - P376 sah ohne Gegner keinen Kampf, der Held
    // stand auf und wurde sofort wieder geschlagen. Jetzt zaehlt ohne Gegner das letzte Ziel (Instanz-Symbol, nur verglichen, nie
    // dereferenziert). Kennt die Mod den Wert nicht, bleibt es beim alten Verhalten (keine Ersatzzahl). Allgemein (G1/G2).
    static oCNpc* CoopLetztesZiel(oCNpc* n) {
        static int aiv = -2;
        if (aiv == -2) {
            aiv = -1;
            zCPar_Symbol* s = parser ? parser->GetSymbol("AIV_LASTTARGET") : NULL;
            int v = -1;
            if (s)
                s->GetValue(v, 0);
            if (v >= 0 && v < COOP_AIV_MAX)
                aiv = v;
            LiegenLog("letztes Ziel ohne Gegner: AIV_LASTTARGET %d (P388)", aiv);
        }
        if (aiv < 0 || !parser || !n)
            return NULL;
        const int id = n->aiscriptvars[aiv];
        zCPar_Symbol* z = id > 0 ? parser->GetSymbol(id) : NULL;
        return z ? (oCNpc*)z->GetInstanceAdr() : NULL;
    }

    // u291 (P376, Nutzer 18.11.: "Wenn Mitspieler im Kampf ist, dann sollte man selbst auch nicht aufstehen koennen, vor allem wenn es
    // in der Naehe ist"): Host - kaempft irgendwo bei irgendeinem Coop-Spieler (Umkreis 20 m) ein NPC? Zaehlt auch: NPC, dessen Kampf
    // ein Client fuehrt (beim Host leerer Zustand), und Monster beim Fressen (ZS_MM_EATBODY) neben einem Spieler
    static bool CoopKampfBeiCoop() {
        oCWorld* w = ogame ? ogame->GetGameWorld() : NULL;
        if (!w || !player)
            return false;
        std::vector<zVEC3> pos;
        std::vector<std::string> wer;
        if (!player->IsDead()) {
            pos.push_back(player->GetPositionWorld());
            wer.push_back("eigener Held");
        }
        for (auto& p : PlayerNpcs) {
            oCNpc* n = p.first;
            if (!n || n == player || n->IsDead())
                continue;
            pos.push_back(n->GetPositionWorld());
            wer.push_back(n->GetObjectName().ToChar());
        }
        if (pos.empty())
            return false;
        static const char* const kampf[] = { "ZS_ATTACK", "ZS_MM_ATTACK", "ZS_MM_ATTACKMAGE", "ZS_ORC_ATTACK", "ZS_BERZERK", "ZS_MM_HUNT",
            "ZS_ASSESSENEMY", "ZS_MM_ASSESSENEMY", "ZS_MM_THREATENENEMY", "ZS_GOTOFIGHT" };
        for (auto* l = CoopWorldNpcListFirst(w); l; l = l->next) {
            oCNpc* n = l->data;
            if (!n || n == player || PlayerNpcs.count(n) || n->IsDead() || n->IsUnconscious())
                continue;
            const zVEC3 np = n->GetPositionWorld();
            int bei = -1;
            for (size_t i = 0; i < pos.size(); i++)
                if ((np - pos[i]).Length() <= 2000.0f) {
                    bei = (int)i;
                    break;
                }
            if (bei < 0)
                continue;
            bool kaempft = CoopLeadIsRemote(n) != false;
            const char* grund = "Kampf fuehrt ein Client";
            if (!kaempft) {
                oCNpc* e = n->enemy;
                const bool ohneGegner = e == NULL;
                if (ohneGegner)
                    e = CoopLetztesZiel(n);   // u292 (P388)
                if (!e || !(e == player || PlayerNpcs.count(e)))
                    continue;
                const char* st = n->state.curState.name.ToChar();
                if (!ohneGegner && _stricmp(st, "ZS_MM_EATBODY") == 0) {
                    kaempft = true;
                    grund = "frisst neben einem Spieler";
                }
                else
                    for (const char* k : kampf)
                        if (_stricmp(st, k) == 0) {
                            kaempft = true;
                            grund = ohneGegner ? "Kampfzustand ohne Gegner, letztes Ziel ein Coop-Spieler (P388)" : "Kampfzustand";
                            break;
                        }
            }
            if (kaempft) {
                if (!CoopKampfGeloggt) {
                    CoopKampfGeloggt = true;
                    LiegenLog("Aufstehen gesperrt (P376): %s bei %s (%s, Zustand %s)", n->GetObjectName().ToChar(), wer[bei].c_str(), grund,
                        n->state.curState.name.ToChar());
                }
                return true;
            }
        }
        return false;
    }

    // Host: alle unten -> alle besiegt; Kampf vorbei -> aufstehen lassen
    static void CoopLiegenHostPulse() {
        static long long naechste = 0;
        if (CurrentMs < naechste || CoopAlleTotMs)
            return;
        naechste = CurrentMs + 500;
        int spieler = 1;
        bool alleUnten = CoopLiegtHier || player->IsDead();
        for (auto& p : PlayerNpcs) {
            oCNpc* n = p.first;
            if (!n || n == player)
                continue;
            spieler++;
            if (!CoopLiegtFigur(n))
                alleUnten = false;
        }
        if (spieler >= 2 && alleUnten) {
            CoopLiegenAlleBesiegt();
            return;
        }
        const std::string ich = MyselfId.ToChar();
        if (CoopLiegtHier && !CoopLiegtAuf) {
            if (CoopKampfBeiCoop())   // u291 (P376): kein Kampf bei irgendeinem Coop-Spieler
                CoopKampfFreiSeit.erase(ich);
            else {
                long long& s = CoopKampfFreiSeit[ich];
                if (!s)
                    s = CurrentMs;
                else if (CurrentMs > s + 3000 && CurrentMs > CoopLiegtSeit + 5000) {
                    CoopKampfFreiSeit.erase(ich);
                    CoopLiegenAufstehen("Kampf vorbei", false);
                    ChatLog(string(CoopTr("Der Kampf ist vorbei - du stehst wieder auf.", "The fight is over - you get back up.")), zCOLOR(160, 230, 160, 255));
                }
            }
        }
        for (auto& p : PlayerNpcs) {
            oCNpc* n = p.first;
            if (!n || n == player)
                continue;
            const std::string id = n->GetObjectName().ToChar();
            auto it = CoopLiegtPartner.find(id);
            if (it == CoopLiegtPartner.end() || n->IsDead()) {
                CoopKampfFreiSeit.erase(id);
                continue;
            }
            auto ag = CoopAufGesendet.find(id);
            if (ag != CoopAufGesendet.end() && CurrentMs < ag->second + 10000)
                continue;   // "auf" ist unterwegs
            if (CoopKampfBeiCoop()) {   // u291 (P376)
                CoopKampfFreiSeit.erase(id);
                continue;
            }
            long long& s = CoopKampfFreiSeit[id];
            if (!s)
                s = CurrentMs;
            else if (CurrentMs > s + 3000 && CurrentMs > it->second + 5000) {
                CoopKampfFreiSeit.erase(id);
                CoopAufGesendet[id] = CurrentMs;
                CoopLiegenSenden("auf", id);
                LiegenLog("%s: Kampf vorbei - steht auf", id.c_str());
            }
        }
    }

    // alle besiegt: Menue offen (Esc) - nicht automatisch laden; der Host laedt nach 5 s
    static void CoopLiegenTotPulse() {
        if (!CoopAlleTotMs || IsLoadingLevel)
            return;
        if (zCMenu::GetActive() && !CoopAlleTotEsc) {
            CoopAlleTotEsc = true;
            if (!ServerThread)
                CoopLiegenSenden("esc");
            LiegenLog("Men\xFC offen - kein automatisches Laden, der Spieler w\xE4hlt selbst");
        }
        if (!ServerThread || CoopAlleTotEsc || CurrentMs < CoopAlleTotMs + 5000)
            return;
        CoopAlleTotMs = 0;
        std::string was;
        if (CoopSaveSyncLoadNewestJoint(was)) {
            LiegenLog("lade den letzten gemeinsamen Stand: %s", was.c_str());
            ChatLog(string((std::string(CoopTr("Lade den letzten gemeinsamen Stand: ", "Loading the last shared save: ")) + was).c_str()), zCOLOR(255, 220, 120, 255));
        }
        else {
            LiegenLog("kein Stand zum Laden: %s", was.c_str());
            ChatLog(string((std::string(CoopTr("Kein gemeinsamer Stand zum Laden (", "No shared save to load (")) + was +
                CoopTr(") - bitte selbst laden.", ") - please load one yourself.")).c_str()), zCOLOR(255, 120, 100, 255));
        }
    }

    // je Bild (aus ReviveFriendLoop)
    static void CoopLiegenPulse() {
        if (!player || IsLoadingLevel)
            return;
        if (CoopLiegtHier) {
            if (player->IsDead()) {
                CoopLiegtHier = false;
                CoopLiegtAuf = false;
            }
            else if (!player->IsUnconscious() && CurrentMs > CoopLiegtSeit + 2000) {
                CoopLiegtHier = false;
                CoopLiegtAuf = false;
                LiegenLog("eigener Held steht wieder (Leben %d)", player->GetAttribute(NPC_ATR_HITPOINTS));
                player->StopFaceAni("T_HURT");   // u291 (P379, Nutzer 18.11.: "Der Mund bleibt offen nach dem Tod")
                CoopKoWaffenPruefen();           // u291 (P380)
            }
            else if (CoopLiegtAuf)
                player->state.SetStateTime(9999);
            else if (!CoopLiegenMitspielerDa())
                CoopLiegenAufstehen("kein Mitspieler mehr da", false);
            else if (ClientThread && !ServerThread && CurrentMs > CoopLiegtSeit + 300000)   // u291 (P376): 5 Minuten statt 2
                CoopLiegenAufstehen("5 Minuten ohne Entscheidung des Hosts", false);
            else
                player->state.SetStateTime(0);
        }
        // u296 (P405): auch nach einem gewoehnlichen K.o. (Gothics DropUnconscious, nicht P363) die gemerkten Waffen pruefen
        if (!CoopLiegtHier && (CoopKoMelee || CoopKoFern) && !player->IsUnconscious() && !player->IsDead() && CurrentMs > CoopKoMerkMs + 2000)
            CoopKoWaffenPruefen();
        const bool unten = CoopLiegtHier || player->IsDead();
        if (unten != CoopUntenGemeldet && (ServerThread || ClientThread)) {
            CoopUntenGemeldet = unten;
            CoopLiegenSenden(unten ? "liegt" : "steht");
        }
        if (ServerThread)
            CoopLiegenHostPulse();
        CoopLiegenTotPulse();
    }

    // SYNC_LIEGEN (PacketProcessor.cpp)
    void CoopLiegenEmpfangen(json& e, const std::string& id) {
        const std::string op = e.contains("op") && e["op"].is_string() ? e["op"].get<std::string>() : std::string();
        const std::string to = e.contains("to") && e["to"].is_string() ? e["to"].get<std::string>() : std::string();
        if (!to.empty() && to != MyselfId.ToChar())
            return;
        if (op == "liegt") {
            CoopLiegtPartner[id] = CurrentMs;
            LiegenLog("%s liegt am Boden bzw. ist tot", id.c_str());
            CoopKampfGeloggt = false;
        }
        else if (op == "steht") {
            CoopLiegtPartner.erase(id);
            CoopKampfFreiSeit.erase(id);
            CoopAufGesendet.erase(id);
            LiegenLog("%s steht wieder", id.c_str());
            for (auto& p : PlayerNpcs)   // u291 (P379): K.o.-Gesicht der Figur loesen
                if (p.first && p.first != player && id == p.first->GetObjectName().ToChar())
                    p.first->StopFaceAni("T_HURT");
        }
        else if (op == "auf" && !ServerThread && CoopLiegtHier) {
            CoopLiegenAufstehen("Kampf vorbei (Host)", false);
            ChatLog(string(CoopTr("Der Kampf ist vorbei - du stehst wieder auf.", "The fight is over - you get back up.")), zCOLOR(160, 230, 160, 255));
        }
        else if (op == "tot" && !ServerThread)
            CoopLiegenAlleBesiegt();
        else if (op == "esc" && ServerThread && CoopAlleTotMs && !CoopAlleTotEsc) {
            CoopAlleTotEsc = true;
            LiegenLog("%s w\xE4hlt selbst einen Stand - kein automatisches Laden", id.c_str());
            ChatLog(string((id + CoopTr(" w\xE4hlt einen Spielstand - kein automatisches Laden.", " is choosing a save - no automatic loading.")).c_str()),
                zCOLOR(255, 220, 120, 255));
        }
    }

    // Plugin.cpp (Game_LoadEnd_SaveGame): nach dem Laden liegt niemand mehr
    void CoopLiegenNachLaden() {
        CoopLiegtHier = CoopLiegtAuf = CoopLiegenEcht = CoopUntenGemeldet = false;
        CoopLiegtPartner.clear();
        CoopKampfFreiSeit.clear();
        CoopAufGesendet.clear();
        CoopAlleTotMs = 0;
        CoopAlleTotEsc = false;
        CoopKampfGeloggt = false;
        CoopKoMelee = CoopKoFern = NULL;
        CoopKoMeleeInst = CoopKoFernInst = -1;
        // u294 (P395, Othello-Test u292: Figur lag nach dem Laden noch, stand erst 3 s spaeter auf; nach dem Tod blieb sie beim Mitspieler
        // dauerhaft in ZS_DEAD): die Figuren (PlayerNpcs) ueberdauern das Laden mit ihrem KI-Zustand. Jede liegende oder tote Figur wird
        // sofort aufgestellt - die echten Werte kommen mit den naechsten Paketen des Mitspielers
        for (auto& kv : PlayerNpcs) {
            oCNpc* f = kv.first;
            if (!f)
                continue;
            const char* zs = f->state.curState.name.ToChar();
            const bool tot = f->IsDead() != 0, ko = f->IsUnconscious() != 0;
            const bool zsLiegt = zs && (_stricmp(zs, "ZS_DEAD") == 0 || _stricmp(zs, "ZS_UNCONSCIOUS") == 0);
            if (!tot && !ko && !zsLiegt)
                continue;
            if (f->GetAttribute(NPC_ATR_HITPOINTS) <= 0) {
                const int hpMax = f->GetAttribute(NPC_ATR_HITPOINTSMAX);
                f->SetAttribute(NPC_ATR_HITPOINTS, hpMax > 1 ? hpMax : 1);
            }
            f->StopFaceAni("T_HURT");
            f->GetEM()->KillMessages();
            f->ClearEM();
            f->state.ClearAIState();
            if (f->GetHomeWorld())
                CoopResetPosSafe(f, f->GetPositionWorld());
            else if (f->GetAnictrl())
                f->GetAnictrl()->SearchStandAni(TRUE);
            LiegenLog("Figur %s nach dem Laden aufgestellt (war %s, HP jetzt %d) (P395)", kv.second.ToChar(), tot ? "tot" : ko ? "bewusstlos" : zs,
                f->GetAttribute(NPC_ATR_HITPOINTS));
        }
    }

    void ReviveFriendLoop() {
        PluginState = "ReviveFriendLoop";

        if (WorldEditMode) {
            return;
        }

        CoopLiegenPulse();   // u282 (P363, P364)

        if (!ServerThread && !ClientThread) {
            return;
        }

        if (ReviveStartTime + 4000 < CurrentMs && ReviveState.Compare("HEALING")) {   // u282 (P365): 4 s statt 10 s
            if (player->IsDead() || player->IsUnconscious()) {
                ReviveStartTime = 0;
                ReviveState = "READY";
                return;
            }

            if (ReviveNpc && Myself)   // v98 (A16): vorher ohne Pruefung vor dem if unten
                Myself->SyncRevived(ReviveNpc->GetObjectName());

            if (ReviveNpc) {
                ReviveNpc->StopFaceAni("T_HURT");
                ReviveNpc->SetWeaponMode2(NPC_WEAPON_NONE);
            }

            ReviveStartTime = 0;
            ReviveState = "READY";
            ReviveNpc = NULL;
            ChatLog(CoopTr("Du hast deinen Mitspieler aufgehoben!", "You picked your friend up!"));
            return;
        }

        if (ReviveState.Compare("HEALING") && ReviveNpc && ReviveNpc->GetPositionWorld().Distance(player->GetPositionWorld()) > 200) {
            ReviveStartTime = 0;
            ReviveState = "READY";

            ChatLog(CoopTr("Beim Wiederbeleben kannst du dich nicht bewegen.", "You cannot move while reviving your friend."));
            return;
        }

        if (CoopKeyHit(CK_REVIVE)) {   // v39: nur mit Shift; u226 (P264): Taste aus Einstellungen > COOP > Coop-Tasten (Vorgabe Shift+F4)
            auto focusedNpc = player->GetFocusNpc();
            if (!focusedNpc || !IsCoopPlayer(focusedNpc->GetObjectName()))
            {
                // v36: ohne Fokus die naechste liegende Coop-Figur in 2 m. Othello (CollectFocusVob -> OTHELLO_ABI+0x8A730) laesst
                // Bewusstlose gar nicht und Tote nur mit Inventar anvisieren - dafuer stand bisher ein Schwert in
                // PC_MULTIPLAYER_HERO.D.
                focusedNpc = NULL;
                float best = 200.0f;
                for (auto& p : PlayerNpcs) {
                    oCNpc* n = p.first;
                    if (!n || n == player || (n->GetAttribute(NPC_ATR_HITPOINTS) != 0 && !CoopLiegtFigur(n)))   // u282 (P363): auch "liegt"
                        continue;
                    const float dist = n->GetPositionWorld().Distance(player->GetPositionWorld());
                    if (dist <= best) {
                        best = dist;
                        focusedNpc = n;
                    }
                }
                if (!focusedNpc)
                    return;
            }

            ReviveNpc = focusedNpc;

            if (ReviveState.Compare("READY")) {
                auto frinedHp = ReviveNpc->GetAttribute(NPC_ATR_HITPOINTS);
                if (frinedHp != 0 && !CoopLiegtFigur(ReviveNpc)) {   // u282 (P363): liegt am Boden (Leben 1) - aufheben geht
                    ChatLog(CoopTr("Dein Mitspieler lebt - nichts wiederzubeleben.", "You cannot revive your friend when alive."));
                    ReviveNpc = NULL;
                    return;
                }

                if (ReviveNpc->GetPositionWorld().Distance(player->GetPositionWorld()) > 200) {
                    ChatLog(CoopTr("Zu weit weg zum Wiederbeleben.", "You are too far away to revive your friend."));
                    ReviveNpc = NULL;
                    return;
                }

                ChatLog(CoopTr("Du hebst deinen Mitspieler auf - 4 Sekunden nicht bewegen.", "You are picking your friend up, please do not move for 4 seconds."));
                ReviveState = "HEALING";
                ReviveStartTime = CurrentMs;
                player->GetModel()->StartAni("T_PLUNDER", 1);
            }
            else if (ReviveState.Compare("HEALING")) {
                ChatLog(CoopTr("Wiederbelebung l" "\xE4" "uft schon - bitte warten.", "You are already reviving your friend, please wait."));
            }
        }
    }
}