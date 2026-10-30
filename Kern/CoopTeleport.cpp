// Supported with union (c) 2020 Union team
// Union SOURCE file

namespace GOTHIC_ENGINE {

    // --- v102: Teleport-Rune zum Mitspieler (22.10.2026) ------------------------------------------------------------------
    //
    // Nutzer: "Ich moechte jetzt ein neues Item einfuehren. Eine Teleportrune. Diese teleportiert einen zum Mitspieler. Jeder
    // Spieler startet mit einer Teleport-Rune. Es wird am Anfang jedes Spiel-Ladens geprueft, ob der Spieler sie im Inventar hat,
    // und wenn nicht, wird sie eingefuegt. Der Wert sollte 0 sein. ... Die Teleport-Animation sollte durchgefuehrt werden. Wir
    // machen erstmal Manakosten 0."
    //
    // So funktionieren Teleport-Runen (G1-Modkit, Othello-DAT): Rune = Item mit spell = SPL_TELEPORTn. Beim Wirken laedt die
    // Engine auf (oCSpell::Invest 0x0047CD40: erst "Mana > 0?", dann je time_per_mana 750 ms CallScriptInvestedMana 0x0047D930
    // -> Spell_ProcessMana -> Spell_Logic_TeleportN, ab dem zweiten Schritt ChangeAttribute(Mana, -1)). Ab 5 investiertem Mana
    // (SPL_SENDCAST_TELEPORT) ruft die Logik AI_Teleport(self, "Wegpunkt") (External 0x00650460: EV_BEAMTO -> oCNpc::BeamTo
    // 0x00693B10, nur ueber Namen) und AI_PlayAni(self, "T_HEASHOOT_2_STAND"). Othello sperrt alle fuenf, solange man in Xardas'
    // altem Turm ist; Othellos eigene Runen 6-12 haben keinen Zauber, sie schalten Marken auf der Karte frei (MAP_TELEPORT).
    //
    // Hier (allgemein G1, Rune in system\Autorun\COOP_ITEMS.D): ITRU_COOP_TELEPORT mit spell = SPL_TELEPORT5.
    //  - Mana 0: Haken auf oCSpell::Invest - zaubert der eigene Held mit dieser Rune (oCMag_Book::GetSpellItem(oCSpell*)
    //    0x00472E40), hat er waehrenddessen mindestens 1 Mana (sonst bricht die Engine ab), danach wieder genau den alten Wert.
    //  - Ziel: das External AI_TELEPORT wird umgeleitet (Symbol-Funktionszeiger wie CoopDialogSync.cpp). Waehrend dieses Wirkens
    //    nimmt GT die Parameter vom Stapel und setzt den Helden im naechsten Spielbild neben den Mitspieler (CoopPlaceBesideHost,
    //    CoopSaveSync.cpp: freie Stelle mit Boden, ohne Wand/Wasser; sonst oCVob::SearchNpcPosition an seiner Stelle), Bewegung
    //    zuruecksetzen wie nach dem Laden (PendingMoveReset). Alle anderen AI_Teleport-Aufrufe gehen unveraendert ans Original.
    //  - Mitspieler: anvisierte Coop-Figur, sonst beim Mitspieler die Figur des Hosts, sonst die erste (nach Name).
    //  - Startausstattung: 3 s und 15 s nach jedem Laden (auch neues Spiel, Weltwechsel; nach dem Wiederherstellen des eigenen
    //    Helden, CoopSaveSync.cpp) - ohne Rune im Inventar (oCNpc::IsInInv(int, int) 0x006A4E80) eine anlegen (PutInInv 0x006A4FF0,
    //    unter WorldSyncApplying: kein Welt-Abgleich).
    //  - Beim Partner spielt GT den Zauber nur mit einem Aufladeschritt nach (RemoteNpc::UpdateSpellCasts) - die Logik teleportiert
    //    erst ab 5 Mana, die Figur springt dort also nicht.
    // Engine-Stellen (Speicherabbild): alle unveraendert.
    //
    // u17 (Gothic 2): G2 hat einen Teleport-Zauber je Ziel; die Coop-Rune (scripts\g2\COOP_ITEMS.D) traegt SPL_TELEPORTXARDAS.
    // Geprueft an GOTHIC.DAT/Gothic2.exe: Spell_Logic_TeleportXardas meldet SPL_SENDCAST erst ab Mana >= SPL_COST_TELEPORT (10),
    // teleportiert wird erst beim Wirken - oCMag_Book::Spell_Cast (0x004767A0, ruft "SPELL_CAST_" + FX-Name bei 0x0047692C) ->
    // Spell_Cast_Teleport -> Spell_Cast_TeleportXardas: B_PrintTeleportTooFarAway(NEWWORLD_ZEN) (nur Meldung, wenn CURRENTLEVEL
    // anders), Mana -= 10, AI_Teleport(self, "XARDAS"). Darum in G2: beim Aufladen Mana mindestens SPL_COST_TELEPORT, beim Wirken
    // (CoopTeleportCastBegin/End aus GTs Haken auf Spell_Cast in Hooks.cpp) Mana und CURRENTLEVEL kurz so, dass keine Kosten und
    // keine Meldung entstehen, AI_Teleport -> Mitspieler wie in G1. Eigenes Modell bis u20 nur G1 (G1-MRM laedt G2 nicht), ab u21
    // auch G2 (eigenes G2-MRM, siehe CoopRuneLookInit).

#if COOP_BUILD(Teleport)   // u3: bis u2 nur G1 - Freigabe je Engine in CoopCore.cpp

    static int TpRuneIdx = -2;               // Symbol ITRU_COOP_TELEPORT (-1: COOP_ITEMS.D fehlt)
    static bool TpCasting = false;           // gerade wirkt der eigene Held die Coop-Rune (innerhalb von oCSpell::Invest)
    static bool TpPending = false;           // im naechsten Spielbild zum Mitspieler
    static bool TpExtInstalled = false;
    typedef int(__cdecl* CoopTpExternal)();
    static CoopTpExternal TpOrigAiTeleport = NULL;
    static long long TpEnsureAt1 = 0, TpEnsureAt2 = 0;
    static long long TpEnsureAtSave = 0;     // u7: kurz nach jedem Speichern (CoopTeleportSaveEnd)

    static void TpLog(const char* fmt, ...) {
        static CoopDiagBudget budget = { 0, 0 };
        if (!CoopDiagAllow(budget, 30))
            return;
        char buf[600];
        va_list ap;
        va_start(ap, fmt);
        _vsnprintf(buf, sizeof(buf), fmt, ap);
        va_end(ap);
        buf[sizeof(buf) - 1] = 0;
        OthelloCompatLog("[%lld s, %s] Teleport-Rune: %s", DiagSeconds(), DiagRole(), buf);
    }

    static int TpRune() {
        if (TpRuneIdx == -2 && parser) {
            TpRuneIdx = parser->GetIndex(zSTRING("ITRU_COOP_TELEPORT"));
            if (TpRuneIdx < 0)
                OthelloCompatLog("Teleport-Rune: ITRU_COOP_TELEPORT fehlt im Skript (system\\Autorun\\COOP_ITEMS.D nicht da?) - Rune aus");
        }
        return TpRuneIdx;
    }

    // Coop-Rune im Zauberbuch des eigenen Helden zu diesem Zauber (sonst NULL)
    static oCItem* TpCoopRuneItem(oCSpell* spell) {
        if (!spell || !player || TpRune() < 0)
            return NULL;
        oCMag_Book* book = player->GetSpellBook();
        oCItem* it = book ? book->GetSpellItem(spell) : NULL;
        return it && it->GetInstance() == TpRuneIdx ? it : NULL;
    }

    static bool TpIsCoopRune(oCSpell* spell) {
        return spell && spell->spellCasterNpc == player && TpCoopRuneItem(spell);
    }

    // u17: Mana, das waehrend des Aufladens mindestens da sein muss
    static int TpManaNeeded() {
#if ENGINE >= Engine_G2
        int cost = 10;
        if (zCPar_Symbol* s = parser ? parser->GetSymbol(zSTRING("SPL_COST_TELEPORT")) : NULL)
            s->GetValue(cost, 0);
        return cost > 0 && cost < 1000 ? cost : 10;
#else
        return 1;
#endif
    }

    // u17 (G2): Wirken der Coop-Rune (Hooks.cpp, Haken auf oCMag_Book::Spell_Cast) - siehe Kopf
    static int TpCastMana = -1, TpCastLevel = -1;
    static bool TpCastActive = false;

    void CoopTeleportCastBegin(oCMag_Book* book) {
#if ENGINE >= Engine_G2
        if (!book || !player || (oCNpc*)book->owner != player || TpCastActive)
            return;
        oCSpell* sp = book->GetSelectedSpell();
        if (!TpIsCoopRune(sp))
            return;
        TpCastActive = true;
        TpCasting = true;
        TpCastMana = player->GetAttribute(NPC_ATR_MANA);
        player->SetAttribute(NPC_ATR_MANA, TpCastMana + TpManaNeeded());   // Spell_Cast_* zieht die Kosten ab
        TpCastLevel = -1;
        zCPar_Symbol* cur = parser ? parser->GetSymbol(zSTRING("CURRENTLEVEL")) : NULL;
        zCPar_Symbol* want = parser ? parser->GetSymbol(zSTRING("NEWWORLD_ZEN")) : NULL;   // B_PrintTeleportTooFarAway(NEWWORLD_ZEN)
        if (cur && want) {
            int w = 1;
            want->GetValue(w, 0);
            cur->GetValue(TpCastLevel, 0);
            cur->SetValue(w, 0);
        }
#endif
    }

    void CoopTeleportCastEnd() {
#if ENGINE >= Engine_G2
        if (!TpCastActive)
            return;
        TpCastActive = false;
        TpCasting = false;
        if (player && TpCastMana >= 0)
            player->SetAttribute(NPC_ATR_MANA, TpCastMana);   // Manakosten 0
        if (TpCastLevel >= 0)
            if (zCPar_Symbol* cur = parser ? parser->GetSymbol(zSTRING("CURRENTLEVEL")) : NULL)
                cur->SetValue(TpCastLevel, 0);
        TpCastMana = TpCastLevel = -1;
#endif
    }

    // Haken: Aufladen - fuer die Coop-Rune ohne Manakosten
    int __fastcall oCSpell_Invest_Coop(oCSpell* _this, void* vtable);
    CInvoke<int(__thiscall*)(oCSpell*)> Ivk_oCSpell_Invest_Coop(CoopAddr::oCSpell_Invest, &oCSpell_Invest_Coop, COOP_IVK(Teleport));
    int __fastcall oCSpell_Invest_Coop(oCSpell* _this, void* vtable) {
        if (!TpIsCoopRune(_this))
            return Ivk_oCSpell_Invest_Coop(_this);
        const int atr = (_this->spellEnergy >= 0 && _this->spellEnergy < NPC_ATR_MAX) ? _this->spellEnergy : NPC_ATR_MANA;
        const int saved = player->GetAttribute(atr);
        const int need = TpManaNeeded();   // u17: G1 1, G2 SPL_COST_TELEPORT (Spell_Logic_Teleport* meldet sonst SENDSTOP)
        if (saved < need)
            player->SetAttribute(atr, need);   // die Engine laedt nur mit Mana > 0 auf
        TpCasting = true;
        const int r = Ivk_oCSpell_Invest_Coop(_this);
        TpCasting = false;
        if (player->GetAttribute(atr) != saved)
            player->SetAttribute(atr, saved);   // Manakosten 0
        return r;
    }

    // v103: Nutzer (Test v102): "Beim Auswaehlen steht der Text 'Teleport ins Sumpflager'". Den Namen beim Wechsel im Zauberkreis
    // zeigt oCMag_Book::DoPerFrame (0x004726F6: spells[gewaehlt]->GetName(), dann kurz auf den Bildschirm); oCSpell::GetName
    // (0x0047E1B0, unveraendert) liest Gothics Namenstabelle nach der Zauber-ID - fuer SPL_TELEPORT5 Othellos Teleport ins
    // Sumpflager. Fuer die Coop-Rune jetzt ihr eigener Name (Item-Name aus COOP_ITEMS.D). Rueckgabe zSTRING per Wert: Aufrufer legt
    // den Platz an und schiebt ihn als (versteckten) ersten Parameter, Funktion endet mit ret 4 - hier ausdruecklich so deklariert.
    zSTRING* __fastcall oCSpell_GetName_Coop(oCSpell* _this, void* vtable, zSTRING* ret);
    CInvoke<zSTRING*(__thiscall*)(oCSpell*, zSTRING*)> Ivk_oCSpell_GetName_Coop(CoopAddr::oCSpell_GetName, &oCSpell_GetName_Coop, COOP_IVK(Teleport));
    zSTRING* __fastcall oCSpell_GetName_Coop(oCSpell* _this, void* vtable, zSTRING* ret) {
        zSTRING* r = Ivk_oCSpell_GetName_Coop(_this, ret);
        oCItem* it = r ? TpCoopRuneItem(_this) : NULL;
        if (it && it->name.Length() > 0)
            *r = it->name;
        return r;
    }

    // External AI_Teleport(self, waypoint) - beim Wirken der Coop-Rune: Ziel Mitspieler
    static int __cdecl CoopExtAiTeleport() {
        zCParser* p = zCParser::GetParser();
        if (!TpCasting || !p || p != parser)
            return TpOrigAiTeleport ? TpOrigAiTeleport() : 0;
        zSTRING wp;
        p->GetParameter(wp);   // letzter Parameter zuerst
        oCNpc* self = (oCNpc*)p->GetInstance();
        if (self == player) {
            TpPending = true;
            TpLog("Rune gewirkt - Ziel Mitspieler statt %s", wp.ToChar());
        }
        else
            TpLog("AI_Teleport(%s) beim Wirken der Rune fuer einen anderen NPC - nicht ausgefuehrt", wp.ToChar());
        return 0;
    }

    static void TpInstallExternal() {
        if (TpExtInstalled || !parser)
            return;
        TpExtInstalled = true;
        zCPar_Symbol* s = parser->GetSymbol(zSTRING("AI_TELEPORT"));
        if (!s || s->type != zPAR_TYPE_FUNC || !(s->flags & zPAR_FLAG_EXTERNAL) || !s->single_intdata) {
            OthelloCompatLog("Teleport-Rune: External AI_TELEPORT nicht gefunden - Rune teleportiert wie Teleport5");
            return;
        }
        if ((CoopTpExternal)s->single_intdata == &CoopExtAiTeleport)
            return;
        TpOrigAiTeleport = (CoopTpExternal)s->single_intdata;
        s->single_intdata = (int)&CoopExtAiTeleport;
        OthelloCompatLog("Teleport-Rune: External AI_TELEPORT umgeleitet (Original %p)", TpOrigAiTeleport);
    }

    static bool TpInWorld(oCNpc* n) {
        return n && n != player && n->GetHomeWorld() && n->vobLeafList.GetNum() > 0 && n->GetModel();
    }

    // Ziel: anvisierte Coop-Figur, sonst (beim Mitspieler) die Figur des Hosts, sonst die erste nach Name
    static oCNpc* TpPartner() {
        if (!ServerThread && !ClientThread)
            return NULL;
        oCNpc* focus = player ? player->GetFocusNpc() : NULL;
        if (focus && PlayerNpcs.count(focus) && TpInWorld(focus))
            return focus;
        if (ClientThread && !ServerThread) {
            auto h = PlayerNameToNpc.find(string("HOST"));
            if (h != PlayerNameToNpc.end() && TpInWorld(h->second))
                return h->second;
        }
        for (auto& kv : PlayerNameToNpc)
            if (TpInWorld(kv.second))
                return kv.second;
        return NULL;
    }

    // --- u39 (N2): Teleport ueber Welten ----------------------------------------------------------------------------------
    // Nutzer: "Es soll noch eine Funktion geben, bei der man sich zum Partner ueber Welten hinaus teleportieren kann". Ist kein
    // Mitspieler in der eigenen Welt, aber einer in einer anderen (CoopPeerWorlds: seine Welt und letzte Lage aus SYNC_POS/INIT_NPC,
    // CoopWorlds.cpp), wechselt die Rune in dessen Welt - wie beim gemeinsamen Weltwechsel, aber nur ich und ohne Abfrage (CoopWcGo,
    // Startpunkt der Welt) - und setzt den Helden danach neben ihn: neben seine Figur, sobald sie da ist (bis 8 s), sonst neben seine
    // zuletzt gemeldete Lage (CoopPlaceBesideHost; ohne legale Stelle oCVob::SearchNpcPosition an ihr). Der Client nimmt bevorzugt
    // den Host. Auch der Coop-Testlauf ruft es (Schritt "Teleport zu mir ueber Welten"). Allgemein (G1/G2).
    struct TpAcrossJob {
        bool on = false;
        std::string id, nick, world;
        int phase = 0;
        int loadsAt = 0;
        long long since = 0, loadedMs = 0;
    };
    static TpAcrossJob TpAcross;

    static bool TpPickElsewhere(const std::string& want, std::string& id, std::string& nick, std::string& world) {
        const int mine = CoopWorldKey();
        const std::string me = MyselfId.ToChar();
        for (auto& kv : CoopPeerWorlds) {
            if (kv.first == me || !kv.second.key || kv.second.key == mine || kv.second.file.empty())
                continue;
            if (!want.empty() && kv.first != want)
                continue;
            id = kv.first;
            nick = kv.second.nick.empty() ? kv.first : kv.second.nick;
            world = kv.second.file;
            return true;
        }
        return false;
    }

    // want: Spieler-Kennung (leer = Client: der Host, sonst der erste in einer anderen Welt). true = Weltwechsel ausgeloest
    bool CoopTeleportAcross(const std::string& want, std::string& why) {
        if (!player || !ogame || IsLoadingLevel) {
            why = "nicht im Spiel";
            return false;
        }
        if (TpAcross.on) {
            why = "Teleport ueber Welten laeuft schon";
            return false;
        }
        std::string id, nick, world;
        const std::string pref = !want.empty() ? want : (ClientThread && !ServerThread) ? std::string("HOST") : std::string();
        if (!TpPickElsewhere(pref, id, nick, world) && (!want.empty() || !TpPickElsewhere(std::string(), id, nick, world))) {
            why = want.empty() ? "kein Mitspieler in einer anderen Welt (oder seine Welt noch unbekannt)" : want + " ist in keiner anderen bekannten Welt";
            return false;
        }
        TpAcross = TpAcrossJob();
        TpAcross.on = true;
        TpAcross.id = id;
        TpAcross.nick = nick;
        TpAcross.world = world;
        TpAcross.loadsAt = CoopWorldLoads;
        TpAcross.since = CurrentMs;
        ChatLog(string::Combine(CoopTr("Teleport: zu %s nach %s ...", "Teleport: to %s in %s ..."), string(nick.c_str()),
            string(CoopWorldShow(world).c_str())), zCOLOR(160, 230, 160, 255));
        TpLog("ueber Welten zu %s (%s): Weltwechsel %s -> %s", nick.c_str(), id.c_str(), CoopWorldName().c_str(), world.c_str());
        CoopWcGo(world, "");
        return true;
    }

    bool CoopTeleportAcrossBusy() {
        return TpAcross.on;
    }

    // u48 (P90): gleich am Ende des Ladens (vor dem ersten Bild) an die zuletzt gemeldete Lage des Mitspielers - bis u47 stand der
    // Held 1-8 s am Startpunkt der Welt, Gothics Spawn-Manager raeumte in der Zeit weit entfernte Tote weg (Test u47: Leiche der
    // Markierung neben dem Partner fehlte). Neben seine Figur setzt TpAcrossPulse danach noch, sobald sie da ist. Aufruf aus LoadEnd
    // (Plugin.cpp) nach CoopRemoveStrayHeroes und CoopWorldsLoadEnd. Allgemein (G1/G2).
    void CoopTeleportAcrossLoadEnd() {
        if (!TpAcross.on || TpAcross.phase != 0 || !player || CoopWorldLoads == TpAcross.loadsAt || !CoopWcInTarget(TpAcross.world))
            return;
        auto pw = CoopPeerWorlds.find(TpAcross.id);
        if (pw == CoopPeerWorlds.end() || !pw->second.hasPos || pw->second.key != CoopWorldKey())
            return;
        const zVEC3 target(pw->second.pos[0], pw->second.pos[1], pw->second.pos[2]);
        std::string how;
        PendingMoveReset = true;
        bool ok = CoopPlaceBesideHost(target, player->GetAtVectorWorld(), how, "die letzte Lage des Mitspielers");
        if (!ok) {
            zVEC3 p = target;
            if (player->SearchNpcPosition(p)) {
                player->SetCollDet(FALSE);
                player->SetPositionWorld(p);
                player->SetCollDet(TRUE);
                how += " - direkt an seiner Lage";
                ok = true;
            }
        }
        if (!ok)
            PendingMoveReset = false;
        TpLog("ueber Welten zu %s: beim Laden %s", TpAcross.id.c_str(), ok ? how.c_str() : "keine Stelle an seiner letzten Lage - Startpunkt");
    }

    static void TpAcrossPulse() {
        if (!TpAcross.on)
            return;
        if (TpAcross.phase == 0) {
            if (CoopWorldLoads != TpAcross.loadsAt && CoopWcInTarget(TpAcross.world)) {
                TpAcross.phase = 1;
                TpAcross.loadedMs = CurrentMs;
            }
            else if (CurrentMs > TpAcross.since + 120000) {
                ChatLog(CoopTr("Teleport: Weltwechsel nicht fertig geworden.", "Teleport: world change did not finish."), zCOLOR(255, 80, 80, 255));
                TpLog("ueber Welten zu %s: nach 120 s nicht in %s (jetzt %s) - aufgegeben", TpAcross.id.c_str(), TpAcross.world.c_str(),
                    CoopWorldName().c_str());
                TpAcross.on = false;
            }
            return;
        }
        if (CurrentMs < TpAcross.loadedMs + 1000)
            return;   // erst ankommen lassen
        oCNpc* t = NULL;
        auto it = PlayerNameToNpc.find(string(TpAcross.id.c_str()));
        if (it != PlayerNameToNpc.end() && TpInWorld(it->second))
            t = it->second;
        if (!t && CurrentMs < TpAcross.loadedMs + 8000)
            return;   // seine Figur kommt mit seinem naechsten INIT_NPC
        auto pw = CoopPeerWorlds.find(TpAcross.id);
        const bool hasPos = pw != CoopPeerWorlds.end() && pw->second.hasPos && pw->second.key == CoopWorldKey();
        const zVEC3 target = t ? t->GetPositionWorld() : hasPos ? zVEC3(pw->second.pos[0], pw->second.pos[1], pw->second.pos[2]) : zVEC3(0.f, 0.f, 0.f);
        std::string how = "Startpunkt der Welt (Lage des Mitspielers unbekannt)";
        bool ok = false;
        if (t || hasPos) {
            PendingMoveReset = true;
            ok = CoopPlaceBesideHost(target, t ? t->GetAtVectorWorld() : player->GetAtVectorWorld(), how, t ? "den Mitspieler" : "die letzte Lage des Mitspielers");
            if (!ok) {
                zVEC3 p = target;
                if (player->SearchNpcPosition(p)) {
                    player->SetCollDet(FALSE);
                    player->SetPositionWorld(p);
                    player->SetCollDet(TRUE);
                    how += " - direkt an seiner Lage";
                    ok = true;
                }
            }
            if (!ok)
                PendingMoveReset = false;
        }
        ChatLog(ok ? string::Combine(CoopTr("Teleport: bei %s in %s.", "Teleport: with %s in %s."), string(TpAcross.nick.c_str()), string(CoopWorldShow(CoopWorldName()).c_str()))
            : string::Combine(CoopTr("Teleport: in %s, aber nicht neben %s.", "Teleport: in %s, but not next to %s."), string(CoopWorldShow(CoopWorldName()).c_str()), string(TpAcross.nick.c_str())),
            ok ? zCOLOR(160, 230, 160, 255) : zCOLOR(255, 200, 80, 255));
        TpLog("ueber Welten zu %s angekommen: %s", TpAcross.id.c_str(), how.c_str());
        TpAcross.on = false;
    }

    static void TpDo() {
        oCNpc* t = TpPartner();
        if (!t) {
            std::string why;
            if (CoopTeleportAcross(std::string(), why))   // u39 (N2): Mitspieler in einer anderen Welt - dorthin
                return;
            ChatLog(CoopTr("Teleport: kein Mitspieler da.", "Teleport: no partner here."), zCOLOR(255, 200, 80, 255));
            TpLog("kein Mitspieler in der Welt (%s) - Lage unveraendert (%s)", (ServerThread || ClientThread) ? "verbunden" : "ohne Coop", why.c_str());
            return;
        }
        const zVEC3 from = player->GetPositionWorld();
        const zVEC3 at = t->GetPositionWorld();
        std::string how;
        PendingMoveReset = true;   // Bewegung im naechsten Bild zuruecksetzen (wie nach dem Laden, CoopSaveSync.cpp)
        bool ok = CoopPlaceBesideHost(at, t->GetAtVectorWorld(), how, "den Mitspieler");
        if (!ok) {
            zVEC3 p = at;
            if (player->SearchNpcPosition(p)) {
                player->SetCollDet(FALSE);
                player->SetPositionWorld(p);
                player->SetCollDet(TRUE);
                char b[160];
                _snprintf(b, sizeof(b), "direkt beim Mitspieler (%s; %.0f/%.0f/%.0f)", how.c_str(), p.n[0], p.n[1], p.n[2]);
                b[sizeof(b) - 1] = 0;
                how = b;
                ok = true;
            }
        }
        if (!ok) {
            PendingMoveReset = false;
            ChatLog(CoopTr("Teleport: beim Mitspieler ist kein Platz.", "Teleport: no room next to your partner."), zCOLOR(255, 200, 80, 255));
        }
        TpLog("zu %s (%.1f m entfernt): %s", PlayerNpcs.count(t) ? PlayerNpcs[t].ToChar() : "?", (at - from).LengthApprox() / 100.f, how.c_str());
    }

    // --- v106: eigenes Aussehen (Nutzer: "Ersetze die Textur fuer die Rune mit diesem Bild", gt-fix\assets\coop-rune.jpg) ---
    // ItRn_TP04.3ds nutzt Sammeltexturen vieler Gegenstaende (ITMI_GEMS, RUBY_GOLD), alle G1-Runen die gemeinsame ITARRUNES -
    // eine eigene Optik braucht ein eigenes Modell. tools\runetex.py baut aus Gothics ITAR_RUNE_17.MRM (flacher Stein: Quadrat
    // oben/unten, vier Spitzen) und dem Bild das Modell ITRU_COOP_TELEPORT.MRM (Material/Textur COOPRUNES, Texturkoordinaten auf
    // die eigene Textur) und COOPRUNES-C.TEX (DXT3 256 x 128: Quadrat mit "F", Spitzen aus den goldenen Dreiecken des Bilds);
    // die Bytes stehen in CoopRuneAssets.h. GT legt beide beim Start in Gothics Ordner fuer vorkompilierte Modelle/Texturen
    // (zoptions DIR_COMPILED_MESHES / DIR_COMPILED_TEXTURES, nur wenn sie fehlen oder abweichen - wie der Kartenpfeil v105, den
    // Gothic aus system\ laedt), laedt das Modell zur Probe (zCVisual::LoadVisual 0x005DA1F0) und setzt erst dann die Konstante
    // COOP_RUNE_VISUAL aus COOP_ITEMS.D (v106; dort vorbelegt "ItRn_TP04.3ds") auf das eigene Modell - die Instanz liest sie bei
    // jedem Anlegen. Schon vorhandene Runen im eigenen Inventar bekommen das Modell nach dem Laden (TpEnsureRune). Geht etwas
    // schief, bleibt Othellos Aussehen. Zeitpunkt: Spielstart, jedes Menuebild bis erledigt, vor jedem Laden.
    // u21 (Test u20, Nutzer: "Ich habe dir noch die Grafik fuer die Rune gegeben, du musst den Hintergrund transparent machen",
    // assets\coop-rune-g2.jpg, freigestellt assets\coop-rune-g2.png): Gothic 2 bekommt ein eigenes Modell auf dieselbe Weise -
    // tools\runetex_g2.py baut aus ITRU_TELEPORTXARDAS.MRM (G2-Format 0x905, dasselbe Steinmodell wie G1) und dem Bild
    // ITRU_COOP_TELEPORT.MRM/COOPRUNES-C.TEX (Oberseite aus dem Bild, Spitzen wie jede G2-Rune aus ITARRUNE1), Bytes in
    // CoopRuneAssets_G2.h. Geht etwas schief, bleibt das Aussehen von "Teleport zu Xardas" (COOP_ITEMS.D).
#if ENGINE >= Engine_G2
#include "CoopRuneAssets_G2.h"
#else
#include "CoopRuneAssets.h"
#endif

    // u89 (P113): bis u88 hielt GT das Modell als festen Zeiger (TpLookVisual, geladen beim Start) und setzte ihn nach jedem
    // Laden/Speichern wieder an die Rune. Jetzt laedt GT es jedes Mal ueber den Namen (zCProgMeshProto::Load gibt das vorhandene
    // oder ein neues) und haelt keine eigene Referenz - ein Zeiger ueber Ladevorgaenge hinweg ist unnoetig. u89 hielt das fuer die
    // Ursache der Host-Haenger (u87/u88); u90 widerlegte das (die Rune traegt nach dem Laden schlicht ihr Standardmodell, der
    // Haenger kam von einem Gegenstand mit zu niedrigem Zaehler in DoDropVob - P114, Helpers.cpp).
    static int TpLookState = 0;   // 0 offen, 1 eigenes Modell aktiv, 2 aus (Othellos Aussehen)
    static const char* TP_LOOK_VISUAL = "ITRU_COOP_TELEPORT.3DS";

    void CoopRuneLookInit() {
        if (TpLookState || !parser || !zoptions || !zrenderer)
            return;
        // u21: Rueckfall je Spiel - G2 das Aussehen aus COOP_ITEMS.D (ItRu_TeleportXardas.3ds), sonst Othellos/Gothics
        const char* const before = ENGINE >= Engine_G2 ? "Aussehen von Teleport zu Xardas" : "Othellos Aussehen";
        zCPar_Symbol* sym = parser->GetSymbol(zSTRING("COOP_RUNE_VISUAL"));
        if (!sym || sym->type != zPAR_TYPE_STRING || !sym->stringdata) {
            TpLookState = 2;
            OthelloCompatLog("Teleport-Rune: COOP_RUNE_VISUAL fehlt (COOP_ITEMS.D zu alt) - Rune beh\xE4lt das %s", before);
            return;
        }
        std::string mesh, tex;
        if (!CoopAssetWrite(DIR_COMPILED_TEXTURES, "COOPRUNES-C.TEX", CoopRuneTex, sizeof(CoopRuneTex), tex) ||   // u5: CoopCore.cpp
            !CoopAssetWrite(DIR_COMPILED_MESHES, "ITRU_COOP_TELEPORT.MRM", CoopRuneMrm, sizeof(CoopRuneMrm), mesh)) {
            TpLookState = 2;
            OthelloCompatLog("Teleport-Rune: Modell/Textur nicht schreibbar (%s, %s) - %s", mesh.c_str(), tex.c_str(), before);
            return;
        }
        zCVisual* probe = zCVisual::LoadVisual(zSTRING(TP_LOOK_VISUAL));   // u89: nur zur Probe - keine Referenz behalten
        if (!probe) {
            TpLookState = 2;
            OthelloCompatLog("Teleport-Rune: %s nicht ladbar (%s) - %s", TP_LOOK_VISUAL, mesh.c_str(), before);
            return;
        }
        probe->Release();
        sym->SetValue(zSTRING(TP_LOOK_VISUAL), 0);
        TpLookState = 1;
        OthelloCompatLog("Teleport-Rune: eigenes Aussehen aktiv (%s, %s)", mesh.c_str(), tex.c_str());
    }

    static void TpEnsureRune(const char* why) {
        if (!player || player->IsDead() || TpRune() < 0)
            return;
        if (oCItem* have = player->IsInInv(TpRuneIdx, 1)) {
            if (TpLookState != 1)
                return;
            zCVisual* v = zCVisual::LoadVisual(zSTRING(TP_LOOK_VISUAL));   // u89 (P113): vorhandenes Modell oder neu geladen
            if (!v) {
                TpLog("%s: %s nicht ladbar - Rune behaelt ihr Aussehen", why, TP_LOOK_VISUAL);
                return;
            }
            if (have->GetVisual() != v) {
                have->SetVisual(v);
                TpLog("%s: Rune im Inventar bekommt das eigene Aussehen", why);
            }
            v->Release();   // SetVisual haelt eine eigene Referenz
            return;
        }
        WorldSyncApplying++;
        oCItem* it = CreateCoopItem(TpRuneIdx);
        oCItem* in = it ? player->PutInInv(it) : NULL;
        WorldSyncApplying--;
        TpLog("%s: keine Rune im Inventar - %s", why, in ? "eine angelegt" : "Anlegen FEHLGESCHLAGEN");
    }

    // u89: traegt die Rune das eigene Modell? Ueber den Namen (zCProgMeshProto::GetVisualName = Objektname) - GT haelt keinen
    // Zeiger mehr (Coop-Testlauf, Schritt "Rune mit eigenem Aussehen")
    static bool TpRuneHasOwnLook(oCItem* it) {
        if (TpLookState != 1 || !it || !it->GetVisual())
            return false;
        zSTRING n = it->GetVisual()->GetVisualName();
        n.Upper();
        return n == zSTRING(TP_LOOK_VISUAL);
    }

    // Plugin.cpp, LoadEnd: nach jedem Laden pruefen (nach dem Wiederherstellen des eigenen Helden)
    void CoopTeleportLoadEnd() {
        const long long now = GetCurrentMs();
        TpEnsureAt1 = now + 3000;
        TpEnsureAt2 = now + 15000;
        TpPending = false;
        TpCasting = false;
    }

    // u7 (Testlauf u6, Schritt 97 in beiden Laeufen: Rune des Hosts ohne eigenes Aussehen): beim gemeinsamen Speichern nahm
    // zUnionUtils (Union, Gothic 1 ohne Mod) Inventar-Gegenstaende heraus und legte sie neu ein (Log "ITRU_COOP_TELEPORT ins
    // Inventar ... ZUNIONUTILS.DLL+6569") - danach trug die Rune nicht mehr TpLookVisual. Bisher setzte GT es nur nach dem
    // Laden. Jetzt auch 1 s nach jedem Speichern (Plugin.cpp, Game_SaveEnd). Allgemein (jedes Plugin, das so umraeumt).
    void CoopTeleportSaveEnd() {
        TpEnsureAtSave = GetCurrentMs() + 1000;
    }

    // Plugin.cpp, Game_Loop
    void CoopTeleportPulse() {
        if (!player || IsLoadingLevel)
            return;
        TpInstallExternal();
        if (TpPending) {
            TpPending = false;
            TpDo();
        }
        TpAcrossPulse();   // u39 (N2)
        const long long now = GetCurrentMs();
        if (TpEnsureAt1 && now >= TpEnsureAt1) {
            TpEnsureAt1 = 0;
            TpEnsureRune("nach dem Laden");
        }
        if (TpEnsureAt2 && now >= TpEnsureAt2) {
            TpEnsureAt2 = 0;
            TpEnsureRune("15 s nach dem Laden");
        }
        if (TpEnsureAtSave && now >= TpEnsureAtSave && !IsSavingGame) {
            TpEnsureAtSave = 0;
            TpEnsureRune("nach dem Speichern");
        }
    }

#else   // Modul in dieser Engine nicht freigegeben (CoopModulesReport)
    void CoopTeleportCastBegin(oCMag_Book*) {}   // u17
    void CoopTeleportCastEnd() {}
    void CoopTeleportLoadEnd() {}
    void CoopTeleportSaveEnd() {}
    void CoopTeleportPulse() {}
    void CoopRuneLookInit() {}
    bool CoopTeleportAcross(const std::string&, std::string& why) { why = "Teleport-Rune in dieser Engine aus"; return false; }   // u39
    bool CoopTeleportAcrossBusy() { return false; }
    void CoopTeleportAcrossLoadEnd() {}   // u48
#endif
}
